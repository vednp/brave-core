/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "brave/components/brave_rewards/core/endpoint/promotion/put_safetynet/put_safetynet.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/test/task_environment.h"
#include "brave/components/brave_rewards/core/ledger.h"
#include "brave/components/brave_rewards/core/ledger_client_mock.h"
#include "brave/components/brave_rewards/core/ledger_impl_mock.h"
#include "net/http/http_status_code.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=PutSafetynetTest.*

using ::testing::_;
using ::testing::Invoke;

namespace ledger {
namespace endpoint {
namespace promotion {

class PutSafetynetTest : public testing::Test {
 private:
  base::test::TaskEnvironment scoped_task_environment_;

 protected:
  std::unique_ptr<ledger::MockLedgerClient> mock_ledger_client_;
  std::unique_ptr<ledger::MockLedgerImpl> mock_ledger_impl_;
  std::unique_ptr<PutSafetynet> safetynet_;

  PutSafetynetTest() {
    mock_ledger_client_ = std::make_unique<ledger::MockLedgerClient>();
    mock_ledger_impl_ =
        std::make_unique<ledger::MockLedgerImpl>(mock_ledger_client_.get());
    safetynet_ = std::make_unique<PutSafetynet>(mock_ledger_impl_.get());
  }
};

TEST_F(PutSafetynetTest, ServerOK) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(Invoke(
          [](mojom::UrlRequestPtr request, client::LoadURLCallback callback) {
            mojom::UrlResponse response;
            response.status_code = 200;
            response.url = request->url;
            response.body = "";
            std::move(callback).Run(response);
          }));

  safetynet_->Request("sdfsdf32d323d23d", "dfasdfasdpflsadfplf2r23re2",
                      base::BindOnce([](mojom::Result result) {
                        EXPECT_EQ(result, mojom::Result::LEDGER_OK);
                      }));
}

TEST_F(PutSafetynetTest, ServerError400) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(Invoke(
          [](mojom::UrlRequestPtr request, client::LoadURLCallback callback) {
            mojom::UrlResponse response;
            response.status_code = 400;
            response.url = request->url;
            response.body = "";
            std::move(callback).Run(response);
          }));

  safetynet_->Request("sdfsdf32d323d23d", "dfasdfasdpflsadfplf2r23re2",
                      base::BindOnce([](mojom::Result result) {
                        EXPECT_EQ(result, mojom::Result::CAPTCHA_FAILED);
                      }));
}

TEST_F(PutSafetynetTest, ServerError401) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(Invoke(
          [](mojom::UrlRequestPtr request, client::LoadURLCallback callback) {
            mojom::UrlResponse response;
            response.status_code = 401;
            response.url = request->url;
            response.body = "";
            std::move(callback).Run(response);
          }));

  safetynet_->Request("sdfsdf32d323d23d", "dfasdfasdpflsadfplf2r23re2",
                      base::BindOnce([](mojom::Result result) {
                        EXPECT_EQ(result, mojom::Result::CAPTCHA_FAILED);
                      }));
}

TEST_F(PutSafetynetTest, ServerError500) {
  ON_CALL(*mock_ledger_client_, LoadURL(_, _))
      .WillByDefault(Invoke(
          [](mojom::UrlRequestPtr request, client::LoadURLCallback callback) {
            mojom::UrlResponse response;
            response.status_code = 500;
            response.url = request->url;
            response.body = "";
            std::move(callback).Run(response);
          }));

  safetynet_->Request("sdfsdf32d323d23d", "dfasdfasdpflsadfplf2r23re2",
                      base::BindOnce([](mojom::Result result) {
                        EXPECT_EQ(result, mojom::Result::LEDGER_ERROR);
                      }));
}

}  // namespace promotion
}  // namespace endpoint
}  // namespace ledger
