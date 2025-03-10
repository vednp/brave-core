/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/search_result_ad/search_result_ad_util.h"

#include "base/strings/string_piece.h"
#include "brave/components/brave_search/common/brave_search_utils.h"
#include "url/gurl.h"
#include "url/third_party/mozilla/url_parse.h"

namespace brave_ads {

namespace {

constexpr char kSearchResultAdClickedPath[] = "/a/redirect";
constexpr char kPlacementIdQueryKey[] = "placement_id";

}  // namespace

absl::optional<std::string> GetPlacementIdFromSearchResultAdClickedUrl(
    const GURL& url) {
  if (!url.is_valid() || !url.SchemeIs(url::kHttpsScheme) ||
      url.path_piece() != kSearchResultAdClickedPath || !url.has_query() ||
      !brave_search::IsAllowedHost(url)) {
    return {};
  }

  url::Component query(0, static_cast<int>(url.query_piece().length()));
  url::Component query_key;
  url::Component query_value;
  while (url::ExtractQueryKeyValue(url.query_piece().data(), &query, &query_key,
                                   &query_value)) {
    base::StringPiece key =
        url.query_piece().substr(query_key.begin, query_key.len);
    if (key == kPlacementIdQueryKey) {
      base::StringPiece value =
          url.query_piece().substr(query_value.begin, query_value.len);
      return static_cast<std::string>(value);
    }
  }

  return {};
}

}  // namespace brave_ads
