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