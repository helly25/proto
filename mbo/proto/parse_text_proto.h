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

#ifndef MBO_PROTO_PARSE_TEXT_PROTO_H_
#define MBO_PROTO_PARSE_TEXT_PROTO_H_

#include <source_location>
#include <string>
#include <string_view>

#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/message.h"

namespace mbo::proto {
namespace internal {

absl::Status ParseTextInternal(std::string_view text_proto, ::google::protobuf::Message* message,
                               std::source_location loc);

}  // namespace internal

// Parses the text in 'text_proto' into a prot message of type 'T'.
// The function dies if parsing fails.
template <typename T>
T ParseTextOrDie(std::string_view text_proto,
                 std::source_location loc = std::source_location::current()) {
  T message;
  absl::Status result = internal::ParseTextInternal(text_proto, &message, loc);
  ABSL_CHECK_OK(result) << "ParseTextOrDie<" << T::GetDescriptor()->name() << ">"
      << " @" << loc.file_name() << ":" << loc.line() << ": " << result;
  return message;
}

class ParseTextProtoHelper final {
 public:
  ParseTextProtoHelper(std::string_view text_proto, std::source_location loc)
      : text_proto_(text_proto), loc_(loc), parsed_(false) {}

  ~ParseTextProtoHelper() {
    ABSL_CHECK(parsed_) << "ParseTextProtoOrDie<T> result unused";
  }

  template <typename T>
  operator T() {  // NOLINT clangtidy(google-explicit-constructor)
    parsed_ = true;
    return ParseTextOrDie<T>(text_proto_, loc_);
  }

 private:
  const std::string text_proto_;
  const std::source_location loc_;
  bool parsed_;
};

inline ParseTextProtoHelper ParseTextProtoOrDie(
    std::string_view text_proto,
    std::source_location loc = std::source_location::current()) {
  return ParseTextProtoHelper(text_proto, loc);
}

}  // namespace mbo::proto

#endif  // MBO_PROTO_PARSE_TEXT_PROTO_H_
