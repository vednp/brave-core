/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_ads/brave_stats_helper.h"

#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "brave/browser/brave_browser_process.h"
#include "brave/components/brave_ads/common/pref_names.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile_test_util.h"
#include "components/prefs/pref_service.h"
#include "content/public/test/browser_test.h"
#include "testing/gtest/include/gtest/gtest.h"

#if BUILDFLAG(IS_ANDROID)
#include "chrome/test/base/android/android_browser_test.h"
#else
#include "chrome/test/base/in_process_browser_test.h"
#endif

namespace brave_ads {

class BraveStatsHelperBrowserTest : public PlatformBrowserTest {
 public:
  BraveStatsHelperBrowserTest() = default;

 protected:
  void SetUpOnMainThread() override {
    PlatformBrowserTest::SetUpOnMainThread();
    profile_manager_ = g_browser_process->profile_manager();
    local_state_ = g_browser_process->local_state();
    brave_stats_helper_ = g_brave_browser_process->ads_brave_stats_helper();
  }

  void PostRunTestOnMainThread() override {
    PlatformBrowserTest::PostRunTestOnMainThread();
  }

  void CreateMultipleProfiles() {
    profile_one_path_ = profile_manager_->GenerateNextProfileDirectoryPath();
    profile_one_ = profiles::testing::CreateProfileSync(profile_manager_,
                                                        profile_one_path_);
    profile_two_path_ = profile_manager_->GenerateNextProfileDirectoryPath();
    profile_two_ = profiles::testing::CreateProfileSync(profile_manager_,
                                                        profile_two_path_);
  }

  base::FilePath profile_one_path_;
  Profile* profile_one_;

  base::FilePath profile_two_path_;
  Profile* profile_two_;

  ProfileManager* profile_manager_;
  PrefService* local_state_;
  raw_ptr<BraveStatsHelper> brave_stats_helper_;

  base::HistogramTester histogram_tester_;
};

IN_PROC_BROWSER_TEST_F(BraveStatsHelperBrowserTest,
                       PrimaryProfileEnabledUpdate) {
  Profile* primary_profile = profile_manager_->GetLastUsedProfile();

  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), false);

  primary_profile->GetPrefs()->SetBoolean(prefs::kEnabled, true);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), true);

  primary_profile->GetPrefs()->SetBoolean(prefs::kEnabled, false);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), false);
}

#if !BUILDFLAG(IS_ANDROID)
IN_PROC_BROWSER_TEST_F(BraveStatsHelperBrowserTest, ProfileSwitch) {
  CreateMultipleProfiles();
  profile_one_->GetPrefs()->SetBoolean(prefs::kEnabled, true);

  profiles::testing::SwitchToProfileSync(profile_one_path_);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), true);

  profiles::testing::SwitchToProfileSync(profile_two_path_);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), false);

  profiles::testing::SwitchToProfileSync(profile_one_path_);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), true);
}

IN_PROC_BROWSER_TEST_F(BraveStatsHelperBrowserTest, MultiProfileEnabledUpdate) {
  CreateMultipleProfiles();
  profile_one_->GetPrefs()->SetBoolean(prefs::kEnabled, true);

  profiles::testing::SwitchToProfileSync(profile_one_path_);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), true);

  profile_two_->GetPrefs()->SetBoolean(prefs::kEnabled, true);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), true);

  profile_one_->GetPrefs()->SetBoolean(prefs::kEnabled, false);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), false);

  profiles::testing::SwitchToProfileSync(profile_two_path_);
  EXPECT_EQ(local_state_->GetBoolean(prefs::kEnabledForLastProfile), true);
}
#endif

IN_PROC_BROWSER_TEST_F(BraveStatsHelperBrowserTest,
                       AdsEnabledInstallationTime) {
  brave_stats_helper_->SetFirstRunTimeForTesting(base::Time::Now() -
                                                 base::Minutes(45));

  Profile* primary_profile = profile_manager_->GetLastUsedProfile();

  primary_profile->GetPrefs()->SetBoolean(prefs::kEnabled, true);

  histogram_tester_.ExpectUniqueSample(kAdsEnabledInstallationTimeHistogramName,
                                       0, 1);

  primary_profile->GetPrefs()->SetBoolean(prefs::kEnabled, false);
  primary_profile->GetPrefs()->SetBoolean(prefs::kEnabled, true);

  histogram_tester_.ExpectUniqueSample(kAdsEnabledInstallationTimeHistogramName,
                                       0, 1);

  // Reset to test another bucket value
  primary_profile->GetPrefs()->SetBoolean(prefs::kEnabled, false);
  local_state_->SetBoolean(prefs::kEverEnabledForAnyProfile, false);
  brave_stats_helper_->SetFirstRunTimeForTesting(base::Time::Now() -
                                                 base::Minutes(70));

  primary_profile->GetPrefs()->SetBoolean(prefs::kEnabled, true);
  histogram_tester_.ExpectBucketCount(kAdsEnabledInstallationTimeHistogramName,
                                      1, 1);
}

}  // namespace brave_ads
