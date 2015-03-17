# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
   },
  'targets': [
    {
      'target_name': 'ios_chrome_browser',
      'type': 'static_library',
      'include_dirs': [
        '../..',
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../../components/components.gyp:dom_distiller_core',
        '../../components/components.gyp:dom_distiller_ios',
        '../../components/components.gyp:infobars_core',
        '../../components/components.gyp:keyed_service_core',
        '../../components/components.gyp:keyed_service_ios',
        '../../components/components.gyp:leveldb_proto',
        '../../components/components.gyp:suggestions',
        '../../components/components.gyp:translate_core_browser',
        '../../components/components.gyp:translate_ios_browser',
        '../../components/components.gyp:web_resource',
        '../../components/components.gyp:webp_transcode',
        '../../components/components_strings.gyp:components_strings',
        '../../net/net.gyp:net',
        '../../skia/skia.gyp:skia',
        '../../third_party/google_toolbox_for_mac/google_toolbox_for_mac.gyp:google_toolbox_for_mac',
        '../../ui/base/ui_base.gyp:ui_base',
        '../../ui/gfx/gfx.gyp:gfx',
        '../../url/url.gyp:url_lib',
        '../provider/ios_provider_chrome.gyp:ios_provider_chrome_browser',
        '../web/ios_web.gyp:ios_web',
        'ios_chrome_resources.gyp:ios_theme_resources_gen',
      ],
      'link_settings': {
        'libraries': [
          '$(SDKROOT)/System/Library/Frameworks/Accelerate.framework',
          '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
          '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
          '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
          '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
        ],
      },
      'sources': [
        'browser/application_context.cc',
        'browser/application_context.h',
        'browser/application_context_impl.cc',
        'browser/application_context_impl.h',
        'browser/arch_util.cc',
        'browser/arch_util.h',
        'browser/browser_state/browser_state_otr_helper.cc',
        'browser/browser_state/browser_state_otr_helper.h',
        'browser/chrome_switches.cc',
        'browser/chrome_switches.h',
        'browser/chrome_url_constants.cc',
        'browser/chrome_url_constants.h',
        'browser/dom_distiller/distiller_viewer.cc',
        'browser/dom_distiller/distiller_viewer.h',
        'browser/dom_distiller/dom_distiller_service_factory.cc',
        'browser/dom_distiller/dom_distiller_service_factory.h',
        'browser/experimental_flags.h',
        'browser/experimental_flags.mm',
        'browser/infobars/confirm_infobar_controller.h',
        'browser/infobars/confirm_infobar_controller.mm',
        'browser/infobars/infobar.h',
        'browser/infobars/infobar.mm',
        'browser/infobars/infobar_container_ios.h',
        'browser/infobars/infobar_container_ios.mm',
        'browser/infobars/infobar_container_view.h',
        'browser/infobars/infobar_container_view.mm',
        'browser/infobars/infobar_controller.h',
        'browser/infobars/infobar_controller.mm',
        'browser/infobars/infobar_manager_impl.cc',
        'browser/infobars/infobar_manager_impl.h',
        'browser/infobars/infobar_utils.h',
        'browser/infobars/infobar_utils.mm',
        'browser/net/image_fetcher.h',
        'browser/net/image_fetcher.mm',
        'browser/pref_names.cc',
        'browser/pref_names.h',
        'browser/snapshots/snapshot_cache.h',
        'browser/snapshots/snapshot_cache.mm',
        'browser/snapshots/snapshot_manager.h',
        'browser/snapshots/snapshot_manager.mm',
        'browser/snapshots/snapshot_overlay.h',
        'browser/snapshots/snapshot_overlay.mm',
        'browser/snapshots/snapshots_util.h',
        'browser/snapshots/snapshots_util.mm',
        'browser/suggestions/image_fetcher_impl.h',
        'browser/suggestions/image_fetcher_impl.mm',
        'browser/suggestions/suggestions_service_factory.h',
        'browser/suggestions/suggestions_service_factory.mm',
        'browser/translate/after_translate_infobar_controller.h',
        'browser/translate/after_translate_infobar_controller.mm',
        'browser/translate/before_translate_infobar_controller.h',
        'browser/translate/before_translate_infobar_controller.mm',
        'browser/translate/chrome_ios_translate_client.h',
        'browser/translate/chrome_ios_translate_client.mm',
        'browser/translate/never_translate_infobar_controller.h',
        'browser/translate/never_translate_infobar_controller.mm',
        'browser/translate/translate_accept_languages_factory.cc',
        'browser/translate/translate_accept_languages_factory.h',
        'browser/translate/translate_infobar_tags.h',
        'browser/translate/translate_message_infobar_controller.h',
        'browser/translate/translate_message_infobar_controller.mm',
        'browser/translate/translate_service_ios.cc',
        'browser/translate/translate_service_ios.h',
        'browser/ui/animation_util.h',
        'browser/ui/animation_util.mm',
        'browser/ui/commands/UIKit+ChromeExecuteCommand.h',
        'browser/ui/commands/UIKit+ChromeExecuteCommand.mm',
        'browser/ui/commands/clear_browsing_data_command.h',
        'browser/ui/commands/clear_browsing_data_command.mm',
        'browser/ui/commands/generic_chrome_command.h',
        'browser/ui/commands/generic_chrome_command.mm',
        'browser/ui/commands/ios_command_ids.h',
        'browser/ui/commands/open_url_command.h',
        'browser/ui/commands/open_url_command.mm',
        'browser/ui/commands/set_up_for_testing_command.h',
        'browser/ui/commands/set_up_for_testing_command.mm',
        'browser/ui/commands/show_accounts_settings_command.h',
        'browser/ui/commands/show_accounts_settings_command.mm',
        'browser/ui/commands/show_mail_composer_command.h',
        'browser/ui/commands/show_mail_composer_command.mm',
        'browser/ui/image_util.h',
        'browser/ui/image_util.mm',
        'browser/ui/reversed_animation.h',
        'browser/ui/reversed_animation.mm',
        'browser/ui/ui_util.h',
        'browser/ui/ui_util.mm',
        'browser/ui/uikit_ui_util.h',
        'browser/ui/uikit_ui_util.mm',
        'browser/ui/url_loader.h',
        'browser/web/dom_altering_lock.h',
        'browser/web/dom_altering_lock.mm',
        'browser/web_resource/ios_web_resource_service.cc',
        'browser/web_resource/ios_web_resource_service.h',
      ],
    },
  ],
}