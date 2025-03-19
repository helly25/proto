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

#ifndef MBO_PROTO_FILE_H_
#define MBO_PROTO_FILE_H_

#include <filesystem>
#include <source_location>
#include <string_view>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "google/protobuf/message.h"

namespace mbo::proto {

template<typename ProtoType>
concept IsProtoType =
    std::derived_from<ProtoType, ::google::protobuf::Message> && !std::same_as<ProtoType, ::google::protobuf::Message>;

namespace proto_internal {

absl::Status ReadBinaryProtoFile(
    const std::filesystem::path& filename,
    ::google::protobuf::Message& result,
    const std::source_location& src_loc);

absl::Status ReadTextProtoFile(
    const std::filesystem::path& filename,
    ::google::protobuf::Message& result,
    const std::source_location& src_loc);

}  // namespace proto_internal

template<IsProtoType ProtoType>
absl::StatusOr<ProtoType> ReadBinaryProtoFile(
    const std::filesystem::path& filename,
    const std::source_location& src_loc = std::source_location::current()) {
  ProtoType result;
  const auto status = proto_internal::ReadBinaryProtoFile(filename, result, src_loc);
  if (!status.ok()) {
    return status;
  }
  return result;
}

absl::Status WriteBinaryProtoFile(
    const std::filesystem::path& filename,
    const ::google::protobuf::Message& proto,
    const std::source_location& src_loc = std::source_location::current());

template<IsProtoType ProtoType>
absl::StatusOr<ProtoType> ReadTextProtoFile(
    std::string_view filename,
    const std::source_location& src_loc = std::source_location::current()) {
  ProtoType result;
  const auto status = proto_internal::ReadTextProtoFile(filename, result, src_loc);
  if (!status.ok()) {
    return status;
  }
  return result;
}

absl::Status WriteTextProtoFile(
    const std::filesystem::path& filename,
    const ::google::protobuf::Message& proto,
    const std::source_location& src_loc = std::source_location::current());

}  // namespace mbo::proto

#endif  // MBO_PROTO_FILE_H_
