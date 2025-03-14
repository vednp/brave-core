/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/resources/behavioral/anti_targeting/anti_targeting_features.h"

#include "base/metrics/field_trial_params.h"

namespace brave_ads::resource::features {

namespace {

constexpr char kResourceVersionFieldTrialParamName[] = "resource_version";
constexpr int kResourceVersionDefaultValue = 1;

}  // namespace

BASE_FEATURE(kAntiTargeting, "AntiTargeting", base::FEATURE_ENABLED_BY_DEFAULT);

bool IsAntiTargetingEnabled() {
  return base::FeatureList::IsEnabled(kAntiTargeting);
}

int GetAntiTargetingResourceVersion() {
  return GetFieldTrialParamByFeatureAsInt(kAntiTargeting,
                                          kResourceVersionFieldTrialParamName,
                                          kResourceVersionDefaultValue);
}

}  // namespace brave_ads::resource::features
