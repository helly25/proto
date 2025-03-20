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

#include "absl/log/absl_log.h"
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

class ReadBinaryProtoFile {
 public:
  // Static read function - so it's address can be taken.
  template<IsProtoType ProtoType>
  static absl::StatusOr<ProtoType> As(
      const std::filesystem::path& filename,
      const std::source_location& src_loc = std::source_location::current()) {
    ProtoType result;
    const auto status = proto_internal::ReadBinaryProtoFile(filename, result, src_loc);
    if (!status.ok()) {
      return status;
    }
    return result;
  }

  // Static read function - so it's address can be taken.
  template<IsProtoType ProtoType>
  static ProtoType OrDie(
      const std::filesystem::path& filename,
      const std::source_location& src_loc = std::source_location::current()) {
    return *As<ProtoType>(filename, src_loc);
  }

  ReadBinaryProtoFile() = delete;

  explicit ReadBinaryProtoFile(
      std::filesystem::path filename,
      const std::source_location& src_loc = std::source_location::current())
      : filename_(std::move(filename)), src_loc_(src_loc) {}

  ReadBinaryProtoFile(const ReadBinaryProtoFile&) = delete;
  ReadBinaryProtoFile& operator=(const ReadBinaryProtoFile&) = delete;
  ReadBinaryProtoFile(ReadBinaryProtoFile&&) = delete;
  ReadBinaryProtoFile& operator=(ReadBinaryProtoFile&&) = delete;

  ~ReadBinaryProtoFile() { ABSL_LOG_IF(DFATAL, !converted_) << "ReadBinaryProtoFile has not read the file."; }

  template<int&..., IsProtoType ProtoType>
  operator absl::StatusOr<ProtoType>() const {  // NOLINT(*-explicit-*)
    converted_ = true;
    ProtoType result;
    const auto status = proto_internal::ReadBinaryProtoFile(filename_, result, src_loc_);
    if (!status.ok()) {
      return status;
    }
    return result;
  }

  template<int&..., IsProtoType ProtoType>
  operator ProtoType() const {  // NOLINT(*-explicit-*)
    absl::StatusOr<ProtoType> proto = *this;
    ABSL_LOG_IF(FATAL, !proto.ok()).AtLocation(src_loc_.file_name(), static_cast<int>(src_loc_.line()))
        << proto.status();
    return *proto;
  }

 private:
  const std::filesystem::path filename_;
  const std::source_location src_loc_;
  mutable bool converted_ = false;
};

absl::Status WriteBinaryProtoFile(
    const std::filesystem::path& filename,
    const ::google::protobuf::Message& proto,
    const std::source_location& src_loc = std::source_location::current());

class ReadTextProtoFile {
 public:
  // Static read function - so it's address can be taken.
  template<IsProtoType ProtoType>
  static absl::StatusOr<ProtoType> As(
      const std::filesystem::path& filename,
      const std::source_location& src_loc = std::source_location::current()) {
    ProtoType result;
    const auto status = proto_internal::ReadTextProtoFile(filename, result, src_loc);
    if (!status.ok()) {
      return status;
    }
    return result;
  }

  // Static read function - so it's address can be taken.
  template<IsProtoType ProtoType>
  static ProtoType OrDie(
      const std::filesystem::path& filename,
      const std::source_location& src_loc = std::source_location::current()) {
    return *As<ProtoType>(filename, src_loc);
  }

  ReadTextProtoFile() = delete;

  explicit ReadTextProtoFile(
      std::filesystem::path filename,
      const std::source_location& src_loc = std::source_location::current())
      : filename_(std::move(filename)), src_loc_(src_loc) {}

  ReadTextProtoFile(const ReadTextProtoFile&) = delete;
  ReadTextProtoFile& operator=(const ReadTextProtoFile&) = delete;
  ReadTextProtoFile(ReadTextProtoFile&&) = delete;
  ReadTextProtoFile& operator=(ReadTextProtoFile&&) = delete;

  ~ReadTextProtoFile() { ABSL_LOG_IF(DFATAL, !converted_) << "ReadTextProtoFile has not read the file."; }

  template<int&..., IsProtoType ProtoType>
  operator absl::StatusOr<ProtoType>() const {  // NOLINT(*-explicit-*)
    converted_ = true;
    ProtoType result;
    const auto status = proto_internal::ReadTextProtoFile(filename_, result, src_loc_);
    if (!status.ok()) {
      return status;
    }
    return result;
  }

  template<int&..., IsProtoType ProtoType>
  operator ProtoType() const {  // NOLINT(*-explicit-*)
    absl::StatusOr<ProtoType> proto = *this;
    ABSL_LOG_IF(FATAL, !proto.ok()).AtLocation(src_loc_.file_name(), static_cast<int>(src_loc_.line()))
        << proto.status();
    return *proto;
  }

 private:
  const std::filesystem::path filename_;
  const std::source_location src_loc_;
  mutable bool converted_ = false;
};

absl::Status WriteTextProtoFile(
    const std::filesystem::path& filename,
    const ::google::protobuf::Message& proto,
    const std::source_location& src_loc = std::source_location::current());

}  // namespace mbo::proto

#endif  // MBO_PROTO_FILE_H_
