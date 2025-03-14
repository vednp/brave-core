/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_NOTIFICATION_AD_SERVING_OBSERVER_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_NOTIFICATION_AD_SERVING_OBSERVER_H_

#include "base/observer_list_types.h"
#include "brave/components/brave_ads/core/internal/segments/segment_alias.h"

namespace brave_ads {

struct NotificationAdInfo;

namespace notification_ads {

class ServingObserver : public base::CheckedObserver {
 public:
  // Invoked when an opportunity arises to serve a notification ad for the
  // |segments|.
  virtual void OnOpportunityAroseToServeNotificationAd(
      const SegmentList& segments) {}

  // Invoked when a notification ad is served.
  virtual void OnDidServeNotificationAd(const NotificationAdInfo& ad) {}

  // Invoked when a notification ad fails to serve.
  virtual void OnFailedToServeNotificationAd() {}
};

}  // namespace notification_ads
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_NOTIFICATION_AD_SERVING_OBSERVER_H_
