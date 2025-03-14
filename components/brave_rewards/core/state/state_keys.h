/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_STATE_STATE_KEYS_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_STATE_STATE_KEYS_H_

namespace ledger {
namespace state {

const char kServerPublisherListStamp[] = "publisher_prefix_list_stamp";
const char kUpholdAnonAddress[] = "uphold_anon_address";  // DEPRECATED
const char kPromotionLastFetchStamp[] = "promotion_last_fetch_stamp";
const char kPromotionCorruptedMigrated[] = "promotion_corrupted_migrated2";
const char kVersion[] = "version";
const char kMinVisitTime[] = "ac.min_visit_time";
const char kMinVisits[] = "ac.min_visits";
const char kAllowNonVerified[] = "ac.allow_non_verified";
const char kScoreA[] = "ac.score.a";
const char kScoreB[] = "ac.score.b";
const char kAutoContributeEnabled[] = "ac.enabled";
const char kAutoContributeAmount[] = "ac.amount";
const char kNextReconcileStamp[] = "ac.next_reconcile_stamp";
const char kCreationStamp[] = "creation_stamp";
const char kRecoverySeed[] = "wallet.seed";     // DEPRECATED
const char kPaymentId[] = "wallet.payment_id";  // DEPRECATED
const char kInlineTipRedditEnabled[] = "inline_tip.reddit";
const char kInlineTipTwitterEnabled[] = "inline_tip.twitter";
const char kInlineTipGithubEnabled[] = "inline_tip.github";
const char kParametersRate[] = "parameters.rate";
const char kParametersAutoContributeChoice[] = "parameters.ac.choice";
const char kParametersAutoContributeChoices[] = "parameters.ac.choices";
const char kParametersTipChoices[] = "parameters.tip.choices";
const char kParametersMonthlyTipChoices[] = "parameters.tip.monthly_choices";
const char kParametersPayoutStatus[] = "parameters.payout_status";
const char kParametersWalletProviderRegions[] =
    "parameters.wallet_provider_regions";
const char kParametersVBatDeadline[] = "parameters.vbat_deadline";
const char kParametersVBatExpired[] = "parameters.vbat_expired";
const char kEmptyBalanceChecked[] = "empty_balance_checked";
const char kExternalWalletType[] = "external_wallet_type";
const char kWalletBrave[] = "wallets.brave";
const char kWalletUphold[] = "wallets.uphold";
const char kWalletBitflyer[] = "wallets.bitflyer";
const char kWalletGemini[] = "wallets.gemini";

}  // namespace state
}  // namespace ledger

#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_STATE_STATE_KEYS_H_
