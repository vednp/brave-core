/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_SERVER_URL_HOSTS_STATIC_SERVER_HOST_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_SERVER_URL_HOSTS_STATIC_SERVER_HOST_H_

#include <string>

#include "brave/components/brave_ads/core/internal/server/url/hosts/server_host_interface.h"

namespace brave_ads {

class StaticServerHost final : public ServerHostInterface {
 public:
  std::string Get() const override;
};

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_SERVER_URL_HOSTS_STATIC_SERVER_HOST_H_
