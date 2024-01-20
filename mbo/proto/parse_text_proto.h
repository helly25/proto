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
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/message.h"

namespace mbo::proto {
namespace internal {

void ParseTextOrDieInternal(
    std::string_view text,
    ::google::protobuf::Message* message,
    std::string_view func,
    std::source_location loc);

absl::Status ParseTextInternal(std::string_view text, ::google::protobuf::Message *message, std::source_location loc);

class ParseTextProtoHelper final {
 public:
  ~ParseTextProtoHelper() noexcept { ABSL_CHECK(parsed_) << "ParseTextProtoOrDie<T> result unused"; }

  ParseTextProtoHelper(std::string_view text_proto, std::source_location loc) noexcept
      : text_proto_(text_proto), loc_(loc)  {}

  // This is a purely temporary object... no copy or move may be used.
  ParseTextProtoHelper(const ParseTextProtoHelper&) noexcept = delete;
  ParseTextProtoHelper& operator=(const ParseTextProtoHelper&) noexcept = delete;
  ParseTextProtoHelper(ParseTextProtoHelper&&) noexcept = delete;
  ParseTextProtoHelper& operator=(ParseTextProtoHelper&&) noexcept = delete;

  template<typename T>
  requires(std::derived_from<T, ::google::protobuf::Message> && !std::same_as<T, ::google::protobuf::Message>)
  operator T() {  // NOLINT clangtidy(google-explicit-constructor)
    parsed_ = true;
    T message;
    ParseTextOrDieInternal(text_proto_, &message, "ParseTextProtoOrDie", loc_);
    return message;
  }

 private:
  const std::string text_proto_;
  const std::source_location loc_;
  bool parsed_{false};
};

}  // namespace internal

// Parses the text in 'text_proto' into the proto message type requested as return type.
// The function dies if parsing fails. Example:
//
// ```
// MyProtoType message = ParseTextProtoOrDie(R"pb(field: 42)pb");
// ```
inline internal::ParseTextProtoHelper ParseTextProtoOrDie(
    std::string_view text_proto,
    std::source_location loc = std::source_location::current()) {
  return {text_proto, loc};
}

// Parses the text in 'text_proto' into a proto message of type 'T' and return it wrapped as StatusOr.
// If parsing fails, then an error status will be returned.
template<typename T>
requires(std::derived_from<T, ::google::protobuf::Message> && !std::same_as<T, ::google::protobuf::Message>)
absl::StatusOr<T> ParseText(std::string_view text_proto, std::source_location loc = std::source_location::current()) {
  T message;
  absl::Status result = internal::ParseTextInternal(text_proto, &message, loc);
  if (!result.ok()) {
    return result;
  }
  return message;
}

// Parses the text in 'text_proto' into a proto message of type 'T'.
// The function dies if parsing fails.
// Use this function only if the return type cannot be determined automatically.
template<typename T>
requires(std::derived_from<T, ::google::protobuf::Message> && !std::same_as<T, ::google::protobuf::Message>)
T ParseTextOrDie(std::string_view text_proto, std::source_location loc = std::source_location::current()) {
  T message;
  internal::ParseTextOrDieInternal(text_proto, &message, "ParseTextOrDie", loc);
  return message;
}

}  // namespace mbo::proto

#endif  // MBO_PROTO_PARSE_TEXT_PROTO_H_
