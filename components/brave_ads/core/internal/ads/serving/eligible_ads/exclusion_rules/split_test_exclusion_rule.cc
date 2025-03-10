/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/ads/serving/eligible_ads/exclusion_rules/split_test_exclusion_rule.h"

#include "base/metrics/field_trial.h"
#include "base/strings/stringprintf.h"
#include "brave/components/brave_ads/core/internal/creatives/creative_ad_info.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_ads {

namespace {

constexpr char kTrialName[] = "AdvertiserSplitTestStudy";

absl::optional<std::string> GetSplitTestGroupName(
    const std::string& trial_name) {
  base::FieldTrial* field_trial = base::FieldTrialList::Find(trial_name);
  if (!field_trial) {
    return absl::nullopt;
  }

  return field_trial->group_name();
}

bool DoesRespectCap(const CreativeAdInfo& creative_ad) {
  const absl::optional<std::string> split_test_group =
      GetSplitTestGroupName(kTrialName);
  if (!split_test_group) {
    // Only respect cap if browser has signed up to a field trial
    return creative_ad.split_test_group.empty();
  }

  if (creative_ad.split_test_group.empty()) {
    // Always respect cap if there is no split testing group in the catalog
    return true;
  }

  if (creative_ad.split_test_group == split_test_group) {
    return true;
  }

  return false;
}

}  // namespace

std::string SplitTestExclusionRule::GetUuid(
    const CreativeAdInfo& creative_ad) const {
  return creative_ad.creative_set_id;
}

bool SplitTestExclusionRule::ShouldExclude(const CreativeAdInfo& creative_ad) {
  if (!DoesRespectCap(creative_ad)) {
    last_message_ = base::StringPrintf(
        "creativeSetId %s excluded as not associated with an advertiser split "
        "test group",
        creative_ad.creative_set_id.c_str());

    return true;
  }

  return false;
}

const std::string& SplitTestExclusionRule::GetLastMessage() const {
  return last_message_;
}

}  // namespace brave_ads
