/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/privacy/p2a/opportunities/p2a_opportunity_questions.h"

#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads::privacy::p2a {

TEST(BatAdsP2AOpportunityQuestionsTest, CreateAdOpportunityQuestions) {
  // Arrange
  const std::vector<std::string> segments = {
      "technology & computing", "personal finance-crypto", "travel"};

  // Act
  const std::vector<std::string> questions =
      CreateAdOpportunityQuestions(segments);

  // Assert
  const std::vector<std::string> expected_questions = {
      "Brave.P2A.AdOpportunitiesPerSegment.technologycomputing",
      "Brave.P2A.AdOpportunitiesPerSegment.personalfinance",
      "Brave.P2A.AdOpportunitiesPerSegment.travel",
      "Brave.P2A.TotalAdOpportunities"};

  EXPECT_EQ(expected_questions, questions);
}

TEST(BatAdsP2AOpportunityQuestionsTest,
     CreateAdOpportunityQuestionsForEmptySegments) {
  // Arrange
  const std::vector<std::string> segments;

  // Act
  const std::vector<std::string> questions =
      CreateAdOpportunityQuestions(segments);

  // Assert
  const std::vector<std::string> expected_questions = {
      "Brave.P2A.TotalAdOpportunities"};

  EXPECT_EQ(expected_questions, questions);
}

}  // namespace brave_ads::privacy::p2a
