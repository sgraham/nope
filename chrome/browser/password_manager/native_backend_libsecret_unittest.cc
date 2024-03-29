// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdarg.h>

#include "base/basictypes.h"
#include "base/prefs/pref_service.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/browser/password_manager/native_backend_libsecret.h"
#include "chrome/test/base/testing_profile.h"
#include "components/autofill/core/common/password_form.h"
#include "components/password_manager/core/browser/psl_matching_helper.h"
#include "components/password_manager/core/common/password_manager_pref_names.h"
#include "testing/gtest/include/gtest/gtest.h"

using autofill::PasswordForm;
using base::UTF8ToUTF16;
using base::UTF16ToUTF8;
using password_manager::PasswordStoreChange;
using password_manager::PasswordStoreChangeList;

namespace {

// What follows is a very simple implementation of the subset of the Libsecret
// API that we actually use. It gets substituted for the real one by
// MockLibsecretLoader, which hooks into the facility normally used to load
// the libsecret library at runtime to avoid a static dependency on it.

struct MockSecretValue {
  gchar* password;
  explicit MockSecretValue(gchar* password) : password(password) {}
  ~MockSecretValue() { g_free(password); }
};

struct MockSecretItem {
  MockSecretValue* value;
  GHashTable* attributes;

  MockSecretItem(MockSecretValue* value, GHashTable* attributes)
      : value(value), attributes(attributes) {}
  ~MockSecretItem() {
    delete value;
    g_hash_table_destroy(attributes);
  }

  void RemoveAttribute(const char* keyname) {
    g_hash_table_remove(attributes, keyname);
  }
};

bool Matches(MockSecretItem* item, GHashTable* query) {
  GHashTable* attributes = item->attributes;
  GHashTableIter iter;
  gchar* name;
  gchar* query_value;
  g_hash_table_iter_init(&iter, query);

  while (g_hash_table_iter_next(&iter, reinterpret_cast<gpointer*>(&name),
                                reinterpret_cast<gpointer*>(&query_value))) {
    gchar* value = static_cast<gchar*>(g_hash_table_lookup(attributes, name));
    if (value == nullptr || strcmp(value, query_value) != 0)
      return false;
  }
  return true;
}

bool IsStringAttribute(const SecretSchema* schema, const std::string& name) {
  for (size_t i = 0; schema->attributes[i].name; ++i)
    if (name == schema->attributes[i].name)
      return schema->attributes[i].type == SECRET_SCHEMA_ATTRIBUTE_STRING;
  NOTREACHED() << "Requested type of nonexistent attribute";
  return false;
}

// The list of all libsecret items we have stored.
ScopedVector<MockSecretItem>* global_mock_libsecret_items;
bool global_mock_libsecret_reject_local_ids = false;

gboolean mock_secret_password_store_sync(const SecretSchema* schema,
                                         const gchar* collection,
                                         const gchar* label,
                                         const gchar* password,
                                         GCancellable* cancellable,
                                         GError** error,
                                         ...) {
  GHashTable* attributes =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  va_list ap;
  va_start(ap, error);
  char* name;
  while ((name = va_arg(ap, gchar*))) {
    char* value;
    if (IsStringAttribute(schema, name)) {
      value = g_strdup(va_arg(ap, gchar*));
      VLOG(1) << "Adding item attribute " << name << ", value '" << value
              << "'";
    } else {
      uint32_t intvalue = va_arg(ap, uint32_t);
      VLOG(1) << "Adding item attribute " << name << ", value " << intvalue;
      value = g_strdup_printf("%u", intvalue);
    }
    g_hash_table_insert(attributes, g_strdup(name), value);
  }
  va_end(ap);
  MockSecretValue* secret_value = new MockSecretValue(g_strdup(password));
  MockSecretItem* item = new MockSecretItem(secret_value, attributes);
  global_mock_libsecret_items->push_back(item);
  return true;
}

GList* mock_secret_service_search_sync(SecretService* service,
                                       const SecretSchema* schema,
                                       GHashTable* attributes,
                                       SecretSearchFlags flags,
                                       GCancellable* cancellable,
                                       GError** error) {
  GList* result = nullptr;
  for (MockSecretItem* item : *global_mock_libsecret_items) {
    if (Matches(item, attributes))
      result = g_list_append(result, item);
  }
  return result;
}

gboolean mock_secret_password_clear_sync(const SecretSchema* schema,
                                         GCancellable* cancellable,
                                         GError** error,
                                         ...) {
  GHashTable* attributes =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  va_list ap;
  va_start(ap, error);
  char* name;
  while ((name = va_arg(ap, gchar*))) {
    char* value;
    if (IsStringAttribute(schema, name)) {
      value = g_strdup(va_arg(ap, gchar*));
      VLOG(1) << "Adding item attribute " << name << ", value '" << value
              << "'";
    } else {
      uint32_t intvalue = va_arg(ap, uint32_t);
      VLOG(1) << "Adding item attribute " << name << ", value " << intvalue;
      value = g_strdup_printf("%u", intvalue);
    }
    g_hash_table_insert(attributes, g_strdup(name), value);
  }
  va_end(ap);
  for (uint32_t i = 0; i < global_mock_libsecret_items->size();)
    if (Matches((*global_mock_libsecret_items)[i], attributes)) {
      global_mock_libsecret_items->erase(global_mock_libsecret_items->begin() +
                                         i);
    } else {
      ++i;
    }
  g_hash_table_unref(attributes);
  return true;
}

MockSecretValue* mock_secret_item_get_secret(MockSecretItem* self) {
  return self->value;
}

const gchar* mock_secret_value_get_text(MockSecretValue* value) {
  return value->password;
}

GHashTable* mock_secret_item_get_attributes(MockSecretItem* self) {
  // Libsecret backend will make unreference of received attributes, so in
  // order to save them we need to increase their reference number.
  g_hash_table_ref(self->attributes);
  return self->attributes;
}

gboolean mock_secret_item_load_secret_sync(MockSecretItem* self,
                                           GCancellable* cancellable,
                                           GError** error) {
  return true;
}

void mock_secret_value_unref(gpointer value) {
}

// Inherit to get access to protected fields.
class MockLibsecretLoader : public LibsecretLoader {
 public:
  static bool LoadMockLibsecret() {
    secret_password_store_sync = &mock_secret_password_store_sync;
    secret_service_search_sync = &mock_secret_service_search_sync;
    secret_password_clear_sync = &mock_secret_password_clear_sync;
    secret_item_get_secret =
        (decltype(&::secret_item_get_secret)) & mock_secret_item_get_secret;
    secret_value_get_text =
        (decltype(&::secret_value_get_text)) & mock_secret_value_get_text;
    secret_item_get_attributes = (decltype(&::secret_item_get_attributes)) &
                                 mock_secret_item_get_attributes;
    secret_item_load_secret_sync = (decltype(&::secret_item_load_secret_sync)) &
                                   mock_secret_item_load_secret_sync;
    secret_value_unref =
        (decltype(&::secret_value_unref)) & mock_secret_value_unref;
    libsecret_loaded = true;
    // Reset the state of the mock library.
    global_mock_libsecret_items->clear();
    global_mock_libsecret_reject_local_ids = false;
    return true;
  }
};

void CheckPasswordChanges(const PasswordStoreChangeList& expected_list,
                          const PasswordStoreChangeList& actual_list) {
  ASSERT_EQ(expected_list.size(), actual_list.size());
  for (size_t i = 0; i < expected_list.size(); ++i) {
    EXPECT_EQ(expected_list[i].type(), actual_list[i].type());
    const PasswordForm& expected = expected_list[i].form();
    const PasswordForm& actual = actual_list[i].form();

    EXPECT_EQ(expected.origin, actual.origin);
    EXPECT_EQ(expected.password_value, actual.password_value);
    EXPECT_EQ(expected.action, actual.action);
    EXPECT_EQ(expected.username_element, actual.username_element);
    EXPECT_EQ(expected.username_value, actual.username_value);
    EXPECT_EQ(expected.password_element, actual.password_element);
    EXPECT_EQ(expected.submit_element, actual.submit_element);
    EXPECT_EQ(expected.signon_realm, actual.signon_realm);
    EXPECT_EQ(expected.ssl_valid, actual.ssl_valid);
    EXPECT_EQ(expected.preferred, actual.preferred);
    EXPECT_EQ(expected.date_created, actual.date_created);
    EXPECT_EQ(expected.blacklisted_by_user, actual.blacklisted_by_user);
    EXPECT_EQ(expected.type, actual.type);
    EXPECT_EQ(expected.times_used, actual.times_used);
    EXPECT_EQ(expected.scheme, actual.scheme);
    EXPECT_EQ(expected.date_synced, actual.date_synced);
    EXPECT_EQ(expected.display_name, actual.display_name);
    EXPECT_EQ(expected.avatar_url, actual.avatar_url);
    EXPECT_EQ(expected.federation_url, actual.federation_url);
    EXPECT_EQ(expected.skip_zero_click, actual.skip_zero_click);
    EXPECT_EQ(expected.generation_upload_status,
              actual.generation_upload_status);
  }
}

}  // anonymous namespace

class NativeBackendLibsecretTest : public testing::Test {
 protected:
  enum UpdateType {  // Used in CheckPSLUpdate().
    UPDATE_BY_UPDATELOGIN,
    UPDATE_BY_ADDLOGIN,
  };
  enum RemoveBetweenMethod {  // Used in CheckRemoveLoginsBetween().
    CREATED,
    SYNCED,
  };

  NativeBackendLibsecretTest() {}

  void SetUp() override {
    ASSERT_FALSE(global_mock_libsecret_items);
    global_mock_libsecret_items = &mock_libsecret_items_;

    ASSERT_TRUE(MockLibsecretLoader::LoadMockLibsecret());

    form_google_.origin = GURL("http://www.google.com/");
    form_google_.action = GURL("http://www.google.com/login");
    form_google_.username_element = UTF8ToUTF16("user");
    form_google_.username_value = UTF8ToUTF16("joeschmoe");
    form_google_.password_element = UTF8ToUTF16("pass");
    form_google_.password_value = UTF8ToUTF16("seekrit");
    form_google_.submit_element = UTF8ToUTF16("submit");
    form_google_.signon_realm = "http://www.google.com/";
    form_google_.type = PasswordForm::TYPE_GENERATED;
    form_google_.date_created = base::Time::Now();
    form_google_.date_synced = base::Time::Now();
    form_google_.display_name = UTF8ToUTF16("Joe Schmoe");
    form_google_.avatar_url = GURL("http://www.google.com/avatar");
    form_google_.federation_url = GURL("http://www.google.com/federation_url");
    form_google_.skip_zero_click = true;
    form_google_.generation_upload_status = PasswordForm::POSITIVE_SIGNAL_SENT;
    form_google_.form_data.name = UTF8ToUTF16("form_name");
    form_google_.form_data.user_submitted = true;

    form_facebook_.origin = GURL("http://www.facebook.com/");
    form_facebook_.action = GURL("http://www.facebook.com/login");
    form_facebook_.username_element = UTF8ToUTF16("user");
    form_facebook_.username_value = UTF8ToUTF16("a");
    form_facebook_.password_element = UTF8ToUTF16("password");
    form_facebook_.password_value = UTF8ToUTF16("b");
    form_facebook_.submit_element = UTF8ToUTF16("submit");
    form_facebook_.signon_realm = "http://www.facebook.com/";
    form_facebook_.date_created = base::Time::Now();
    form_facebook_.date_synced = base::Time::Now();
    form_facebook_.display_name = UTF8ToUTF16("Joe Schmoe");
    form_facebook_.avatar_url = GURL("http://www.facebook.com/avatar");
    form_facebook_.federation_url = GURL("http://www.facebook.com/federation");
    form_facebook_.skip_zero_click = true;
    form_facebook_.generation_upload_status = PasswordForm::NO_SIGNAL_SENT;

    form_isc_.origin = GURL("http://www.isc.org/");
    form_isc_.action = GURL("http://www.isc.org/auth");
    form_isc_.username_element = UTF8ToUTF16("id");
    form_isc_.username_value = UTF8ToUTF16("janedoe");
    form_isc_.password_element = UTF8ToUTF16("passwd");
    form_isc_.password_value = UTF8ToUTF16("ihazabukkit");
    form_isc_.submit_element = UTF8ToUTF16("login");
    form_isc_.signon_realm = "http://www.isc.org/";
    form_isc_.date_created = base::Time::Now();
    form_isc_.date_synced = base::Time::Now();

    other_auth_.origin = GURL("http://www.example.com/");
    other_auth_.username_value = UTF8ToUTF16("username");
    other_auth_.password_value = UTF8ToUTF16("pass");
    other_auth_.signon_realm = "http://www.example.com/Realm";
    other_auth_.date_created = base::Time::Now();
    other_auth_.date_synced = base::Time::Now();
  }

  void TearDown() override {
    base::MessageLoop::current()->PostTask(FROM_HERE,
                                           base::MessageLoop::QuitClosure());
    base::MessageLoop::current()->Run();
    ASSERT_TRUE(global_mock_libsecret_items);
    global_mock_libsecret_items = nullptr;
  }

  void RunUIThread() { base::MessageLoop::current()->Run(); }

  void CheckUint32Attribute(const MockSecretItem* item,
                            const std::string& attribute,
                            uint32_t value) {
    gpointer item_value =
        g_hash_table_lookup(item->attributes, attribute.c_str());
    EXPECT_TRUE(item_value) << " in attribute " << attribute;
    if (item_value) {
      uint32_t int_value;
      bool conversion_ok = base::StringToUint((char*)item_value, &int_value);
      EXPECT_TRUE(conversion_ok);
      EXPECT_EQ(value, int_value);
    }
  }

  void CheckStringAttribute(const MockSecretItem* item,
                            const std::string& attribute,
                            const std::string& value) {
    gpointer item_value =
        g_hash_table_lookup(item->attributes, attribute.c_str());
    EXPECT_TRUE(item_value) << " in attribute " << attribute;
    if (item_value) {
      EXPECT_EQ(value, static_cast<char*>(item_value));
    }
  }

  void CheckMockSecretItem(const MockSecretItem* item,
                           const PasswordForm& form,
                           const std::string& app_string) {
    EXPECT_EQ(UTF16ToUTF8(form.password_value), item->value->password);
    EXPECT_EQ(22u, g_hash_table_size(item->attributes));
    CheckStringAttribute(item, "origin_url", form.origin.spec());
    CheckStringAttribute(item, "action_url", form.action.spec());
    CheckStringAttribute(item, "username_element",
                         UTF16ToUTF8(form.username_element));
    CheckStringAttribute(item, "username_value",
                         UTF16ToUTF8(form.username_value));
    CheckStringAttribute(item, "password_element",
                         UTF16ToUTF8(form.password_element));
    CheckStringAttribute(item, "submit_element",
                         UTF16ToUTF8(form.submit_element));
    CheckStringAttribute(item, "signon_realm", form.signon_realm);
    CheckUint32Attribute(item, "ssl_valid", form.ssl_valid);
    CheckUint32Attribute(item, "preferred", form.preferred);
    // We don't check the date created. It varies.
    CheckUint32Attribute(item, "blacklisted_by_user", form.blacklisted_by_user);
    CheckUint32Attribute(item, "type", form.type);
    CheckUint32Attribute(item, "times_used", form.times_used);
    CheckUint32Attribute(item, "scheme", form.scheme);
    CheckStringAttribute(
        item, "date_synced",
        base::Int64ToString(form.date_synced.ToInternalValue()));
    CheckStringAttribute(item, "display_name", UTF16ToUTF8(form.display_name));
    CheckStringAttribute(item, "avatar_url", form.avatar_url.spec());
    CheckStringAttribute(item, "federation_url", form.federation_url.spec());
    CheckUint32Attribute(item, "skip_zero_click", form.skip_zero_click);
    CheckUint32Attribute(item, "generation_upload_status",
                         form.generation_upload_status);
    CheckStringAttribute(item, "application", app_string);
    autofill::FormData actual;
    DeserializeFormDataFromBase64String(
        static_cast<char*>(g_hash_table_lookup(item->attributes, "form_data")),
        &actual);
    EXPECT_TRUE(form.form_data.SameFormAs(actual));
  }

  // Saves |credentials| and then gets logins matching |url| and |scheme|.
  // Returns true when something is found, and in such case copies the result to
  // |result| when |result| is not nullptr. (Note that there can be max. 1
  // result derived from |credentials|.)
  bool CheckCredentialAvailability(const PasswordForm& credentials,
                                   const GURL& url,
                                   const PasswordForm::Scheme& scheme,
                                   PasswordForm* result) {
    NativeBackendLibsecret backend(321);

    backend.AddLogin(credentials);

    PasswordForm target_form;
    target_form.origin = url;
    target_form.signon_realm = url.spec();
    if (scheme != PasswordForm::SCHEME_HTML) {
      // For non-HTML forms, the realm used for authentication
      // (http://tools.ietf.org/html/rfc1945#section-10.2) is appended to the
      // signon_realm. Just use a default value for now.
      target_form.signon_realm.append("Realm");
      target_form.scheme = scheme;
    }
    ScopedVector<autofill::PasswordForm> form_list;
    backend.GetLogins(target_form, &form_list);

    EXPECT_EQ(1u, global_mock_libsecret_items->size());
    if (!global_mock_libsecret_items->empty())
      CheckMockSecretItem((*global_mock_libsecret_items)[0], credentials,
                          "chrome-321");
    global_mock_libsecret_items->clear();

    if (form_list.empty())
      return false;
    EXPECT_EQ(1u, form_list.size());
    if (result)
      *result = *form_list[0];
    return true;
  }

  // Test that updating does not use PSL matching: Add a www.facebook.com
  // password, then use PSL matching to get a copy of it for m.facebook.com, and
  // add that copy as well. Now update the www.facebook.com password -- the
  // m.facebook.com password should not get updated. Depending on the argument,
  // the credential update is done via UpdateLogin or AddLogin.
  void CheckPSLUpdate(UpdateType update_type) {
    NativeBackendLibsecret backend(321);

    backend.AddLogin(form_facebook_);

    // Get the PSL-matched copy of the saved login for m.facebook.
    const GURL kMobileURL("http://m.facebook.com/");
    PasswordForm m_facebook_lookup;
    m_facebook_lookup.origin = kMobileURL;
    m_facebook_lookup.signon_realm = kMobileURL.spec();
    ScopedVector<autofill::PasswordForm> form_list;
    backend.GetLogins(m_facebook_lookup, &form_list);

    EXPECT_EQ(1u, global_mock_libsecret_items->size());
    EXPECT_EQ(1u, form_list.size());
    PasswordForm m_facebook = *form_list[0];
    form_list.clear();
    EXPECT_EQ(kMobileURL, m_facebook.origin);
    EXPECT_EQ(kMobileURL.spec(), m_facebook.signon_realm);

    // Add the PSL-matched copy to saved logins.
    backend.AddLogin(m_facebook);
    EXPECT_EQ(2u, global_mock_libsecret_items->size());

    // Update www.facebook.com login.
    PasswordForm new_facebook(form_facebook_);
    const base::string16 kOldPassword(form_facebook_.password_value);
    const base::string16 kNewPassword(UTF8ToUTF16("new_b"));
    EXPECT_NE(kOldPassword, kNewPassword);
    new_facebook.password_value = kNewPassword;
    scoped_ptr<PasswordStoreChangeList> not_used(new PasswordStoreChangeList());
    switch (update_type) {
      case UPDATE_BY_UPDATELOGIN:
        backend.UpdateLogin(new_facebook, not_used.get());
        break;
      case UPDATE_BY_ADDLOGIN:
        backend.AddLogin(new_facebook);
        break;
    }

    EXPECT_EQ(2u, global_mock_libsecret_items->size());

    // Check that m.facebook.com login was not modified by the update.
    backend.GetLogins(m_facebook_lookup, &form_list);

    // There should be two results -- the exact one, and the PSL-matched one.
    EXPECT_EQ(2u, form_list.size());
    size_t index_non_psl = 0;
    if (!form_list[index_non_psl]->original_signon_realm.empty())
      index_non_psl = 1;
    EXPECT_EQ(kMobileURL, form_list[index_non_psl]->origin);
    EXPECT_EQ(kMobileURL.spec(), form_list[index_non_psl]->signon_realm);
    EXPECT_EQ(kOldPassword, form_list[index_non_psl]->password_value);
    form_list.clear();

    // Check that www.facebook.com login was modified by the update.
    backend.GetLogins(form_facebook_, &form_list);
    // There should be two results -- the exact one, and the PSL-matched one.
    EXPECT_EQ(2u, form_list.size());
    index_non_psl = 0;
    if (!form_list[index_non_psl]->original_signon_realm.empty())
      index_non_psl = 1;
    EXPECT_EQ(form_facebook_.origin, form_list[index_non_psl]->origin);
    EXPECT_EQ(form_facebook_.signon_realm,
              form_list[index_non_psl]->signon_realm);
    EXPECT_EQ(kNewPassword, form_list[index_non_psl]->password_value);
    form_list.clear();
  }

  // Checks various types of matching for forms with a non-HTML |scheme|.
  void CheckMatchingWithScheme(const PasswordForm::Scheme& scheme) {
    ASSERT_NE(PasswordForm::SCHEME_HTML, scheme);
    other_auth_.scheme = scheme;

    // Don't match a non-HTML form with an HTML form.
    EXPECT_FALSE(
        CheckCredentialAvailability(other_auth_, GURL("http://www.example.com"),
                                    PasswordForm::SCHEME_HTML, nullptr));
    // Don't match an HTML form with non-HTML auth form.
    EXPECT_FALSE(CheckCredentialAvailability(
        form_google_, GURL("http://www.google.com/"), scheme, nullptr));
    // Don't match two different non-HTML auth forms with different origin.
    EXPECT_FALSE(CheckCredentialAvailability(
        other_auth_, GURL("http://first.example.com"), scheme, nullptr));
    // Do match non-HTML forms from the same origin.
    EXPECT_TRUE(CheckCredentialAvailability(
        other_auth_, GURL("http://www.example.com/"), scheme, nullptr));
  }

  void CheckRemoveLoginsBetween(RemoveBetweenMethod date_to_test) {
    NativeBackendLibsecret backend(42);

    base::Time now = base::Time::Now();
    base::Time next_day = now + base::TimeDelta::FromDays(1);
    form_google_.date_synced = base::Time();
    form_isc_.date_synced = base::Time();
    form_google_.date_created = now;
    form_isc_.date_created = now;
    if (date_to_test == CREATED) {
      form_google_.date_created = now;
      form_isc_.date_created = next_day;
    } else {
      form_google_.date_synced = now;
      form_isc_.date_synced = next_day;
    }

    backend.AddLogin(form_google_);
    backend.AddLogin(form_isc_);

    PasswordStoreChangeList expected_changes;
    expected_changes.push_back(
        PasswordStoreChange(PasswordStoreChange::REMOVE, form_google_));
    PasswordStoreChangeList changes;
    bool (NativeBackendLibsecret::*method)(
        base::Time, base::Time, password_manager::PasswordStoreChangeList*) =
        date_to_test == CREATED
            ? &NativeBackendLibsecret::RemoveLoginsCreatedBetween
            : &NativeBackendLibsecret::RemoveLoginsSyncedBetween;

    EXPECT_TRUE(base::Bind(method, base::Unretained(&backend), base::Time(),
                           next_day, &changes).Run());
    CheckPasswordChanges(expected_changes, changes);

    EXPECT_EQ(1u, global_mock_libsecret_items->size());
    if (!global_mock_libsecret_items->empty() > 0)
      CheckMockSecretItem((*global_mock_libsecret_items)[0], form_isc_,
                          "chrome-42");

    // Remove form_isc_.
    expected_changes.clear();
    expected_changes.push_back(
        PasswordStoreChange(PasswordStoreChange::REMOVE, form_isc_));

    EXPECT_TRUE(base::Bind(method, base::Unretained(&backend), next_day,
                           base::Time(), &changes).Run());
    CheckPasswordChanges(expected_changes, changes);

    EXPECT_TRUE(global_mock_libsecret_items->empty());
  }

  base::MessageLoopForUI message_loop_;

  // Provide some test forms to avoid having to set them up in each test.
  PasswordForm form_google_;
  PasswordForm form_facebook_;
  PasswordForm form_isc_;
  PasswordForm other_auth_;

  ScopedVector<MockSecretItem> mock_libsecret_items_;
};

TEST_F(NativeBackendLibsecretTest, BasicAddLogin) {
  NativeBackendLibsecret backend(42);

  backend.AddLogin(form_google_);

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");
}

TEST_F(NativeBackendLibsecretTest, BasicListLogins) {
  NativeBackendLibsecret backend(42);

  backend.AddLogin(form_google_);

  ScopedVector<autofill::PasswordForm> form_list;
  backend.GetAutofillableLogins(&form_list);

  // Quick check that we got something back.
  EXPECT_EQ(1u, form_list.size());
  form_list.clear();

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");
}

// Save a password for www.facebook.com and see it suggested for m.facebook.com.
TEST_F(NativeBackendLibsecretTest, PSLMatchingPositive) {
  PasswordForm result;
  const GURL kMobileURL("http://m.facebook.com/");
  EXPECT_TRUE(CheckCredentialAvailability(form_facebook_, kMobileURL,
                                          PasswordForm::SCHEME_HTML, &result));
  EXPECT_EQ(kMobileURL, result.origin);
  EXPECT_EQ(kMobileURL.spec(), result.signon_realm);
}

// Save a password for www.facebook.com and see it not suggested for
// m-facebook.com.
TEST_F(NativeBackendLibsecretTest, PSLMatchingNegativeDomainMismatch) {
  EXPECT_FALSE(CheckCredentialAvailability(form_facebook_,
                                           GURL("http://m-facebook.com/"),
                                           PasswordForm::SCHEME_HTML, nullptr));
}

// Test PSL matching is off for domains excluded from it.
TEST_F(NativeBackendLibsecretTest, PSLMatchingDisabledDomains) {
  EXPECT_FALSE(CheckCredentialAvailability(form_google_,
                                           GURL("http://one.google.com/"),
                                           PasswordForm::SCHEME_HTML, nullptr));
}

// Make sure PSL matches aren't available for non-HTML forms.
TEST_F(NativeBackendLibsecretTest, PSLMatchingDisabledForNonHTMLForms) {
  CheckMatchingWithScheme(PasswordForm::SCHEME_BASIC);
  CheckMatchingWithScheme(PasswordForm::SCHEME_DIGEST);
  CheckMatchingWithScheme(PasswordForm::SCHEME_OTHER);
}

TEST_F(NativeBackendLibsecretTest, PSLUpdatingStrictUpdateLogin) {
  CheckPSLUpdate(UPDATE_BY_UPDATELOGIN);
}

TEST_F(NativeBackendLibsecretTest, PSLUpdatingStrictAddLogin) {
  // TODO(vabr): if AddLogin becomes no longer valid for existing logins, then
  // just delete this test.
  CheckPSLUpdate(UPDATE_BY_ADDLOGIN);
}

TEST_F(NativeBackendLibsecretTest, BasicUpdateLogin) {
  NativeBackendLibsecret backend(42);

  backend.AddLogin(form_google_);

  PasswordForm new_form_google(form_google_);
  new_form_google.times_used = 1;
  new_form_google.action = GURL("http://www.google.com/different/login");

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");

  // Update login
  PasswordStoreChangeList changes;
  backend.UpdateLogin(new_form_google, &changes);

  ASSERT_EQ(1u, changes.size());
  EXPECT_EQ(PasswordStoreChange::UPDATE, changes.front().type());
  EXPECT_EQ(new_form_google, changes.front().form());
  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], new_form_google,
                        "chrome-42");
}

TEST_F(NativeBackendLibsecretTest, BasicRemoveLogin) {
  NativeBackendLibsecret backend(42);

  backend.AddLogin(form_google_);

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");

  backend.RemoveLogin(form_google_);

  EXPECT_TRUE(global_mock_libsecret_items->empty());
}

// Verify fix for http://crbug.com/408783.
TEST_F(NativeBackendLibsecretTest, RemoveLoginActionMismatch) {
  NativeBackendLibsecret backend(42);

  backend.AddLogin(form_google_);

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");

  // Action url match not required for removal.
  form_google_.action = GURL("https://some.other.url.com/path");

  backend.RemoveLogin(form_google_);

  EXPECT_TRUE(global_mock_libsecret_items->empty());
}

TEST_F(NativeBackendLibsecretTest, RemoveNonexistentLogin) {
  NativeBackendLibsecret backend(42);

  // First add an unrelated login.
  backend.AddLogin(form_google_);

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");

  // Attempt to remove a login that doesn't exist.
  backend.RemoveLogin(form_isc_);

  // Make sure we can still get the first form back.
  ScopedVector<autofill::PasswordForm> form_list;
  backend.GetAutofillableLogins(&form_list);

  // Quick check that we got something back.
  EXPECT_EQ(1u, form_list.size());
  form_list.clear();

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");
}

TEST_F(NativeBackendLibsecretTest, UpdateNonexistentLogin) {
  NativeBackendLibsecret backend(42);

  // First add an unrelated login.
  backend.AddLogin(form_google_);

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");

  // Attempt to update a login that doesn't exist.
  PasswordStoreChangeList changes;
  backend.UpdateLogin(form_isc_, &changes);

  EXPECT_EQ(PasswordStoreChangeList(), changes);
  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");
}

TEST_F(NativeBackendLibsecretTest, AddDuplicateLogin) {
  NativeBackendLibsecret backend(42);

  PasswordStoreChangeList expected_changes, actual_changes;
  expected_changes.push_back(
      PasswordStoreChange(PasswordStoreChange::ADD, form_google_));
  actual_changes = backend.AddLogin(form_google_);
  CheckPasswordChanges(expected_changes, actual_changes);

  expected_changes.clear();
  expected_changes.push_back(
      PasswordStoreChange(PasswordStoreChange::REMOVE, form_google_));
  form_google_.times_used++;
  expected_changes.push_back(
      PasswordStoreChange(PasswordStoreChange::ADD, form_google_));

  actual_changes = backend.AddLogin(form_google_);
  CheckPasswordChanges(expected_changes, actual_changes);

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");
}

TEST_F(NativeBackendLibsecretTest, ListLoginsAppends) {
  NativeBackendLibsecret backend(42);

  backend.AddLogin(form_google_);

  // Send the same request twice with the same list both times.
  ScopedVector<autofill::PasswordForm> form_list;
  backend.GetAutofillableLogins(&form_list);
  backend.GetAutofillableLogins(&form_list);

  // Quick check that we got two results back.
  EXPECT_EQ(2u, form_list.size());
  form_list.clear();

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  if (!global_mock_libsecret_items->empty())
    CheckMockSecretItem((*global_mock_libsecret_items)[0], form_google_,
                        "chrome-42");
}

TEST_F(NativeBackendLibsecretTest, AndroidCredentials) {
  NativeBackendLibsecret backend(42);
  backend.Init();

  PasswordForm observed_android_form;
  observed_android_form.scheme = PasswordForm::SCHEME_HTML;
  observed_android_form.signon_realm =
      "android://7x7IDboo8u9YKraUsbmVkuf1-@net.rateflix.app/";
  PasswordForm saved_android_form = observed_android_form;
  saved_android_form.username_value = base::UTF8ToUTF16("randomusername");
  saved_android_form.password_value = base::UTF8ToUTF16("password");
  saved_android_form.date_created = base::Time::Now();

  backend.AddLogin(saved_android_form);

  ScopedVector<autofill::PasswordForm> form_list;
  backend.GetAutofillableLogins(&form_list);

  EXPECT_EQ(1u, form_list.size());
  EXPECT_EQ(saved_android_form, *form_list[0]);
}


TEST_F(NativeBackendLibsecretTest, RemoveLoginsCreatedBetween) {
  CheckRemoveLoginsBetween(CREATED);
}

TEST_F(NativeBackendLibsecretTest, RemoveLoginsSyncedBetween) {
  CheckRemoveLoginsBetween(SYNCED);
}

TEST_F(NativeBackendLibsecretTest, SomeKeyringAttributesAreMissing) {
  // Absent attributes should be filled with default values.
  NativeBackendLibsecret backend(42);

  backend.AddLogin(form_google_);

  EXPECT_EQ(1u, global_mock_libsecret_items->size());
  // Remove a string attribute.
  (*global_mock_libsecret_items)[0]->RemoveAttribute("avatar_url");
  // Remove an integer attribute.
  (*global_mock_libsecret_items)[0]->RemoveAttribute("ssl_valid");

  ScopedVector<autofill::PasswordForm> form_list;
  backend.GetAutofillableLogins(&form_list);

  EXPECT_EQ(1u, form_list.size());
  EXPECT_EQ(GURL(""), form_list[0]->avatar_url);
  EXPECT_FALSE(form_list[0]->ssl_valid);
}

// TODO(mdm): add more basic tests here at some point.
