/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CREATIVES_NOTIFICATION_ADS_CREATIVE_NOTIFICATION_AD_UNITTEST_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CREATIVES_NOTIFICATION_ADS_CREATIVE_NOTIFICATION_AD_UNITTEST_UTIL_H_

#include "brave/components/brave_ads/core/internal/creatives/notification_ads/creative_notification_ad_info.h"

namespace brave_ads {

void SaveCreativeAds(const CreativeNotificationAdList& creative_ads);

CreativeNotificationAdList BuildCreativeNotificationAds(int count);
CreativeNotificationAdInfo BuildCreativeNotificationAd(
    bool should_use_random_guids = true);

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CREATIVES_NOTIFICATION_ADS_CREATIVE_NOTIFICATION_AD_UNITTEST_UTIL_H_
