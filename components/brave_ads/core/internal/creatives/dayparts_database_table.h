/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CREATIVES_DAYPARTS_DATABASE_TABLE_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CREATIVES_DAYPARTS_DATABASE_TABLE_H_

#include <string>

#include "brave/components/brave_ads/common/interfaces/ads.mojom.h"
#include "brave/components/brave_ads/core/ads_client_callback.h"
#include "brave/components/brave_ads/core/internal/creatives/creative_ad_info.h"
#include "brave/components/brave_ads/core/internal/database/database_table_interface.h"

namespace brave_ads::database::table {

class Dayparts final : public TableInterface {
 public:
  void InsertOrUpdate(mojom::DBTransactionInfo* transaction,
                      const CreativeAdList& creative_ads);

  void Delete(ResultCallback callback) const;

  std::string GetTableName() const override;

  void Migrate(mojom::DBTransactionInfo* transaction, int to_version) override;

 private:
  std::string BuildInsertOrUpdateQuery(
      mojom::DBCommandInfo* command,
      const CreativeAdList& creative_ads) const;
};

}  // namespace brave_ads::database::table

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CREATIVES_DAYPARTS_DATABASE_TABLE_H_
