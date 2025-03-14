/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_AD_EVENTS_SEARCH_RESULT_ADS_SEARCH_RESULT_AD_EVENT_FACTORY_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_AD_EVENTS_SEARCH_RESULT_ADS_SEARCH_RESULT_AD_EVENT_FACTORY_H_

#include <memory>

#include "brave/components/brave_ads/common/interfaces/ads.mojom-shared.h"
#include "brave/components/brave_ads/core/internal/ads/ad_events/ad_event_interface.h"

namespace brave_ads {

struct SearchResultAdInfo;

namespace search_result_ads {

class AdEventFactory final {
 public:
  static std::unique_ptr<AdEventInterface<SearchResultAdInfo>> Build(
      mojom::SearchResultAdEventType event_type);
};

}  // namespace search_result_ads
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_AD_EVENTS_SEARCH_RESULT_ADS_SEARCH_RESULT_AD_EVENT_FACTORY_H_
