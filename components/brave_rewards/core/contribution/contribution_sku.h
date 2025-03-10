/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_CONTRIBUTION_CONTRIBUTION_SKU_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_CONTRIBUTION_CONTRIBUTION_SKU_H_

#include <memory>
#include <string>
#include <vector>

#include "brave/components/brave_rewards/core/credentials/credentials_factory.h"
#include "brave/components/brave_rewards/core/ledger.h"
#include "brave/components/brave_rewards/core/sku/sku_factory.h"

namespace ledger {
class LedgerImpl;

namespace contribution {

class ContributionSKU {
 public:
  explicit ContributionSKU(LedgerImpl* ledger);
  ~ContributionSKU();

  void AutoContribution(const std::string& contribution_id,
                        const std::string& wallet_type,
                        ledger::LegacyResultCallback callback);

  void Merchant(const mojom::SKUTransaction&,
                client::LegacyResultCallback callback);

  void Retry(mojom::ContributionInfoPtr contribution,
             ledger::LegacyResultCallback callback);

 private:
  void Start(const std::string& contribution_id,
             const mojom::SKUOrderItem& item,
             const std::string& wallet_type,
             ledger::LegacyResultCallback callback);

  void GetContributionInfo(mojom::ContributionInfoPtr contribution,
                           const mojom::SKUOrderItem& item,
                           const std::string& wallet_type,
                           ledger::LegacyResultCallback callback);

  void GetOrder(mojom::Result result,
                const std::string& order_id,
                const std::string& contribution_id,
                ledger::LegacyResultCallback callback);

  void OnGetOrder(mojom::SKUOrderPtr order,
                  const std::string& contribution_id,
                  ledger::LegacyResultCallback callback);

  void Completed(mojom::Result result,
                 const std::string& contribution_id,
                 mojom::RewardsType type,
                 ledger::LegacyResultCallback callback);

  void CredsStepSaved(mojom::Result result,
                      const std::string& contribution_id,
                      ledger::LegacyResultCallback callback);

  void GetUnblindedTokens(std::vector<mojom::UnblindedTokenPtr> list,
                          const mojom::SKUTransaction&,
                          client::LegacyResultCallback);

  void GetOrderMerchant(mojom::SKUOrderPtr,
                        const credential::CredentialsRedeem&,
                        client::LegacyResultCallback);

  void OnRedeemTokens(mojom::Result, client::LegacyResultCallback);

  void OnOrder(mojom::SKUOrderPtr order,
               std::shared_ptr<mojom::ContributionInfoPtr> shared_contribution,
               ledger::LegacyResultCallback callback);

  void RetryStartStep(mojom::ContributionInfoPtr contribution,
                      mojom::SKUOrderPtr order,
                      ledger::LegacyResultCallback callback);

  LedgerImpl* ledger_;  // NOT OWNED
  std::unique_ptr<credential::Credentials> credentials_;
  std::unique_ptr<sku::SKU> sku_;
};

}  // namespace contribution
}  // namespace ledger
#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_CONTRIBUTION_CONTRIBUTION_SKU_H_
