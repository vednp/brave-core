/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_PROMOTED_CONTENT_AD_VALUE_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_PROMOTED_CONTENT_AD_VALUE_UTIL_H_

#include "base/values.h"

namespace brave_ads {

struct PromotedContentAdInfo;

base::Value::Dict PromotedContentAdToValue(const PromotedContentAdInfo& ad);
PromotedContentAdInfo PromotedContentAdFromValue(const base::Value::Dict& root);

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_PROMOTED_CONTENT_AD_VALUE_UTIL_H_
