// SPDX-FileCopyrightText: Copyright (c) The helly25/mbo authors (helly25.com), The CPP Proto Builder Authors
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mbo/proto/file.h"

#include <filesystem>
#include <fstream>
#include <ios>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mbo/proto/matchers.h"
#include "mbo/proto/tests/simple_message.pb.h"

namespace mbo::proto {
namespace {
// NOLINTBEGIN(*-magic-numbers)

using ::mbo::proto::EqualsProto;
using ::mbo::proto::tests::SimpleMessage;
using ::testing::HasSubstr;

struct FileProtoTest : ::testing::Test {
  [[nodiscard]] static bool WriteFile(const std::filesystem::path& filename, std::string_view data) {
    std::ofstream output(filename, std::ios::binary);
    if (output.good()) {
      output << data;
      output.close();
    }
    return output.good();
  }
};

TEST_F(FileProtoTest, BinaryProto) {
  mbo::proto::tests::SimpleMessage message;
  message.set_one(25);
  message.add_two(33);
  message.add_two(42);
  ASSERT_TRUE(WriteBinaryProtoFile("test.pb", message).ok());
  const auto result = ReadBinaryProtoFile<SimpleMessage>("test.pb");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, EqualsProto(message));
}

TEST_F(FileProtoTest, BinaryProtoError) {
  {
    const auto result = ReadBinaryProtoFile<SimpleMessage>("DoesNotExist.pb");
    ASSERT_FALSE(result.ok());
    EXPECT_THAT(result.status().code(), absl::StatusCode::kNotFound);
    EXPECT_THAT(result.status().message(), HasSubstr("Cannot open 'DoesNotExist.pb'"));
  }
  {
    ASSERT_TRUE(WriteFile("SomeFile.pb", "\xFF"));
    const auto result = ReadBinaryProtoFile<SimpleMessage>("SomeFile.pb");
    ASSERT_FALSE(result.ok());
    EXPECT_THAT(result.status().code(), absl::StatusCode::kAborted);
    EXPECT_THAT(result.status().message(), HasSubstr("Cannot parse binary proto file 'SomeFile.pb'"));
  }
}

TEST_F(FileProtoTest, TextProto) {
  mbo::proto::tests::SimpleMessage message;
  message.set_one(25);
  message.add_two(33);
  message.add_two(42);
  ASSERT_TRUE(WriteTextProtoFile("test.textproto", message).ok());
  const auto result = ReadTextProtoFile<SimpleMessage>("test.textproto");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, EqualsProto(message));
}

TEST_F(FileProtoTest, TextProtoError) {
  {
    const auto result = ReadTextProtoFile<SimpleMessage>("DoesNotExist.textproto");
    ASSERT_FALSE(result.ok());
    EXPECT_THAT(result.status().code(), absl::StatusCode::kNotFound);
    EXPECT_THAT(result.status().message(), HasSubstr("Cannot open 'DoesNotExist.textproto'"));
  }
  {
    ASSERT_TRUE(WriteFile("SomeFile.textproto", "\xFF"));
    const auto result = ReadTextProtoFile<SimpleMessage>("SomeFile.textproto");
    ASSERT_FALSE(result.ok());
    EXPECT_THAT(result.status().code(), absl::StatusCode::kAborted);
    EXPECT_THAT(result.status().message(), HasSubstr("Cannot parse text proto file 'SomeFile.textproto'"));
  }
}

// NOLINTEND(*-magic-numbers)
}  // namespace
}  // namespace mbo::proto
