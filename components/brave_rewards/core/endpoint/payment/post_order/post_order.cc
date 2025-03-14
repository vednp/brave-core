/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_rewards/core/endpoint/payment/post_order/post_order.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "brave/components/brave_rewards/core/endpoint/payment/payment_util.h"
#include "brave/components/brave_rewards/core/ledger_impl.h"
#include "net/http/http_status_code.h"

using std::placeholders::_1;

namespace ledger {
namespace endpoint {
namespace payment {

PostOrder::PostOrder(LedgerImpl* ledger) : ledger_(ledger) {
  DCHECK(ledger_);
}

PostOrder::~PostOrder() = default;

std::string PostOrder::GetUrl() {
  return GetServerUrl("/v1/orders");
}

std::string PostOrder::GeneratePayload(
    const std::vector<mojom::SKUOrderItem>& items) {
  base::Value::List order_items;
  for (const auto& item : items) {
    base::Value::Dict order_item;
    order_item.Set("sku", item.sku);
    order_item.Set("quantity", item.quantity);
    order_items.Append(std::move(order_item));
  }

  base::Value::Dict body;
  body.Set("items", std::move(order_items));

  std::string json;
  base::JSONWriter::Write(body, &json);

  return json;
}

mojom::Result PostOrder::CheckStatusCode(const int status_code) {
  if (status_code == net::HTTP_BAD_REQUEST) {
    BLOG(0, "Invalid request");
    return mojom::Result::RETRY_SHORT;
  }

  if (status_code == net::HTTP_INTERNAL_SERVER_ERROR) {
    BLOG(0, "Internal server error");
    return mojom::Result::RETRY_SHORT;
  }

  if (status_code != net::HTTP_CREATED) {
    BLOG(0, "Unexpected HTTP status: " << status_code);
    return mojom::Result::LEDGER_ERROR;
  }

  return mojom::Result::LEDGER_OK;
}

mojom::Result PostOrder::ParseBody(
    const std::string& body,
    const std::vector<mojom::SKUOrderItem>& order_items,
    mojom::SKUOrder* order) {
  DCHECK(order);

  absl::optional<base::Value> dictionary = base::JSONReader::Read(body);
  if (!dictionary || !dictionary->is_dict()) {
    BLOG(0, "Invalid JSON");
    return mojom::Result::LEDGER_ERROR;
  }

  const auto* id = dictionary->FindStringKey("id");
  if (id) {
    order->order_id = *id;
  }

  if (order->order_id.empty()) {
    BLOG(0, "Order id empty");
    return mojom::Result::LEDGER_ERROR;
  }

  const auto* total_amount = dictionary->FindStringKey("totalPrice");
  if (total_amount) {
    const bool success =
        base::StringToDouble(*total_amount, &order->total_amount);
    if (!success) {
      order->total_amount = 0.0;
    }
  }

  const auto* merchant_id = dictionary->FindStringKey("merchantId");
  if (merchant_id) {
    order->merchant_id = *merchant_id;
  }

  const auto* location = dictionary->FindStringKey("location");
  if (location) {
    order->location = *location;
  }

  order->status = mojom::SKUOrderStatus::PENDING;

  auto* items = dictionary->FindListKey("items");
  if (!items) {
    return mojom::Result::LEDGER_OK;
  }

  if (items->GetList().size() != order_items.size()) {
    BLOG(0, "Invalid JSON");
    return mojom::Result::LEDGER_ERROR;
  }

  int count = 0;
  for (auto& item : items->GetList()) {
    auto order_item = mojom::SKUOrderItem::New();
    order_item->order_id = order->order_id;
    order_item->sku = order_items[count].sku;
    order_item->type = order_items[count].type;

    const auto* order_item_id = item.FindStringKey("id");
    if (order_item_id) {
      order_item->order_item_id = *order_item_id;
    }

    const auto quantity = item.FindIntKey("quantity");
    if (quantity) {
      order_item->quantity = *quantity;
    }

    const auto* price = item.FindStringKey("price");
    if (price) {
      const bool success = base::StringToDouble(*price, &order_item->price);
      if (!success) {
        order_item->price = 0.0;
      }
    }

    const auto* name = item.FindStringKey("name");
    if (name) {
      order_item->name = *name;
    }

    const auto* description = item.FindStringKey("description");
    if (description) {
      order_item->description = *description;
    }

    order->items.push_back(std::move(order_item));

    count++;
  }

  return mojom::Result::LEDGER_OK;
}

void PostOrder::Request(const std::vector<mojom::SKUOrderItem>& items,
                        PostOrderCallback callback) {
  auto url_callback =
      std::bind(&PostOrder::OnRequest, this, _1, items, callback);

  auto request = mojom::UrlRequest::New();
  request->url = GetUrl();
  request->content = GeneratePayload(items);
  request->content_type = "application/json; charset=utf-8";
  request->method = mojom::UrlMethod::POST;
  ledger_->LoadURL(std::move(request), url_callback);
}

void PostOrder::OnRequest(const mojom::UrlResponse& response,
                          const std::vector<mojom::SKUOrderItem>& items,
                          PostOrderCallback callback) {
  ledger::LogUrlResponse(__func__, response);

  mojom::Result result = CheckStatusCode(response.status_code);

  if (result != mojom::Result::LEDGER_OK) {
    callback(result, nullptr);
    return;
  }

  auto order = mojom::SKUOrder::New();
  result = ParseBody(response.body, items, order.get());
  callback(result, std::move(order));
}

}  // namespace payment
}  // namespace endpoint
}  // namespace ledger
