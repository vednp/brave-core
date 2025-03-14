/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "base/command_line.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/field_trial_param_associator.h"
#include "brave/browser/ui/whats_new/pref_names.h"
#include "brave/browser/ui/whats_new/whats_new_util.h"
#include "brave/components/l10n/common/test/scoped_default_locale.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"

namespace whats_new {

class BraveWhatsNewBrowserTest : public InProcessBrowserTest {
 public:
  BraveWhatsNewBrowserTest() {
    scoped_default_locale_ =
        std::make_unique<brave_l10n::test::ScopedDefaultLocale>("en_US");
    PrepareValidFieldTrialParams();
    SetCurrentVersionForTesting(1.51);

    // To disable tab presets for startup.
    // When preset tabs are used, whats-new page is not launched.
    set_open_about_blank_on_browser_launch(false);
  }

  ~BraveWhatsNewBrowserTest() override = default;

  void PrepareValidFieldTrialParams() {
    constexpr char kWhatsNewTrial[] = "WhatsNewStudy";
    std::map<std::string, std::string> params;
    params["target_major_version"] = "1.51";
    ASSERT_TRUE(
        base::AssociateFieldTrialParams(kWhatsNewTrial, "Enabled", params));
    base::FieldTrialList::CreateFieldTrial(kWhatsNewTrial, "Enabled");
  }

  TabStripModel* tab_model() const { return browser()->tab_strip_model(); }
  PrefService* local_state() const { return g_browser_process->local_state(); }

  std::unique_ptr<brave_l10n::test::ScopedDefaultLocale> scoped_default_locale_;
};

IN_PROC_BROWSER_TEST_F(BraveWhatsNewBrowserTest, PRE_WhatsNewPageLaunchTest) {
  // Whats-new page is not shown with onboarding page together.
  // It's upstream logic - see the comments of
  // StartupBrowserCreatorImpl::DetermineStartupTabs().
  EXPECT_EQ(1, tab_model()->count());
  EXPECT_EQ(GURL(" chrome://welcome/"),
            tab_model()->GetActiveWebContents()->GetVisibleURL());

  // In production, fresh user doesn't see whats-new for that version.
  // For testing purpose, clear cache to test whether whats-new is launched in
  // another launch.
  local_state()->SetDouble(prefs::kWhatsNewLastVersion, 0);
}

IN_PROC_BROWSER_TEST_F(BraveWhatsNewBrowserTest, WhatsNewPageLaunchTest) {
  // Two tabs - first one is whats-new and another one is welcome
  EXPECT_EQ(2, tab_model()->count());
  EXPECT_EQ(GURL("https://brave.com/whats-new/"),
            tab_model()->GetActiveWebContents()->GetVisibleURL());
  EXPECT_EQ(1.51, local_state()->GetDouble(prefs::kWhatsNewLastVersion));
}

}  // namespace whats_new
