/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/covariates/log_entries/time_since_last_user_activity_event.h"

#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"
#include "brave/components/brave_ads/core/internal/user_attention/user_activity/user_activity_manager.h"
#include "brave/components/brave_ads/core/internal/user_attention/user_activity/user_activity_util.h"

namespace brave_ads {

namespace {
constexpr base::TimeDelta kTimeWindow = base::Minutes(30);
}  // namespace

TimeSinceLastUserActivityEvent::TimeSinceLastUserActivityEvent(
    UserActivityEventType event_type,
    brave_federated::mojom::CovariateType covariate_type)
    : event_type_(event_type), covariate_type_(covariate_type) {}

brave_federated::mojom::DataType TimeSinceLastUserActivityEvent::GetDataType()
    const {
  return brave_federated::mojom::DataType::kInt;
}

brave_federated::mojom::CovariateType TimeSinceLastUserActivityEvent::GetType()
    const {
  return covariate_type_;
}

std::string TimeSinceLastUserActivityEvent::GetValue() const {
  const UserActivityEventList events =
      UserActivityManager::GetInstance()->GetHistoryForTimeWindow(kTimeWindow);

  return base::NumberToString(
      GetTimeSinceLastUserActivityEvent(events, event_type_));
}

}  // namespace brave_ads
