/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <utility>

#include "base/base64.h"
#include "base/containers/contains.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include "brave/components/brave_rewards/core/common/time_util.h"
#include "brave/components/brave_rewards/core/constants.h"
#include "brave/components/brave_rewards/core/endpoints/get_parameters/get_parameters_utils.h"
#include "brave/components/brave_rewards/core/ledger_impl.h"
#include "brave/components/brave_rewards/core/option_keys.h"
#include "brave/components/brave_rewards/core/state/state.h"
#include "brave/components/brave_rewards/core/state/state_keys.h"
#include "brave/components/brave_rewards/core/state/state_migration.h"

namespace {

std::string VectorDoubleToString(const std::vector<double>& items) {
  base::Value::List list;
  for (const auto& item : items) {
    list.Append(item);
  }

  std::string items_string;
  base::JSONWriter::Write(list, &items_string);

  return items_string;
}

std::vector<double> StringToVectorDouble(const std::string& items_string) {
  absl::optional<base::Value> list = base::JSONReader::Read(items_string);
  if (!list || !list->is_list()) {
    return {};
  }

  auto& list_value = list->GetList();
  std::vector<double> items;
  for (auto& item : list_value) {
    if (!item.is_double()) {
      continue;
    }
    items.push_back(item.GetDouble());
  }

  return items;
}

std::string PayoutStatusToString(
    const base::flat_map<std::string, std::string>& payout_status) {
  base::Value::Dict dict;
  for (const auto& status : payout_status) {
    dict.Set(status.first, status.second);
  }

  std::string payout_status_string;
  base::JSONWriter::Write(dict, &payout_status_string);

  return payout_status_string;
}

base::flat_map<std::string, std::string> StringToPayoutStatus(
    const std::string& payout_status_string) {
  auto json = base::JSONReader::Read(payout_status_string);
  if (!json || !json->is_dict()) {
    return {};
  }

  auto& dict = json->GetDict();
  base::flat_map<std::string, std::string> payout_status;
  for (const auto [key, value] : dict) {
    if (!value.is_string()) {
      continue;
    }
    payout_status.emplace(key, value.GetString());
  }

  return payout_status;
}

base::Value WalletProviderRegionsToValue(
    const base::flat_map<std::string, ledger::mojom::RegionsPtr>&
        wallet_provider_regions) {
  base::Value::Dict wallet_provider_regions_dict;

  for (const auto& [wallet_provider, regions] : wallet_provider_regions) {
    base::Value::List allow;
    for (const auto& country : regions->allow) {
      allow.Append(country);
    }

    base::Value::List block;
    for (const auto& country : regions->block) {
      block.Append(country);
    }

    base::Value::Dict regions_dict;
    regions_dict.Set("allow", std::move(allow));
    regions_dict.Set("block", std::move(block));

    wallet_provider_regions_dict.Set(wallet_provider, std::move(regions_dict));
  }

  return base::Value(std::move(wallet_provider_regions_dict));
}

base::flat_map<std::string, ledger::mojom::RegionsPtr>
ValueToWalletProviderRegions(const base::Value& value) {
  DCHECK(value.is_dict());
  if (!value.is_dict()) {
    BLOG(0, "Failed to parse JSON!");
    return {};
  }

  auto wallet_provider_regions =
      ledger::endpoints::GetWalletProviderRegions(value.GetDict());
  if (!wallet_provider_regions) {
    BLOG(0, "Failed to parse JSON!");
    return {};
  }

  return std::move(*wallet_provider_regions);
}

std::string ConvertInlineTipPlatformToKey(
    const ledger::mojom::InlineTipsPlatforms platform) {
  switch (platform) {
    case ledger::mojom::InlineTipsPlatforms::REDDIT: {
      return ledger::state::kInlineTipRedditEnabled;
    }
    case ledger::mojom::InlineTipsPlatforms::TWITTER: {
      return ledger::state::kInlineTipTwitterEnabled;
    }
    case ledger::mojom::InlineTipsPlatforms::GITHUB: {
      return ledger::state::kInlineTipGithubEnabled;
    }
    case ledger::mojom::InlineTipsPlatforms::NONE: {
      NOTREACHED();
      return "";
    }
  }
}

}  // namespace

namespace ledger {
namespace state {

State::State(LedgerImpl* ledger)
    : ledger_(ledger), migration_(std::make_unique<StateMigration>(ledger)) {
  DCHECK(ledger_);
}

State::~State() = default;

void State::Initialize(ledger::LegacyResultCallback callback) {
  migration_->Start(callback);
}

void State::SetVersion(const int version) {
  ledger_->database()->SaveEventLog(kVersion, std::to_string(version));
  ledger_->ledger_client()->SetIntegerState(kVersion, version);
}

int State::GetVersion() {
  return ledger_->ledger_client()->GetIntegerState(kVersion);
}

void State::SetPublisherMinVisitTime(const int duration) {
  ledger_->database()->SaveEventLog(kMinVisitTime, std::to_string(duration));
  ledger_->ledger_client()->SetIntegerState(kMinVisitTime, duration);
  ledger_->publisher()->CalcScoreConsts(duration);
  ledger_->publisher()->SynopsisNormalizer();
}

int State::GetPublisherMinVisitTime() {
  return ledger_->ledger_client()->GetIntegerState(kMinVisitTime);
}

void State::SetPublisherMinVisits(const int visits) {
  ledger_->database()->SaveEventLog(kMinVisits, std::to_string(visits));
  ledger_->ledger_client()->SetIntegerState(kMinVisits, visits);
  ledger_->publisher()->SynopsisNormalizer();
}

int State::GetPublisherMinVisits() {
  return ledger_->ledger_client()->GetIntegerState(kMinVisits);
}

void State::SetPublisherAllowNonVerified(const bool allow) {
  ledger_->database()->SaveEventLog(kAllowNonVerified, std::to_string(allow));
  ledger_->ledger_client()->SetBooleanState(kAllowNonVerified, allow);
  ledger_->publisher()->SynopsisNormalizer();
}

bool State::GetPublisherAllowNonVerified() {
  return ledger_->ledger_client()->GetBooleanState(kAllowNonVerified);
}

void State::SetScoreValues(double a, double b) {
  ledger_->database()->SaveEventLog(kScoreA, std::to_string(a));
  ledger_->database()->SaveEventLog(kScoreB, std::to_string(b));
  ledger_->ledger_client()->SetDoubleState(kScoreA, a);
  ledger_->ledger_client()->SetDoubleState(kScoreB, b);
}

void State::GetScoreValues(double* a, double* b) {
  DCHECK(a && b);
  *a = ledger_->ledger_client()->GetDoubleState(kScoreA);
  *b = ledger_->ledger_client()->GetDoubleState(kScoreB);
}

void State::SetAutoContributeEnabled(bool enabled) {
  // Auto-contribute is not supported for regions where bitFlyer is the external
  // wallet provider. If AC is not supported, then always set the pref to false.
  if (ledger_->ledger_client()->GetBooleanOption(option::kIsBitflyerRegion)) {
    enabled = false;
  }

  ledger_->database()->SaveEventLog(kAutoContributeEnabled,
                                    std::to_string(enabled));
  ledger_->ledger_client()->SetBooleanState(kAutoContributeEnabled, enabled);

  if (enabled) {
    ledger_->publisher()->CalcScoreConsts(GetPublisherMinVisitTime());
  }
}

bool State::GetAutoContributeEnabled() {
  // Auto-contribute is not supported for regions where bitFlyer is the external
  // wallet provider. If AC is not supported, then always report AC as disabled.
  if (ledger_->ledger_client()->GetBooleanOption(option::kIsBitflyerRegion)) {
    return false;
  }

  return ledger_->ledger_client()->GetBooleanState(kAutoContributeEnabled);
}

void State::SetAutoContributionAmount(const double amount) {
  ledger_->database()->SaveEventLog(kAutoContributeAmount,
                                    std::to_string(amount));
  ledger_->ledger_client()->SetDoubleState(kAutoContributeAmount, amount);
}

double State::GetAutoContributionAmount() {
  double amount =
      ledger_->ledger_client()->GetDoubleState(kAutoContributeAmount);
  if (amount == 0.0) {
    amount = GetAutoContributeChoice();
  }

  return amount;
}

uint64_t State::GetReconcileStamp() {
  auto stamp = ledger_->ledger_client()->GetUint64State(kNextReconcileStamp);
  if (stamp == 0) {
    ResetReconcileStamp();
    stamp = ledger_->ledger_client()->GetUint64State(kNextReconcileStamp);
  }

  return stamp;
}

void State::SetReconcileStamp(const int reconcile_interval) {
  uint64_t reconcile_stamp = util::GetCurrentTimeStamp();
  if (reconcile_interval > 0) {
    reconcile_stamp += reconcile_interval * 60;
  } else {
    reconcile_stamp += constant::kReconcileInterval;
  }

  ledger_->database()->SaveEventLog(kNextReconcileStamp,
                                    std::to_string(reconcile_stamp));
  ledger_->ledger_client()->SetUint64State(kNextReconcileStamp,
                                           reconcile_stamp);
  ledger_->ledger_client()->ReconcileStampReset();
}
void State::ResetReconcileStamp() {
  SetReconcileStamp(ledger::reconcile_interval);
}

uint64_t State::GetCreationStamp() {
  return ledger_->ledger_client()->GetUint64State(kCreationStamp);
}

void State::SetCreationStamp(const uint64_t stamp) {
  ledger_->database()->SaveEventLog(kCreationStamp, std::to_string(stamp));
  ledger_->ledger_client()->SetUint64State(kCreationStamp, stamp);
}

bool State::GetInlineTippingPlatformEnabled(
    const mojom::InlineTipsPlatforms platform) {
  return ledger_->ledger_client()->GetBooleanState(
      ConvertInlineTipPlatformToKey(platform));
}

void State::SetInlineTippingPlatformEnabled(
    const mojom::InlineTipsPlatforms platform,
    const bool enabled) {
  const std::string platform_string = ConvertInlineTipPlatformToKey(platform);
  ledger_->database()->SaveEventLog(platform_string, std::to_string(enabled));
  ledger_->ledger_client()->SetBooleanState(platform_string, enabled);
}

void State::SetRewardsParameters(const mojom::RewardsParameters& parameters) {
  ledger_->ledger_client()->SetDoubleState(kParametersRate, parameters.rate);
  ledger_->ledger_client()->SetDoubleState(kParametersAutoContributeChoice,
                                           parameters.auto_contribute_choice);
  ledger_->ledger_client()->SetStringState(
      kParametersAutoContributeChoices,
      VectorDoubleToString(parameters.auto_contribute_choices));
  ledger_->ledger_client()->SetStringState(
      kParametersTipChoices, VectorDoubleToString(parameters.tip_choices));
  ledger_->ledger_client()->SetStringState(
      kParametersMonthlyTipChoices,
      VectorDoubleToString(parameters.monthly_tip_choices));
  ledger_->ledger_client()->SetStringState(
      kParametersPayoutStatus, PayoutStatusToString(parameters.payout_status));
  ledger_->ledger_client()->SetValueState(
      kParametersWalletProviderRegions,
      WalletProviderRegionsToValue(parameters.wallet_provider_regions));
  ledger_->ledger_client()->SetTimeState(kParametersVBatDeadline,
                                         parameters.vbat_deadline);
  ledger_->ledger_client()->SetBooleanState(kParametersVBatExpired,
                                            parameters.vbat_expired);
}

mojom::RewardsParametersPtr State::GetRewardsParameters() {
  auto parameters = mojom::RewardsParameters::New();
  parameters->rate = GetRate();
  parameters->auto_contribute_choice = GetAutoContributeChoice();
  parameters->auto_contribute_choices = GetAutoContributeChoices();
  parameters->tip_choices = GetTipChoices();
  parameters->monthly_tip_choices = GetMonthlyTipChoices();
  parameters->payout_status = GetPayoutStatus();
  parameters->wallet_provider_regions = GetWalletProviderRegions();
  parameters->vbat_deadline = GetVBatDeadline();
  parameters->vbat_expired = GetVBatExpired();

  return parameters;
}

double State::GetRate() {
  return ledger_->ledger_client()->GetDoubleState(kParametersRate);
}

double State::GetAutoContributeChoice() {
  return ledger_->ledger_client()->GetDoubleState(
      kParametersAutoContributeChoice);
}

std::vector<double> State::GetAutoContributeChoices() {
  const std::string amounts_string = ledger_->ledger_client()->GetStringState(
      kParametersAutoContributeChoices);
  std::vector<double> amounts = StringToVectorDouble(amounts_string);

  const double current_amount = GetAutoContributionAmount();
  if (!base::Contains(amounts, current_amount)) {
    amounts.push_back(current_amount);
    std::sort(amounts.begin(), amounts.end());

    ledger_->ledger_client()->SetStringState(kParametersAutoContributeChoices,
                                             VectorDoubleToString(amounts));
  }

  return amounts;
}

std::vector<double> State::GetTipChoices() {
  return StringToVectorDouble(
      ledger_->ledger_client()->GetStringState(kParametersTipChoices));
}

std::vector<double> State::GetMonthlyTipChoices() {
  return StringToVectorDouble(
      ledger_->ledger_client()->GetStringState(kParametersMonthlyTipChoices));
}

base::flat_map<std::string, std::string> State::GetPayoutStatus() {
  return StringToPayoutStatus(
      ledger_->ledger_client()->GetStringState(kParametersPayoutStatus));
}

base::flat_map<std::string, mojom::RegionsPtr>
State::GetWalletProviderRegions() {
  return ValueToWalletProviderRegions(ledger_->ledger_client()->GetValueState(
      kParametersWalletProviderRegions));
}

base::Time State::GetVBatDeadline() {
  return ledger_->ledger_client()->GetTimeState(kParametersVBatDeadline);
}

bool State::GetVBatExpired() {
  return ledger_->ledger_client()->GetBooleanState(kParametersVBatExpired);
}

void State::SetEmptyBalanceChecked(const bool checked) {
  ledger_->database()->SaveEventLog(kEmptyBalanceChecked,
                                    std::to_string(checked));
  ledger_->ledger_client()->SetBooleanState(kEmptyBalanceChecked, checked);
}

bool State::GetEmptyBalanceChecked() {
  return ledger_->ledger_client()->GetBooleanState(kEmptyBalanceChecked);
}

void State::SetServerPublisherListStamp(const uint64_t stamp) {
  ledger_->ledger_client()->SetUint64State(kServerPublisherListStamp, stamp);
}

uint64_t State::GetServerPublisherListStamp() {
  return ledger_->ledger_client()->GetUint64State(kServerPublisherListStamp);
}

void State::SetPromotionCorruptedMigrated(const bool migrated) {
  ledger_->database()->SaveEventLog(kPromotionCorruptedMigrated,
                                    std::to_string(migrated));
  ledger_->ledger_client()->SetBooleanState(kPromotionCorruptedMigrated,
                                            migrated);
}

bool State::GetPromotionCorruptedMigrated() {
  return ledger_->ledger_client()->GetBooleanState(kPromotionCorruptedMigrated);
}

void State::SetPromotionLastFetchStamp(const uint64_t stamp) {
  ledger_->ledger_client()->SetUint64State(kPromotionLastFetchStamp, stamp);
}

void State::ResetWalletType() {
  ledger_->ledger_client()->SetStringState(kExternalWalletType, "");
}

uint64_t State::GetPromotionLastFetchStamp() {
  return ledger_->ledger_client()->GetUint64State(kPromotionLastFetchStamp);
}

absl::optional<std::string> State::GetEncryptedString(const std::string& key) {
  std::string value = ledger_->ledger_client()->GetStringState(key);

  // If the state value is empty, then we consider this a successful read of a
  // default empty string.
  if (value.empty()) {
    return "";
  }

  if (!base::Base64Decode(value, &value)) {
    BLOG(0, "Base64 decoding failed for " << key);
    return {};
  }

  auto decrypted = ledger_->ledger_client()->DecryptString(value);
  if (!decrypted) {
    BLOG(0, "Decryption failed for " << key);
    return {};
  }

  return *decrypted;
}

bool State::SetEncryptedString(const std::string& key,
                               const std::string& value) {
  auto encrypted = ledger_->ledger_client()->EncryptString(value);
  if (!encrypted) {
    BLOG(0, "Encryption failed for " << key);
    return false;
  }

  std::string base64_string;
  base::Base64Encode(*encrypted, &base64_string);

  ledger_->ledger_client()->SetStringState(key, base64_string);
  return true;
}

}  // namespace state
}  // namespace ledger
