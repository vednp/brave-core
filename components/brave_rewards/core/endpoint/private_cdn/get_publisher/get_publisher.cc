/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "brave/components/brave_rewards/core/endpoint/private_cdn/get_publisher/get_publisher.h"

#include <utility>

#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "brave/components/brave_private_cdn/private_cdn_helper.h"
#include "brave/components/brave_rewards/core/common/brotli_util.h"
#include "brave/components/brave_rewards/core/common/time_util.h"
#include "brave/components/brave_rewards/core/endpoint/private_cdn/private_cdn_util.h"
#include "brave/components/brave_rewards/core/ledger_impl.h"
#include "brave/components/brave_rewards/core/publisher/protos/channel_response.pb.h"
#include "net/base/load_flags.h"
#include "net/http/http_status_code.h"

using std::placeholders::_1;

// Due to privacy concerns, the request length must be consistent
// for all publisher lookups. Do not add URL parameters or headers
// whose size will vary depending on the publisher key.

namespace {

ledger::mojom::PublisherBannerPtr GetPublisherBannerFromMessage(
    const publishers_pb::SiteBannerDetails& banner_details) {
  auto banner = ledger::mojom::PublisherBanner::New();

  banner->title = banner_details.title();
  banner->description = banner_details.description();

  if (!banner_details.background_url().empty()) {
    banner->background =
        "chrome://rewards-image/" + banner_details.background_url();
  }

  if (!banner_details.logo_url().empty()) {
    banner->logo = "chrome://rewards-image/" + banner_details.logo_url();
  }

  if (banner_details.has_social_links()) {
    auto& links = banner_details.social_links();
    if (!links.youtube().empty()) {
      banner->links.insert(std::make_pair("youtube", links.youtube()));
    }
    if (!links.twitter().empty()) {
      banner->links.insert(std::make_pair("twitter", links.twitter()));
    }
    if (!links.twitch().empty()) {
      banner->links.insert(std::make_pair("twitch", links.twitch()));
    }
  }

  return banner;
}

void GetPublisherStatusFromMessage(
    const publishers_pb::ChannelResponse& response,
    ledger::mojom::ServerPublisherInfo* info) {
  DCHECK(info);
  info->status = ledger::mojom::PublisherStatus::NOT_VERIFIED;
  for (const auto& wallet : response.wallets()) {
    if (wallet.has_uphold_wallet()) {
      auto& uphold = wallet.uphold_wallet();
      if (uphold.wallet_state() == publishers_pb::UPHOLD_ACCOUNT_KYC &&
          !uphold.address().empty()) {
        info->status = ledger::mojom::PublisherStatus::UPHOLD_VERIFIED;
        info->address = uphold.address();
        return;
      }
    }
    if (wallet.has_bitflyer_wallet()) {
      auto& bitflyer = wallet.bitflyer_wallet();
      if (bitflyer.wallet_state() == publishers_pb::BITFLYER_ACCOUNT_KYC &&
          !bitflyer.address().empty()) {
        info->status = ledger::mojom::PublisherStatus::BITFLYER_VERIFIED;
        info->address = bitflyer.address();
        return;
      }
    }
    if (wallet.has_gemini_wallet()) {
      auto& gemini = wallet.gemini_wallet();
      if (gemini.wallet_state() == publishers_pb::GEMINI_ACCOUNT_KYC &&
          !gemini.address().empty()) {
        info->status = ledger::mojom::PublisherStatus::GEMINI_VERIFIED;
        info->address = gemini.address();
        return;
      }
    }
  }
}

void GetServerInfoForEmptyResponse(const std::string& publisher_key,
                                   ledger::mojom::ServerPublisherInfo* info) {
  DCHECK(info);
  info->publisher_key = publisher_key;
  info->status = ledger::mojom::PublisherStatus::NOT_VERIFIED;
  info->updated_at = ledger::util::GetCurrentTimeStamp();
}

ledger::mojom::Result ServerPublisherInfoFromMessage(
    const publishers_pb::ChannelResponseList& message,
    const std::string& expected_key,
    ledger::mojom::ServerPublisherInfo* info) {
  DCHECK(info);

  if (expected_key.empty()) {
    return ledger::mojom::Result::LEDGER_ERROR;
  }

  for (const auto& entry : message.channel_responses()) {
    if (entry.channel_identifier() != expected_key) {
      continue;
    }

    info->publisher_key = entry.channel_identifier();
    info->updated_at = ledger::util::GetCurrentTimeStamp();
    GetPublisherStatusFromMessage(entry, info);

    if (entry.has_site_banner_details()) {
      info->banner = GetPublisherBannerFromMessage(entry.site_banner_details());
    }
    return ledger::mojom::Result::LEDGER_OK;
  }

  return ledger::mojom::Result::LEDGER_ERROR;
}

bool DecompressMessage(base::StringPiece payload, std::string* output) {
  constexpr size_t buffer_size = 32 * 1024;
  return ledger::util::DecodeBrotliStringWithBuffer(payload, buffer_size,
                                                    output);
}

}  // namespace

namespace ledger {
namespace endpoint {
namespace private_cdn {

GetPublisher::GetPublisher(LedgerImpl* ledger) : ledger_(ledger) {
  DCHECK(ledger_);
}

GetPublisher::~GetPublisher() = default;

std::string GetPublisher::GetUrl(const std::string& hash_prefix) {
  const std::string prefix = base::ToLowerASCII(hash_prefix);
  const std::string path =
      base::StringPrintf("/publishers/prefixes/%s", prefix.c_str());
  return GetServerUrl(path);
}

mojom::Result GetPublisher::CheckStatusCode(const int status_code) {
  if (status_code == net::HTTP_NOT_FOUND) {
    return mojom::Result::NOT_FOUND;
  }

  if (status_code != net::HTTP_OK) {
    BLOG(0, "Unexpected HTTP status: " << status_code);
    return mojom::Result::LEDGER_ERROR;
  }

  return mojom::Result::LEDGER_OK;
}

mojom::Result GetPublisher::ParseBody(const std::string& body,
                                      const std::string& publisher_key,
                                      mojom::ServerPublisherInfo* info) {
  DCHECK(info);

  if (body.empty()) {
    BLOG(0, "Publisher data empty");
    return mojom::Result::LEDGER_ERROR;
  }

  base::StringPiece body_payload(body.data(), body.size());
  if (!brave::PrivateCdnHelper::GetInstance()->RemovePadding(&body_payload)) {
    BLOG(0, "Publisher data response has invalid padding");
    return mojom::Result::LEDGER_ERROR;
  }

  std::string message_string;
  if (!DecompressMessage(body_payload, &message_string)) {
    BLOG(1,
         "Error decompressing publisher data response. "
         "Attempting to parse as uncompressed message.");
    message_string.assign(body_payload.data(), body_payload.size());
  }

  publishers_pb::ChannelResponseList message;
  if (!message.ParseFromString(message_string)) {
    BLOG(0, "Error parsing publisher data protobuf message");
    return mojom::Result::LEDGER_ERROR;
  }

  auto result = ServerPublisherInfoFromMessage(message, publisher_key, info);
  if (result != mojom::Result::LEDGER_OK) {
    GetServerInfoForEmptyResponse(publisher_key, info);
  }

  return mojom::Result::LEDGER_OK;
}

void GetPublisher::Request(const std::string& publisher_key,
                           const std::string& hash_prefix,
                           GetPublisherCallback callback) {
  auto url_callback =
      std::bind(&GetPublisher::OnRequest, this, _1, publisher_key, callback);

  auto request = mojom::UrlRequest::New();
  request->url = GetUrl(hash_prefix);
  request->load_flags = net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;
  ledger_->LoadURL(std::move(request), url_callback);
}

void GetPublisher::OnRequest(const mojom::UrlResponse& response,
                             const std::string& publisher_key,
                             GetPublisherCallback callback) {
  ledger::LogUrlResponse(__func__, response);
  auto result = CheckStatusCode(response.status_code);

  auto info = mojom::ServerPublisherInfo::New();
  if (result == mojom::Result::NOT_FOUND) {
    GetServerInfoForEmptyResponse(publisher_key, info.get());
    callback(mojom::Result::LEDGER_OK, std::move(info));
    return;
  }

  if (result != mojom::Result::LEDGER_OK) {
    callback(mojom::Result::LEDGER_ERROR, nullptr);
    return;
  }

  result = ParseBody(response.body, publisher_key, info.get());

  if (result != mojom::Result::LEDGER_OK) {
    callback(result, nullptr);
    return;
  }

  callback(mojom::Result::LEDGER_OK, std::move(info));
}

}  // namespace private_cdn
}  // namespace endpoint
}  // namespace ledger
