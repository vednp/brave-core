/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/token.h"

#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/blinded_token.h"
#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/challenge_bypass_ristretto_util.h"

namespace brave_ads::privacy::cbr {

namespace {

absl::optional<challenge_bypass_ristretto::Token> Create(
    const std::string& token_base64) {
  if (token_base64.empty()) {
    return absl::nullopt;
  }

  return ValueOrLogError(
      challenge_bypass_ristretto::Token::decode_base64(token_base64));
}

}  // namespace

Token::Token()
    : token_(ValueOrLogError(challenge_bypass_ristretto::Token::random())) {}

Token::Token(const std::string& token_base64) : token_(Create(token_base64)) {}

Token::Token(const Token& other) = default;

Token& Token::operator=(const Token& other) = default;

Token::Token(Token&& other) noexcept = default;

Token& Token::operator=(Token&& other) noexcept = default;

Token::~Token() = default;

bool Token::operator==(const Token& other) const {
  return EncodeBase64().value_or("") == other.EncodeBase64().value_or("");
}

bool Token::operator!=(const Token& other) const {
  return !(*this == other);
}

Token Token::DecodeBase64(const std::string& token_base64) {
  return Token(token_base64);
}

absl::optional<std::string> Token::EncodeBase64() const {
  if (!token_ || !has_value()) {
    return absl::nullopt;
  }

  return ValueOrLogError(token_->encode_base64());
}

absl::optional<BlindedToken> Token::Blind() {
  if (!token_ || !has_value()) {
    return absl::nullopt;
  }

  return ValueOrLogError<challenge_bypass_ristretto::BlindedToken,
                         BlindedToken>(token_->blind());
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  os << token.EncodeBase64().value_or("");
  return os;
}

}  // namespace brave_ads::privacy::cbr
