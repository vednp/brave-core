/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_rewards/core/test/test_ledger_client.h"

#include <utility>

#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/json/values_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/ranges/algorithm.h"
#include "base/strings/escape.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/task/sequenced_task_runner.h"
#include "net/http/http_status_code.h"

namespace ledger {

std::string FakeEncryption::EncryptString(const std::string& value) {
  return "ENCRYPTED:" + value;
}

absl::optional<std::string> FakeEncryption::DecryptString(
    const std::string& value) {
  size_t pos = value.find("ENCRYPTED:");
  if (pos == 0) {
    return value.substr(10);
  }

  return {};
}

std::string FakeEncryption::Base64EncryptString(const std::string& value) {
  std::string fake_encrypted = EncryptString(value);
  std::string encoded;
  base::Base64Encode(fake_encrypted, &encoded);
  return encoded;
}

absl::optional<std::string> FakeEncryption::Base64DecryptString(
    const std::string& value) {
  std::string decoded;
  if (!base::Base64Decode(value, &decoded)) {
    return {};
  }

  return DecryptString(decoded);
}

TestNetworkResult::TestNetworkResult(const std::string& url,
                                     mojom::UrlMethod method,
                                     mojom::UrlResponsePtr response)
    : url(url), method(method), response(std::move(response)) {}

TestNetworkResult::~TestNetworkResult() = default;

base::FilePath GetTestDataPath() {
  base::FilePath path;
  base::PathService::Get(base::DIR_SOURCE_ROOT, &path);
  path = path.AppendASCII("brave");
  path = path.AppendASCII("components");
  path = path.AppendASCII("brave_rewards");
  path = path.AppendASCII("core");
  path = path.AppendASCII("test");
  path = path.AppendASCII("data");
  return path;
}

TestLedgerClient::TestLedgerClient() : ledger_database_(base::FilePath()) {
  CHECK(ledger_database_.GetInternalDatabaseForTesting()->OpenInMemory());
}

TestLedgerClient::~TestLedgerClient() = default;

void TestLedgerClient::OnReconcileComplete(
    const mojom::Result result,
    mojom::ContributionInfoPtr contribution) {}

void TestLedgerClient::LoadLedgerState(client::OnLoadCallback callback) {
  callback(mojom::Result::NO_LEDGER_STATE, "");
}

void TestLedgerClient::LoadPublisherState(client::OnLoadCallback callback) {
  callback(mojom::Result::NO_PUBLISHER_STATE, "");
}

void TestLedgerClient::OnPanelPublisherInfo(
    mojom::Result result,
    mojom::PublisherInfoPtr publisher_info,
    uint64_t windowId) {}

void TestLedgerClient::OnPublisherRegistryUpdated() {}

void TestLedgerClient::OnPublisherUpdated(const std::string& publisher_id) {}

void TestLedgerClient::FetchFavIcon(const std::string& url,
                                    const std::string& favicon_key,
                                    client::FetchIconCallback callback) {
  callback(true, favicon_key);
}

std::string TestLedgerClient::URIEncode(const std::string& value) {
  return base::EscapeQueryParamValue(value, false);
}

void TestLedgerClient::LoadURL(mojom::UrlRequestPtr request,
                               client::LoadURLCallback callback) {
  DCHECK(request);
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(&TestLedgerClient::LoadURLAfterDelay,
                                weak_factory_.GetWeakPtr(), std::move(request),
                                std::move(callback)));
}

void TestLedgerClient::Log(const char* file,
                           const int line,
                           const int verbose_level,
                           const std::string& message) {
  int vlog_level = logging::GetVlogLevelHelper(file, strlen(file));
  if (verbose_level <= vlog_level) {
    logging::LogMessage(file, line, -verbose_level).stream() << message;
  }

  if (log_callback_) {
    log_callback_.Run(message);
  }
}

void TestLedgerClient::PublisherListNormalized(
    std::vector<mojom::PublisherInfoPtr> list) {}

void TestLedgerClient::SetBooleanState(const std::string& name, bool value) {
  state_store_.SetByDottedPath(name, value);
}

bool TestLedgerClient::GetBooleanState(const std::string& name) const {
  return state_store_.FindBoolByDottedPath(name).value_or(false);
}

void TestLedgerClient::SetIntegerState(const std::string& name, int value) {
  state_store_.SetByDottedPath(name, value);
}

int TestLedgerClient::GetIntegerState(const std::string& name) const {
  return state_store_.FindIntByDottedPath(name).value_or(0);
}

void TestLedgerClient::SetDoubleState(const std::string& name, double value) {
  state_store_.SetByDottedPath(name, value);
}

double TestLedgerClient::GetDoubleState(const std::string& name) const {
  return state_store_.FindDoubleByDottedPath(name).value_or(0.0);
}

void TestLedgerClient::SetStringState(const std::string& name,
                                      const std::string& value) {
  state_store_.SetByDottedPath(name, value);
}

std::string TestLedgerClient::GetStringState(const std::string& name) const {
  const auto* value = state_store_.FindStringByDottedPath(name);
  return value ? *value : base::EmptyString();
}

void TestLedgerClient::SetInt64State(const std::string& name, int64_t value) {
  state_store_.SetByDottedPath(name, base::NumberToString(value));
}

int64_t TestLedgerClient::GetInt64State(const std::string& name) const {
  if (const std::string* opt = state_store_.FindStringByDottedPath(name)) {
    int64_t value;
    if (base::StringToInt64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

void TestLedgerClient::SetUint64State(const std::string& name, uint64_t value) {
  state_store_.SetByDottedPath(name, base::NumberToString(value));
}

uint64_t TestLedgerClient::GetUint64State(const std::string& name) const {
  if (const std::string* opt = state_store_.FindStringByDottedPath(name)) {
    uint64_t value;
    if (base::StringToUint64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

void TestLedgerClient::SetValueState(const std::string& name,
                                     base::Value value) {
  state_store_.SetByDottedPath(name, std::move(value));
}

base::Value TestLedgerClient::GetValueState(const std::string& name) const {
  const auto* value = state_store_.FindByDottedPath(name);
  return value ? value->Clone() : base::Value();
}

void TestLedgerClient::SetTimeState(const std::string& name, base::Time time) {
  state_store_.SetByDottedPath(name, base::TimeToValue(time));
}

base::Time TestLedgerClient::GetTimeState(const std::string& name) const {
  const auto* value = state_store_.FindByDottedPath(name);
  DCHECK(value);
  if (!value) {
    return base::Time();
  }

  auto time = base::ValueToTime(*value);
  DCHECK(time);
  return time.value_or(base::Time());
}

void TestLedgerClient::ClearState(const std::string& name) {
  state_store_.RemoveByDottedPath(name);
}

bool TestLedgerClient::GetBooleanOption(const std::string& name) const {
  return option_store_.FindBoolByDottedPath(name).value_or(false);
}

int TestLedgerClient::GetIntegerOption(const std::string& name) const {
  return option_store_.FindIntByDottedPath(name).value_or(0);
}

double TestLedgerClient::GetDoubleOption(const std::string& name) const {
  return option_store_.FindDoubleByDottedPath(name).value_or(0.0);
}

std::string TestLedgerClient::GetStringOption(const std::string& name) const {
  const auto* value = option_store_.FindStringByDottedPath(name);
  return value ? *value : base::EmptyString();
}

int64_t TestLedgerClient::GetInt64Option(const std::string& name) const {
  if (const std::string* opt = option_store_.FindStringByDottedPath(name)) {
    int64_t value;
    if (base::StringToInt64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

uint64_t TestLedgerClient::GetUint64Option(const std::string& name) const {
  if (const std::string* opt = option_store_.FindStringByDottedPath(name)) {
    uint64_t value;
    if (base::StringToUint64(*opt, &value)) {
      return value;
    }
  }
  return 0;
}

void TestLedgerClient::OnContributeUnverifiedPublishers(
    mojom::Result result,
    const std::string& publisher_key,
    const std::string& publisher_name) {}

std::string TestLedgerClient::GetLegacyWallet() {
  return "";
}

void TestLedgerClient::ShowNotification(const std::string& type,
                                        const std::vector<std::string>& args,
                                        client::LegacyResultCallback callback) {
}

mojom::ClientInfoPtr TestLedgerClient::GetClientInfo() {
  auto info = mojom::ClientInfo::New();
  info->platform = mojom::Platform::DESKTOP;
  info->os = mojom::OperatingSystem::UNDEFINED;
  return info;
}

void TestLedgerClient::UnblindedTokensReady() {}

void TestLedgerClient::ReconcileStampReset() {}

void TestLedgerClient::RunDBTransaction(
    mojom::DBTransactionPtr transaction,
    client::RunDBTransactionCallback callback) {
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(&TestLedgerClient::RunDBTransactionAfterDelay,
                                weak_factory_.GetWeakPtr(),
                                std::move(transaction), std::move(callback)));
}

void TestLedgerClient::GetCreateScript(
    client::GetCreateScriptCallback callback) {
  callback("", 0);
}

void TestLedgerClient::PendingContributionSaved(const mojom::Result result) {}

void TestLedgerClient::ClearAllNotifications() {}

void TestLedgerClient::ExternalWalletConnected() const {}

void TestLedgerClient::ExternalWalletLoggedOut() const {}

void TestLedgerClient::ExternalWalletReconnected() const {}

void TestLedgerClient::DeleteLog(client::LegacyResultCallback callback) {
  callback(mojom::Result::LEDGER_OK);
}

absl::optional<std::string> TestLedgerClient::EncryptString(
    const std::string& value) {
  return FakeEncryption::EncryptString(value);
}

absl::optional<std::string> TestLedgerClient::DecryptString(
    const std::string& value) {
  return FakeEncryption::DecryptString(value);
}

void TestLedgerClient::SetOptionForTesting(const std::string& name,
                                           base::Value value) {
  option_store_.SetByDottedPath(name, std::move(value));
}

void TestLedgerClient::AddNetworkResultForTesting(
    const std::string& url,
    mojom::UrlMethod method,
    mojom::UrlResponsePtr response) {
  network_results_.emplace_back(url, method, std::move(response));
}

void TestLedgerClient::SetLogCallbackForTesting(LogCallback callback) {
  log_callback_ = std::move(callback);
}

void TestLedgerClient::LoadURLAfterDelay(mojom::UrlRequestPtr request,
                                         client::LoadURLCallback callback) {
  auto iter = base::ranges::find_if(network_results_, [&request](auto& result) {
    return request->url == result.url && request->method == result.method;
  });

  if (iter != network_results_.end()) {
    std::move(callback).Run(*iter->response);
    network_results_.erase(iter);
    return;
  }

  LOG(INFO) << "Test network result not found for " << request->method << ":"
            << request->url;

  mojom::UrlResponse response;
  response.url = request->url;
  response.status_code = net::HTTP_BAD_REQUEST;
  std::move(callback).Run(response);
}

void TestLedgerClient::RunDBTransactionAfterDelay(
    mojom::DBTransactionPtr transaction,
    client::RunDBTransactionCallback callback) {
  auto response = ledger_database_.RunTransaction(std::move(transaction));
  std::move(callback).Run(std::move(response));
}

}  // namespace ledger
