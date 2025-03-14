/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_CONVERSION_USER_DATA_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_CONVERSION_USER_DATA_H_

#include <string>

#include "base/functional/callback_forward.h"
#include "base/values.h"
namespace brave_ads {

class ConfirmationType;

namespace user_data {

using ConversionCallback = base::OnceCallback<void(base::Value::Dict)>;

void GetConversion(const std::string& creative_instance_id,
                   const ConfirmationType& confirmation_type,
                   ConversionCallback callback);

}  // namespace user_data
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_CONVERSION_USER_DATA_H_
