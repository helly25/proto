# 0.6.0

* Added Bazel module support.
* Updated Clang to 19.1.6.
* Updated protobuf to v29.3.
* Updated other dependencies to latest.
* Added new modern `SilentErrorCollector`.

# 0.5

* Added support for com_google_protobuf >= Protocol Buffers v26.
* Moved `ParseTextProtoHelper` into namespace `mbo::proto::proto_internal`.
* Changed `ParseTextProtoHelper` to only accept derived proto messages on operator access.
* Changed `ParseTextProtoHelper` to die on parsing failure.
* Added `ParseText<T>() -> absl::StatusOr<T>`.
* Improved error message detail for all parsing functions.
* Updated all dependencies.
* Verified protobuf versions 25.2, 26.0, 26.1, 27.0, 27.1, 27.2, 28.0-rc1.
* Added Clang support.
* Updated GH actions.

# 0.4

* Changed `ParseTextProto`, `ParseTextProtoOrDie` to only work for actual derived proto types.
* Added `ParseProto` which returns `absl::StatusOr<MessageType>`.
* Updated external dependencies.

# 0.3

* Added missing dependency.

# 0.2

* Small fixes, mostly to support a few older abseil/re2 versions.

# 0.1

* Initial release
