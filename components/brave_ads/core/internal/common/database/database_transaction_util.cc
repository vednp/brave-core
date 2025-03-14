/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/common/database/database_transaction_util.h"

#include <utility>

#include "base/functional/callback.h"
#include "brave/components/brave_ads/common/interfaces/ads.mojom.h"

namespace brave_ads::database {

void OnResultCallback(ResultCallback callback,
                      mojom::DBCommandResponseInfoPtr response) {
  DCHECK(response);

  if (response->status !=
      mojom::DBCommandResponseInfo::StatusType::RESPONSE_OK) {
    return std::move(callback).Run(/*success*/ false);
  }

  std::move(callback).Run(/*success*/ true);
}

}  // namespace brave_ads::database
