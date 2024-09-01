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

#include <source_location>
#include <string>
#include <vector>

#include "absl/base/log_severity.h"
#include "absl/log/absl_log.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/io/tokenizer.h"
#include "google/protobuf/text_format.h"

namespace mbo::proto::proto_internal {
namespace {

// Collects errors from proto parsing.
class SilentErrorCollector : public google::protobuf::io::ErrorCollector {
 public:
  struct ErrorInfo {
    int line = 0;
    int column = 0;
    std::string message;
    absl::LogSeverity severity = absl::LogSeverity::kError;
  };

#if GOOGLE_PROTOBUF_VERSION >= 5'026'000
  void RecordError(int line, int column, std::string_view message) override {
#else
  void AddError(int line, int column, const std::string& message) override {
#endif
    errors_.push_back(
        {.line = line, .column = column, .message = std::string(message), .severity = absl::LogSeverity::kError});
  }

#if GOOGLE_PROTOBUF_VERSION >= 5'026'000
  void RecordWarning(int line, int column, std::string_view message) override {
#else
  void AddWarning(int line, int column, const std::string& message) override {
#endif
    errors_.push_back(
        {.line = line, .column = column, .message = std::string(message), .severity = absl::LogSeverity::kWarning});
  }

  std::string GetErrors() const;

  const std::vector<ErrorInfo>& errors() const { return errors_; }

 private:
  std::vector<ErrorInfo> errors_;
};

std::string SilentErrorCollector::GetErrors() const {
  std::string result;
  for (const auto& error : errors()) {
    absl::StrAppendFormat(&result, "Line %d, Col %d: %s\n", error.line, error.column, error.message);
  }
  return result;
}

}  // namespace

absl::Status ParseTextInternal(
    std::string_view text_proto,
    ::google::protobuf::Message* message,
    std::string_view func,
    std::source_location loc) {
  google::protobuf::TextFormat::Parser parser;
  SilentErrorCollector error_collector;
  parser.RecordErrorsTo(&error_collector);
  if (parser.ParseFromString(std::string(text_proto), message)) {
    return absl::OkStatus();
  }
  return absl::InvalidArgumentError(absl::StrFormat(
      "%s<%s>\nFile: '%s', Line: %d: %s\nError: %s", func, message->GetDescriptor()->name(), loc.file_name(),
      loc.line(), loc.function_name(), error_collector.GetErrors()));
}

void ParseTextOrDieInternal(
    std::string_view text,
    ::google::protobuf::Message* message,
    std::string_view func,
    std::source_location loc) {
  const absl::Status status = ParseTextInternal(text, message, func, loc);
  if (!status.ok()) {
    ABSL_LOG(FATAL) << "Check failed: " << status;
  }
}

}  // namespace mbo::proto::proto_internal
