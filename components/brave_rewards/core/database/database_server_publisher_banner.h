/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_DATABASE_DATABASE_SERVER_PUBLISHER_BANNER_H_
#define BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_DATABASE_DATABASE_SERVER_PUBLISHER_BANNER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "brave/components/brave_rewards/core/database/database_server_publisher_links.h"
#include "brave/components/brave_rewards/core/database/database_table.h"

namespace ledger {
namespace database {

class DatabaseServerPublisherBanner : public DatabaseTable {
 public:
  explicit DatabaseServerPublisherBanner(LedgerImpl* ledger);
  ~DatabaseServerPublisherBanner() override;

  void InsertOrUpdate(mojom::DBTransaction* transaction,
                      const mojom::ServerPublisherInfo& server_info);

  void DeleteRecords(mojom::DBTransaction* transaction,
                     const std::string& publisher_key_list);

  void GetRecord(const std::string& publisher_key,
                 ledger::PublisherBannerCallback callback);

 private:
  void OnGetRecord(mojom::DBCommandResponsePtr response,
                   const std::string& publisher_key,
                   ledger::PublisherBannerCallback callback);

  void OnGetRecordLinks(const std::map<std::string, std::string>& links,
                        const mojom::PublisherBanner& banner,
                        ledger::PublisherBannerCallback callback);

  std::unique_ptr<DatabaseServerPublisherLinks> links_;
};

}  // namespace database
}  // namespace ledger

#endif  // BRAVE_COMPONENTS_BRAVE_REWARDS_CORE_DATABASE_DATABASE_SERVER_PUBLISHER_BANNER_H_
