// Copyright 2021 The CPP Proto Builder Authors
// Copyright 2023 M.Boerger
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

#include "mbo/proto/parse_text_proto.h"

#include "gmock/gmock.h"
#include "mbo/proto/matchers.h"
#include "mbo/proto/tests/simple_message.pb.h"

namespace mbo::proto {
namespace {

using ::mbo::proto::EqualsProto;
using ::mbo::proto::tests::SimpleMessage;
using ::testing::ContainsRegex;

class ParseTextProtoTest : public ::testing::Test {};

TEST_F(ParseTextProtoTest, ParseTextOrDiePass) {
  EXPECT_THAT(ParseTextOrDie<SimpleMessage>(""), EqualsProto(""));
  EXPECT_THAT(ParseTextOrDie<SimpleMessage>("one: 42"), EqualsProto("one: 42"));
}

TEST_F(ParseTextProtoTest, ParseTextOrDieFail) {
  EXPECT_DEATH(
      ParseTextOrDie<SimpleMessage>("!!!"),
      // Using [0-9] in lieu of \\d to be compatible in open source.
      ".*Check failed: "                                   // Prefix
      "INVALID_ARGUMENT: "                                 // Status
      "ParseTextOrDie<SimpleMessage>.*\n.*"                // Called function
      "File: '.*/parse_text_proto.*', Line: [0-9]+.*\n.*"  // Source
      "Line 0, Col 0: Expected identifier, got: !.*");     // Error
}

TEST_F(ParseTextProtoTest, ParseTextProtoOrDiePass) {
  EXPECT_THAT((SimpleMessage)ParseTextProtoOrDie("one: 25"), EqualsProto("one: 25"));
}

TEST_F(ParseTextProtoTest, ParseTextProtoOrDieFail) {
  EXPECT_DEATH(
      SimpleMessage msg = ParseTextProtoOrDie("!!!"),
      // Using [0-9] in lieu of \\d to be compatible in open source.
      ".*Check failed: "                                   // Prefix
      "INVALID_ARGUMENT: "                                 // StatusCode
      "ParseTextProtoOrDie<SimpleMessage>.*\n.*"           // Called function
      "File: '.*/parse_text_proto.*', Line: [0-9]+.*\n.*"  // Source
      "Line 0, Col 0: Expected identifier, got: !.*");     // Error
}

TEST_F(ParseTextProtoTest, ParseText) {
  EXPECT_THAT(*ParseText<SimpleMessage>("one: 25"), EqualsProto("one: 25"));
  const absl::StatusOr<SimpleMessage> status = ParseText<SimpleMessage>("!!!");
  static constexpr std::string_view expected =  //
                                                // Using [0-9] in lieu of \\d to be compatible in open source.
      "ParseText<SimpleMessage>.*\n.*"                     // Called function
      "File: '.*/parse_text_proto.*', Line: [0-9]+.*\n.*"  // Source
      "Line 0, Col 0: Expected identifier, got: !.*";      // Error
  ASSERT_THAT(status.status().ok(), false);
  EXPECT_THAT(status.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.status().message(), ContainsRegex(expected));
  EXPECT_DEATH(
      auto msg = *ParseText<SimpleMessage>("!!!"),
      absl::StrCat(
          "RAW: Attempting to fetch value instead of handling error ",  // Prefix
          "INVALID_ARGUMENT: ",                                         // StatusCode
          expected));
}

}  // namespace
}  // namespace mbo::proto
