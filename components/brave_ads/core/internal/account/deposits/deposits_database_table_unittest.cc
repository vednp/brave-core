/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/deposits/deposits_database_table.h"

#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads::database::table {

TEST(BatAdsDepositsDatabaseTableTest, TableName) {
  // Arrange
  const Deposits database_table;

  // Act
  const std::string table_name = database_table.GetTableName();

  // Assert
  const std::string expected_table_name = "deposits";
  EXPECT_EQ(expected_table_name, table_name);
}

}  // namespace brave_ads::database::table
