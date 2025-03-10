/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/legacy_migration/client/legacy_client_migration_unittest_util.h"

#include "base/check.h"
#include "base/functional/bind.h"
#include "brave/components/brave_ads/common/pref_names.h"
#include "brave/components/brave_ads/core/internal/ads_client_helper.h"
#include "brave/components/brave_ads/core/internal/legacy_migration/client/legacy_client_migration.h"

namespace brave_ads::client {

void Migrate(const bool should_migrate) {
  Migrate(base::BindOnce(
      [](const bool should_migrate, const bool success) {
        CHECK_EQ(success, should_migrate);
      },
      should_migrate));
}

uint64_t GetHash() {
  return AdsClientHelper::GetInstance()->GetUint64Pref(prefs::kClientHash);
}

void SetHash(const uint64_t hash) {
  AdsClientHelper::GetInstance()->SetUint64Pref(prefs::kClientHash, hash);
}

}  // namespace brave_ads::client
