/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/flags/did_override/did_override_command_line_switches_util.h"

#include <memory>
#include <string>
#include <utility>

#include "base/strings/stringprintf.h"
#include "base/test/scoped_feature_list.h"
#include "brave/components/brave_ads/core/internal/common/unittest/command_line_switch_info.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_command_line_switch_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads {

namespace {

constexpr char kFooBarSwitch[] = "foobar";
constexpr char kEnableAutomationSwitch[] = "enable-automation";

struct ParamInfo final {
  CommandLineSwitchInfo command_line_switch;
  bool expected_did_override_command_line_switch;
} const kTestCases[] = {{/*command_line_switch*/ {kFooBarSwitch, {}},
                         /*expected_did_override_command_line_switch*/ false},
                        {/*command_line_switch*/ {kEnableAutomationSwitch, {}},
                         /*expected_did_override_command_line_switch*/ true}};

}  // namespace

class BatAdsDidOverrideCommandLineSwitchesUtilTest
    : public UnitTestBase,
      public ::testing::WithParamInterface<ParamInfo> {
 protected:
  void SetUpMocks() override {
    const ParamInfo param = GetParam();

    AppendCommandLineSwitches({param.command_line_switch});

    std::unique_ptr<base::FeatureList> feature_list =
        std::make_unique<base::FeatureList>();
    feature_list->InitializeFromCommandLine(param.command_line_switch.value,
                                            {});
    scoped_feature_list_.InitWithFeatureList(std::move(feature_list));
  }

  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_P(BatAdsDidOverrideCommandLineSwitchesUtilTest,
       DidOverrideCommandLineSwitches) {
  // Arrange

  // Act

  // Assert
  EXPECT_EQ(GetParam().expected_did_override_command_line_switch,
            DidOverrideCommandLineSwitches());
}

std::string TestParamToString(
    const ::testing::TestParamInfo<ParamInfo>& test_param) {
  const std::string expected_did_override_command_line_switch =
      test_param.param.expected_did_override_command_line_switch
          ? "DidOverride"
          : "DidNotOverride";

  const std::string sanitized_command_line_switch =
      SanitizeCommandLineSwitch(test_param.param.command_line_switch);

  return base::StringPrintf("Set%sCommandLineSwitchesFor%s",
                            expected_did_override_command_line_switch.c_str(),
                            sanitized_command_line_switch.c_str());
}

INSTANTIATE_TEST_SUITE_P(,
                         BatAdsDidOverrideCommandLineSwitchesUtilTest,
                         testing::ValuesIn(kTestCases),
                         TestParamToString);

}  // namespace brave_ads
