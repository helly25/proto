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

#include "mbo/proto/silent_error_collector.h"

#include <string>

#include "absl/strings/str_split.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace mbo::proto {

using ::testing::ElementsAreArray;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::NotNull;
using ::testing::SizeIs;

struct SilentErrorCollectorTest : ::testing::Test {
  // Simulate adding an error using an `ErrorCollector*`
  static void Error(int line, int column, const std::string& message, ::google::protobuf::io::ErrorCollector* ec) {
    ASSERT_THAT(ec, NotNull());
#if GOOGLE_PROTOBUF_VERSION >= 5'026'000
    ec->RecordError(line, column, message);
#else
    ec->AddError(line, column, message);
#endif
  }

  // Simulate adding a warning using an `ErrorCollector&`
  static void Warning(int line, int column, const std::string& message, ::google::protobuf::io::ErrorCollector& ec) {
#if GOOGLE_PROTOBUF_VERSION >= 5'026'000
    ec.RecordWarning(line, column, message);
#else
    ec.AddWarning(line, column, message);
#endif
  }
};

TEST_F(SilentErrorCollectorTest, Test) {
  SilentErrorCollector collector;
  EXPECT_THAT(collector, IsEmpty());
  EXPECT_THAT(collector, SizeIs(0));
  Error(25, 42, "error", collector);
  Warning(33, 99, "warning", collector);
  EXPECT_THAT(collector, Not(IsEmpty()));
  EXPECT_THAT(collector, SizeIs(2));
  EXPECT_THAT(
      collector,  // NL
      ElementsAreArray<SilentErrorCollector::ErrorInfo>({
          {.line = 25, .column = 42, .message = "error", .severity = absl::LogSeverity::kError},
          {.line = 33, .column = 99, .message = "warning", .severity = absl::LogSeverity::kWarning},
      }));
  EXPECT_THAT(
      absl::StrSplit(collector.GetErrors(), '\n'),  // NL
      ElementsAreArray({
          "Line 25, Col 42: error",
          "Line 33, Col 99: warning",
      }));
}

}  // namespace mbo::proto
