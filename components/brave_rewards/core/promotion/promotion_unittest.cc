/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/test/task_environment.h"
#include "brave/components/brave_rewards/core/database/database_mock.h"
#include "brave/components/brave_rewards/core/endpoint/promotion/promotions_util.h"
#include "brave/components/brave_rewards/core/ledger.h"
#include "brave/components/brave_rewards/core/ledger_client_mock.h"
#include "brave/components/brave_rewards/core/ledger_impl_mock.h"
#include "brave/components/brave_rewards/core/promotion/promotion.h"
#include "brave/components/brave_rewards/core/state/state_keys.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=PromotionTest.*

using ::testing::_;
using ::testing::Invoke;

namespace ledger {
namespace promotion {

std::string GetResponse(const std::string& url) {
  std::map<std::string, std::string> response;

  // Fetch promotions
  response.insert(std::make_pair(
      endpoint::promotion::GetServerUrl(
          "/v1/promotions"
          "?migrate=true&paymentId=fa5dea51-6af4-44ca-801b-07b6df3dcfe4"
          "&platform="),
      R"({
      "promotions":[{
        "id":"36baa4c3-f92d-4121-b6d9-db44cb273a02",
        "createdAt":"2019-10-30T23:17:15.681226Z",
        "expiresAt":"2020-02-29T23:17:15.681226Z",
        "version":5,
        "suggestionsPerGrant":70,
        "approximateValue":"17.5",
        "type":"ugp",
        "available":true,
        "platform":"desktop",
        "publicKeys":["vNnt88kCh650dFFHt+48SS4d4skQ2FYSxmmlzmKDgkE="],
        "legacyClaimed":false
      }]})"));

  return response[url];
}

class PromotionTest : public testing::Test {
 private:
  base::test::TaskEnvironment scoped_task_environment_;

 protected:
  std::unique_ptr<ledger::MockLedgerClient> mock_ledger_client_;
  std::unique_ptr<ledger::MockLedgerImpl> mock_ledger_impl_;
  std::unique_ptr<Promotion> promotion_;
  std::unique_ptr<database::MockDatabase> mock_database_;

  PromotionTest() {
    mock_ledger_client_ = std::make_unique<ledger::MockLedgerClient>();
    mock_ledger_impl_ =
        std::make_unique<ledger::MockLedgerImpl>(mock_ledger_client_.get());
    promotion_ = std::make_unique<Promotion>(mock_ledger_impl_.get());
    mock_database_ =
        std::make_unique<database::MockDatabase>(mock_ledger_impl_.get());
  }

  void SetUp() override {
    const std::string wallet = R"({
      "payment_id":"fa5dea51-6af4-44ca-801b-07b6df3dcfe4",
      "recovery_seed":"AN6DLuI2iZzzDxpzywf+IKmK1nzFRarNswbaIDI3pQg="
    })";
    ON_CALL(*mock_ledger_client_, GetStringState(state::kWalletBrave))
        .WillByDefault(testing::Return(wallet));

    ON_CALL(*mock_ledger_impl_, database())
        .WillByDefault(testing::Return(mock_database_.get()));

    ON_CALL(*mock_ledger_client_, LoadURL(_, _))
        .WillByDefault(Invoke(
            [](mojom::UrlRequestPtr request, client::LoadURLCallback callback) {
              mojom::UrlResponse response;
              response.status_code = 200;
              response.url = request->url;
              response.body = GetResponse(request->url);
              std::move(callback).Run(response);
            }));
  }
};

TEST_F(PromotionTest, LegacyPromotionIsNotOverwritten) {
  bool inserted = false;
  ON_CALL(*mock_database_, GetAllPromotions(_))
      .WillByDefault(
          Invoke([&inserted](ledger::GetAllPromotionsCallback callback) {
            auto promotion = mojom::Promotion::New();
            base::flat_map<std::string, mojom::PromotionPtr> map;
            if (inserted) {
              const std::string id = "36baa4c3-f92d-4121-b6d9-db44cb273a02";
              promotion->id = id;
              promotion->public_keys =
                  "[\"vNnt88kCh650dFFHt+48SS4d4skQ2FYSxmmlzmKDgkE=\"]";
              promotion->legacy_claimed = true;
              promotion->status = mojom::PromotionStatus::ATTESTED;
              map.insert(std::make_pair(id, std::move(promotion)));
            }

            callback(std::move(map));
          }));

  EXPECT_CALL(*mock_database_, SavePromotion(_, _)).Times(1);

  promotion_->Fetch(base::DoNothing());
  inserted = true;
  promotion_->Fetch(base::DoNothing());
}

}  // namespace promotion
}  // namespace ledger
