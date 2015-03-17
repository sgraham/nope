// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/crypto/proxy_media_keys.h"

#include <vector>

#include "base/basictypes.h"
#include "base/logging.h"
#include "base/stl_util.h"
#include "content/renderer/media/crypto/renderer_cdm_manager.h"
#include "media/base/cdm_key_information.h"
#include "media/base/cdm_promise.h"
#include "media/base/key_systems.h"

namespace content {

scoped_ptr<ProxyMediaKeys> ProxyMediaKeys::Create(
    const std::string& key_system,
    const GURL& security_origin,
    RendererCdmManager* manager,
    const media::SessionMessageCB& session_message_cb,
    const media::SessionClosedCB& session_closed_cb,
    const media::SessionErrorCB& session_error_cb,
    const media::SessionKeysChangeCB& session_keys_change_cb,
    const media::SessionExpirationUpdateCB& session_expiration_update_cb) {
  DCHECK(manager);
  scoped_ptr<ProxyMediaKeys> proxy_media_keys(new ProxyMediaKeys(
      manager, session_message_cb, session_closed_cb, session_error_cb,
      session_keys_change_cb, session_expiration_update_cb));
  proxy_media_keys->InitializeCdm(key_system, security_origin);
  return proxy_media_keys.Pass();
}

ProxyMediaKeys::~ProxyMediaKeys() {
  manager_->DestroyCdm(cdm_id_);
  manager_->UnregisterMediaKeys(cdm_id_);
  cdm_promise_adapter_.Clear();
}

void ProxyMediaKeys::SetServerCertificate(
    const uint8* certificate_data,
    int certificate_data_length,
    scoped_ptr<media::SimpleCdmPromise> promise) {
  uint32_t promise_id = cdm_promise_adapter_.SavePromise(promise.Pass());
  manager_->SetServerCertificate(
      cdm_id_, promise_id,
      std::vector<uint8>(certificate_data,
                         certificate_data + certificate_data_length));
}

void ProxyMediaKeys::CreateSessionAndGenerateRequest(
    SessionType session_type,
    const std::string& init_data_type,
    const uint8* init_data,
    int init_data_length,
    scoped_ptr<media::NewSessionCdmPromise> promise) {
  if (session_type != media::MediaKeys::TEMPORARY_SESSION) {
    promise->reject(NOT_SUPPORTED_ERROR, 0,
                    "Only the temporary session type is supported.");
    return;
  }

  // TODO(xhwang): Move these checks up to blink and DCHECK here.
  // See http://crbug.com/342510
  CdmHostMsg_CreateSession_InitDataType create_session_init_data_type;
  if (init_data_type == "cenc") {
    create_session_init_data_type = INIT_DATA_TYPE_CENC;
  } else if (init_data_type == "webm") {
    create_session_init_data_type = INIT_DATA_TYPE_WEBM;
  } else {
    DLOG(ERROR) << "Unsupported EME CreateSession content type of "
                << init_data_type;
    promise->reject(
        NOT_SUPPORTED_ERROR,
        0,
        "Unsupported EME CreateSession init data type of " + init_data_type);
    return;
  }

  uint32_t promise_id = cdm_promise_adapter_.SavePromise(promise.Pass());
  manager_->CreateSessionAndGenerateRequest(
      cdm_id_, promise_id, create_session_init_data_type,
      std::vector<uint8>(init_data, init_data + init_data_length));
}

void ProxyMediaKeys::LoadSession(
    SessionType session_type,
    const std::string& session_id,
    scoped_ptr<media::NewSessionCdmPromise> promise) {
  // TODO(xhwang): Check key system and platform support for LoadSession in
  // blink and add NOTREACHED() here. See http://crbug.com/384152
  DLOG(ERROR) << "ProxyMediaKeys doesn't support session loading.";
  promise->reject(NOT_SUPPORTED_ERROR, 0, "LoadSession() is not supported.");
}

void ProxyMediaKeys::UpdateSession(
    const std::string& session_id,
    const uint8* response,
    int response_length,
    scoped_ptr<media::SimpleCdmPromise> promise) {
  uint32_t promise_id = cdm_promise_adapter_.SavePromise(promise.Pass());
  manager_->UpdateSession(
      cdm_id_, promise_id, session_id,
      std::vector<uint8>(response, response + response_length));
}

void ProxyMediaKeys::CloseSession(const std::string& session_id,
                                  scoped_ptr<media::SimpleCdmPromise> promise) {
  uint32_t promise_id = cdm_promise_adapter_.SavePromise(promise.Pass());
  manager_->CloseSession(cdm_id_, promise_id, session_id);
}

void ProxyMediaKeys::RemoveSession(
    const std::string& session_id,
    scoped_ptr<media::SimpleCdmPromise> promise) {
  // TODO(xhwang): Check key system and platform support for LoadSession in
  // blink and add NOTREACHED() here. See http://crbug.com/384152
  promise->reject(NOT_SUPPORTED_ERROR, 0, "RemoveSession() not supported.");
}

media::CdmContext* ProxyMediaKeys::GetCdmContext() {
  return this;
}

media::Decryptor* ProxyMediaKeys::GetDecryptor() {
  return NULL;
}

int ProxyMediaKeys::GetCdmId() const {
  return cdm_id_;
}

void ProxyMediaKeys::OnSessionMessage(
    const std::string& session_id,
    media::MediaKeys::MessageType message_type,
    const std::vector<uint8>& message,
    const GURL& legacy_destination_url) {
  session_message_cb_.Run(session_id, message_type, message,
                          legacy_destination_url);
}

void ProxyMediaKeys::OnSessionClosed(const std::string& session_id) {
  session_closed_cb_.Run(session_id);
}

void ProxyMediaKeys::OnLegacySessionError(const std::string& session_id,
                                          media::MediaKeys::Exception exception,
                                          uint32 system_code,
                                          const std::string& error_message) {
  session_error_cb_.Run(session_id, exception, system_code, error_message);
}

void ProxyMediaKeys::OnSessionKeysChange(const std::string& session_id,
                                         bool has_additional_usable_key,
                                         media::CdmKeysInfo keys_info) {
  session_keys_change_cb_.Run(session_id, has_additional_usable_key,
                              keys_info.Pass());
}

void ProxyMediaKeys::OnSessionExpirationUpdate(
    const std::string& session_id,
    const base::Time& new_expiry_time) {
  session_expiration_update_cb_.Run(session_id, new_expiry_time);
}

void ProxyMediaKeys::OnPromiseResolved(uint32_t promise_id) {
  cdm_promise_adapter_.ResolvePromise(promise_id);
}

void ProxyMediaKeys::OnPromiseResolvedWithSession(
    uint32_t promise_id,
    const std::string& session_id) {
  cdm_promise_adapter_.ResolvePromise(promise_id, session_id);
}

void ProxyMediaKeys::OnPromiseRejected(uint32_t promise_id,
                                       media::MediaKeys::Exception exception,
                                       uint32_t system_code,
                                       const std::string& error_message) {
  cdm_promise_adapter_.RejectPromise(promise_id, exception, system_code,
                                     error_message);
}

ProxyMediaKeys::ProxyMediaKeys(
    RendererCdmManager* manager,
    const media::SessionMessageCB& session_message_cb,
    const media::SessionClosedCB& session_closed_cb,
    const media::SessionErrorCB& session_error_cb,
    const media::SessionKeysChangeCB& session_keys_change_cb,
    const media::SessionExpirationUpdateCB& session_expiration_update_cb)
    : manager_(manager),
      session_message_cb_(session_message_cb),
      session_closed_cb_(session_closed_cb),
      session_error_cb_(session_error_cb),
      session_keys_change_cb_(session_keys_change_cb),
      session_expiration_update_cb_(session_expiration_update_cb) {
  cdm_id_ = manager->RegisterMediaKeys(this);
}

void ProxyMediaKeys::InitializeCdm(const std::string& key_system,
                                   const GURL& security_origin) {
  manager_->InitializeCdm(cdm_id_, this, key_system, security_origin);
}

}  // namespace content