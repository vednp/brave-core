/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/ads/serving/permission_rules/unblinded_tokens_permission_rule.h"

#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/privacy/tokens/unblinded_tokens/unblinded_tokens_unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads {

class BatAdsUnblindedTokensPermissionRuleTest : public UnitTestBase {};

TEST_F(BatAdsUnblindedTokensPermissionRuleTest, AllowAdIfDoesNotExceedCap) {
  // Arrange
  privacy::SetUnblindedTokens(10);

  // Act
  UnblindedTokensPermissionRule permission_rule;
  const bool is_allowed = permission_rule.ShouldAllow();

  // Assert
  EXPECT_TRUE(is_allowed);
}

TEST_F(BatAdsUnblindedTokensPermissionRuleTest,
       DoNotAllowAdIfNoUnblindedTokens) {
  // Arrange

  // Act
  UnblindedTokensPermissionRule permission_rule;
  const bool is_allowed = permission_rule.ShouldAllow();

  // Assert
  EXPECT_FALSE(is_allowed);
}

TEST_F(BatAdsUnblindedTokensPermissionRuleTest, DoNotAllowAdIfExceedsCap) {
  // Arrange
  privacy::SetUnblindedTokens(9);

  // Act
  UnblindedTokensPermissionRule permission_rule;
  const bool is_allowed = permission_rule.ShouldAllow();

  // Assert
  EXPECT_FALSE(is_allowed);
}

}  // namespace brave_ads
