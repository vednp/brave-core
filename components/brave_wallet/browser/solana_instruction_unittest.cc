/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/solana_instruction.h"

#include <utility>
#include <vector>

#include "base/test/gtest_util.h"
#include "base/test/values_test_util.h"
#include "brave/components/brave_wallet/browser/solana_account_meta.h"
#include "brave/components/brave_wallet/browser/solana_instruction_data_decoder.h"
#include "brave/components/brave_wallet/common/brave_wallet_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace brave_wallet {

TEST(SolanaInstructionUnitTest, SerializeDeserialize) {
  // Test serializing an instruction for transfering SOL from an account to
  // itself.
  std::string from_account = "3Lu176FQzbQJCc8iL9PnmALbpMPhZeknoturApnXRDJw";
  std::string to_account = from_account;

  SolanaInstruction instruction(
      // Program ID
      mojom::kSolanaSystemProgramId,
      // Accounts
      {SolanaAccountMeta(from_account, true, true),
       SolanaAccountMeta(to_account, false, true)},
      // Data
      {2, 0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0});

  std::vector<SolanaAccountMeta> account_metas = {
      SolanaAccountMeta(from_account, true, true),
      SolanaAccountMeta(mojom::kSolanaSystemProgramId, false, false)};
  std::vector<uint8_t> bytes;
  ASSERT_TRUE(instruction.Serialize(account_metas, &bytes));
  std::vector<uint8_t> expected_bytes = {
      1,                                         // program id index
      2,                                         // length of accounts
      0,  0,                                     // account indices
      12,                                        // data length
      2,  0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0  // data
  };
  EXPECT_EQ(bytes, expected_bytes);

  // Deserialize and serialize again should get the same byte array.
  size_t bytes_index = 0;
  auto deserialized_instruction =
      SolanaInstruction::Deserialize(account_metas, bytes, &bytes_index);
  ASSERT_TRUE(deserialized_instruction);
  EXPECT_EQ(bytes_index, bytes.size());

  bytes.clear();
  ASSERT_TRUE(deserialized_instruction->Serialize(account_metas, &bytes));
  EXPECT_EQ(bytes, expected_bytes);

  // Program ID not found.
  EXPECT_FALSE(
      instruction.Serialize({SolanaAccountMeta(from_account, true, true),
                             SolanaAccountMeta(from_account, true, true)},
                            &bytes));

  // Account not found.
  EXPECT_FALSE(instruction.Serialize(
      {SolanaAccountMeta(mojom::kSolanaSystemProgramId, false, false),
       SolanaAccountMeta(mojom::kSolanaSystemProgramId, false, false)},
      &bytes));

  // Input account meta length > uint8_t max.
  std::vector<SolanaAccountMeta> oversize_account_metas = account_metas;
  for (uint16_t i = 1; i < 256; i++) {
    oversize_account_metas.push_back(account_metas[0]);
  }
  EXPECT_FALSE(instruction.Serialize(oversize_account_metas, &bytes));

  // Instruction account size > input account meta array size.
  EXPECT_FALSE(instruction.Serialize(
      {SolanaAccountMeta(from_account, true, true)}, &bytes));

  // Instruction account size > uint8_t max.
  SolanaInstruction invalid_instruction(
      // Program ID
      mojom::kSolanaSystemProgramId,
      // Accounts
      std::move(oversize_account_metas),
      // Data
      {2, 0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0});
  EXPECT_FALSE(invalid_instruction.Serialize(account_metas, &bytes));

  EXPECT_DCHECK_DEATH(instruction.Serialize({}, nullptr));
  EXPECT_DCHECK_DEATH(instruction.Deserialize(account_metas, bytes, nullptr));
}

TEST(SolanaInstructionUnitTest, FromToValue) {
  std::string from_account = "3Lu176FQzbQJCc8iL9PnmALbpMPhZeknoturApnXRDJw";
  std::string to_account = from_account;
  std::vector<uint8_t> data = {2, 0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0};

  SolanaInstruction instruction(
      // Program ID
      mojom::kSolanaSystemProgramId,
      // Accounts
      {SolanaAccountMeta(from_account, true, true),
       SolanaAccountMeta(to_account, false, true)},
      data);

  base::Value::Dict value = instruction.ToValue();
  auto expect_instruction_value = base::test::ParseJson(R"(
    {
      "program_id": "11111111111111111111111111111111",
      "accounts": [
        {
          "pubkey": "3Lu176FQzbQJCc8iL9PnmALbpMPhZeknoturApnXRDJw",
          "is_signer": true,
          "is_writable": true
        },
        {
          "pubkey": "3Lu176FQzbQJCc8iL9PnmALbpMPhZeknoturApnXRDJw",
          "is_signer": false,
          "is_writable": true
        }
      ],
      "data": "AgAAAICWmAAAAAAA",
      "decoded_data": {
        "account_params": [
          {
            "name": "from_account",
            "localized_name": "From Account",
          },
          {
            "name": "to_account",
            "localized_name": "To Account"
          }
        ],
        "params": [
          {
            "name": "lamports",
            "localized_name": "Lamports",
            "value": "10000000",
            "type": 2
          }
        ],
        "sys_ins_type": "2"
      }
    }
  )");
  EXPECT_EQ(value, expect_instruction_value.GetDict());

  auto instruction_from_value = SolanaInstruction::FromValue(value);
  EXPECT_EQ(instruction, instruction_from_value);

  std::vector<std::string> invalid_value_strings = {
      "{}", R"({"program_id": "program id", "accounts": []})",
      R"({"program_id": "program id", "data": ""})",
      R"({"accounts": [], "data": ""})"};

  for (const auto& invalid_value_string : invalid_value_strings) {
    auto invalid_value = base::test::ParseJson(invalid_value_string);
    EXPECT_FALSE(SolanaInstruction::FromValue(invalid_value.GetDict()))
        << ":" << invalid_value_string;
  }
}

TEST(SolanaInstructionUnitTest, FromMojomSolanaInstructions) {
  const std::string pubkey1 = "3Lu176FQzbQJCc8iL9PnmALbpMPhZeknoturApnXRDJw";
  const std::string pubkey2 = "83astBRguLMdt2h5U1Tpdq5tjFoJ6noeGwaY3mDLVcri";
  std::vector<uint8_t> data = {2, 0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0};
  mojom::SolanaAccountMetaPtr mojom_account_meta1 =
      mojom::SolanaAccountMeta::New(pubkey1, true, false);
  mojom::SolanaAccountMetaPtr mojom_account_meta2 =
      mojom::SolanaAccountMeta::New(pubkey2, false, true);
  std::vector<mojom::SolanaAccountMetaPtr> mojom_account_metas1;
  mojom_account_metas1.push_back(mojom_account_meta1.Clone());
  mojom_account_metas1.push_back(mojom_account_meta2.Clone());
  std::vector<mojom::SolanaAccountMetaPtr> mojom_account_metas2;
  mojom_account_metas2.push_back(mojom_account_meta2.Clone());
  mojom_account_metas2.push_back(mojom_account_meta1.Clone());

  std::vector<mojom::SolanaInstructionParamPtr> mojom_params;
  mojom_params.emplace_back(mojom::SolanaInstructionParam::New(
      "lamports", "Lamports", "10000000",
      mojom::SolanaInstructionParamType::kUint64));
  auto mojom_decoded_data = mojom::DecodedSolanaInstructionData::New(
      static_cast<uint32_t>(mojom::SolanaSystemInstruction::kTransfer),
      solana_ins_data_decoder::GetMojomAccountParamsForTesting(
          mojom::SolanaSystemInstruction::kTransfer, absl::nullopt),
      std::move(mojom_params));

  const std::string config_program =
      "Config1111111111111111111111111111111111111";
  mojom::SolanaInstructionPtr mojom_instruction1 =
      mojom::SolanaInstruction::New(mojom::kSolanaSystemProgramId,
                                    std::move(mojom_account_metas1), data,
                                    std::move(mojom_decoded_data));
  mojom::SolanaInstructionPtr mojom_instruction2 =
      mojom::SolanaInstruction::New(
          config_program, std::move(mojom_account_metas2), data, nullptr);
  std::vector<mojom::SolanaInstructionPtr> mojom_instructions;
  mojom_instructions.push_back(std::move(mojom_instruction1));
  mojom_instructions.push_back(std::move(mojom_instruction2));

  std::vector<SolanaInstruction> instructions;
  SolanaInstruction::FromMojomSolanaInstructions(mojom_instructions,
                                                 &instructions);
  EXPECT_EQ(std::vector<SolanaInstruction>(
                {SolanaInstruction(mojom::kSolanaSystemProgramId,
                                   {SolanaAccountMeta(pubkey1, true, false),
                                    SolanaAccountMeta(pubkey2, false, true)},
                                   data),
                 SolanaInstruction(config_program,
                                   {SolanaAccountMeta(pubkey2, false, true),
                                    SolanaAccountMeta(pubkey1, true, false)},
                                   data, absl::nullopt)}),
            instructions);
}

TEST(SolanaInstructionUnitTest, ToMojomSolanaInstruction) {
  const std::string pubkey1 = "3Lu176FQzbQJCc8iL9PnmALbpMPhZeknoturApnXRDJw";
  const std::string pubkey2 = "83astBRguLMdt2h5U1Tpdq5tjFoJ6noeGwaY3mDLVcri";
  std::vector<uint8_t> data = {2, 0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0};

  SolanaInstruction instruction(mojom::kSolanaSystemProgramId,
                                {SolanaAccountMeta(pubkey1, true, false),
                                 SolanaAccountMeta(pubkey2, false, true)},
                                data);

  auto mojom_instruction = instruction.ToMojomSolanaInstruction();
  ASSERT_TRUE(mojom_instruction);
  EXPECT_EQ(mojom_instruction->program_id, mojom::kSolanaSystemProgramId);
  EXPECT_EQ(mojom_instruction->account_metas.size(), 2u);
  EXPECT_EQ(mojom_instruction->account_metas[0],
            mojom::SolanaAccountMeta::New(pubkey1, true, false));
  EXPECT_EQ(mojom_instruction->account_metas[1],
            mojom::SolanaAccountMeta::New(pubkey2, false, true));
  EXPECT_EQ(mojom_instruction->data, data);
  std::vector<mojom::SolanaInstructionParamPtr> mojom_params;
  mojom_params.emplace_back(mojom::SolanaInstructionParam::New(
      "lamports", "Lamports", "10000000",
      mojom::SolanaInstructionParamType::kUint64));
  EXPECT_EQ(
      mojom_instruction->decoded_data,
      mojom::DecodedSolanaInstructionData::New(
          static_cast<uint32_t>(mojom::SolanaSystemInstruction::kTransfer),
          solana_ins_data_decoder::GetMojomAccountParamsForTesting(
              mojom::SolanaSystemInstruction::kTransfer, absl::nullopt),
          std::move(mojom_params)));
}

}  // namespace brave_wallet
