/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_ENDPOINT_BITFLYER_BITFLYER_SERVER_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_ENDPOINT_BITFLYER_BITFLYER_SERVER_H_

#include <memory>

#include "brave/components/brave_rewards/core/endpoint/bitflyer/get_balance/get_balance_bitflyer.h"
#include "brave/components/brave_rewards/core/endpoint/bitflyer/post_oauth/post_oauth_bitflyer.h"
#include "brave/components/brave_rewards/core/ledger.h"

namespace ledger {
class LedgerImpl;

namespace endpoint {

class BitflyerServer {
 public:
  explicit BitflyerServer(LedgerImpl* ledger);
  ~BitflyerServer();

  bitflyer::GetBalance* get_balance() const;

  bitflyer::PostOauth* post_oauth() const;

 private:
  std::unique_ptr<bitflyer::GetBalance> get_balance_;
  std::unique_ptr<bitflyer::PostOauth> post_oauth_;
};

}  // namespace endpoint
}  // namespace ledger

#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_ENDPOINT_BITFLYER_BITFLYER_SERVER_H_
