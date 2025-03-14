/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/legacy_migration/client/legacy_client_migration.h"

#include <string>
#include <utility>

#include "base/functional/bind.h"
#include "brave/components/brave_ads/common/pref_names.h"
#include "brave/components/brave_ads/core/internal/ads_client_helper.h"
#include "brave/components/brave_ads/core/internal/common/logging_util.h"
#include "brave/components/brave_ads/core/internal/deprecated/client/client_info.h"
#include "brave/components/brave_ads/core/internal/deprecated/client/client_state_manager_constants.h"
#include "brave/components/brave_ads/core/internal/legacy_migration/client/legacy_client_migration_util.h"

namespace brave_ads::client {

namespace {

void FailedToMigrate(InitializeCallback callback) {
  std::move(callback).Run(/*success*/ false);
}

void SuccessfullyMigrated(InitializeCallback callback) {
  AdsClientHelper::GetInstance()->SetBooleanPref(prefs::kHasMigratedClientState,
                                                 true);
  std::move(callback).Run(/*success*/ true);
}

}  // namespace

void Migrate(InitializeCallback callback) {
  if (HasMigrated()) {
    return std::move(callback).Run(/*success*/ true);
  }

  AdsClientHelper::GetInstance()->Load(
      kClientStateFilename,
      base::BindOnce(
          [](InitializeCallback callback, const bool success,
             const std::string& json) {
            if (!success) {
              // Client state does not exist
              return SuccessfullyMigrated(std::move(callback));
            }

            ClientInfo client;
            if (!client.FromJson(json)) {
              BLOG(0, "Failed to load client state");
              return FailedToMigrate(std::move(callback));
            }

            BLOG(1, "Migrating client state");

            const std::string migrated_json = client.ToJson();
            SetHashForJson(migrated_json);

            AdsClientHelper::GetInstance()->Save(
                kClientStateFilename, migrated_json,
                base::BindOnce(
                    [](InitializeCallback callback, const bool success) {
                      if (!success) {
                        BLOG(0, "Failed to save client state");
                        return FailedToMigrate(std::move(callback));
                      }

                      BLOG(3, "Successfully migrated client state");
                      SuccessfullyMigrated(std::move(callback));
                    },
                    std::move(callback)));
          },
          std::move(callback)));
}

}  // namespace brave_ads::client
