This package contains a collection of utilities around Google's [Protocolbuffer](https://github.com/protocolbuffers/protobuf). Some of the files were cloned from other repositories as described in section [Clone](#clone).

# Installation and requirements

This repository requires a C++20 compiler (in case of MacOS XCode 15 is needed).

This is done because the original sources use Abseil's `SourceLocation` has not been open sourced and instead of making
it available through this project, the project simply requires `std::source_location` which requires C++20.

The project only comes with a [Bazel](https://bazel.build/) BUILD.bazel file and can be added to other Bazel projects:

```
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "com_helly25_proto",
  url = "https://github.com/helly25/proto/archive/refs/heads/main.tar.gz",
)
```

The project is formatted with specific `clang-format` settings which require clang 16+ (in case of MacOs LLVM 16+ can be installed using brew).

# Parse Proto

* rule: `@com_helly25_proto//mbo/proto:parse_text_proto_cc`
* namespace: `mbo::proto`

* `ParseTextProtoOrDie`(`text_proto` [, `std::source_location`])
  * `text_proto` is a text proto best identified as a raw-string with marker 'pb'.
  * If `text_proto` cannot be parsed into the receiving proto type, then the function will fail.
  * Prefer this function over template function `ParseTextOrDie`.

* `ParseTextOrDie`<`Proto`>(`text_proto` [, `std::source_location`])
  * `text_proto` is a text proto best identified as a raw-string with marker 'pb'.
  * `Proto` is the type to produce.
  * If `text_proto` cannot be parsed as a `Proto`, then the function will fail.
  * Use this only if the `Proto` type cannot be inferred by `ParserTextProtoOrDie`.

## Usage

BUILD.bazel:

```
cc_test(
    name = "test",
    srcs = ["test.cc"],
    deps = ["@com_helly25_proto_matchers//mbo/proto:parse_text_proto",]
)
```

Source test.cc:

```.cc
#include "mbo/proto/parse_text_proto.h"

using ::mbo::proto::ParseTextProtoOrDie;

TEST(Foo, Test) {
    MyProto msg = ParseTextProtoOrDie(R"pb(field: "name")pb");
    // ...
}
```

# Proto Matchers

* rule: `@com_helly25_proto//mbo/proto:matchers_cc`
* namespace: `mbo::proto`

* `EqualsProto`(`msg`)
  * `msg`: protocolbuffer Message or string
  * Checks whether `msg` and the argument are the same proto.
  * If a string is used it is advisable to format the string as a raw-string
    with 'pb' marker as demonstrated above.

* `EqualsProto`()
  * 2-tuple polymorphic matcher that can be used for container comparisons.

* `EquivToProto`(`msg`)
  * `msg`: protocolbuffer Message or string
  * Similar to `EqualsProto` but checks whether `msg` and the argument are equivalent.
  * Equivalent means that if one side sets a field to the default value and the other side does not
    have that field specified, then they are equivalent.

* `EquivToProto`()
  * 2-tuple polymorphic matcher that can be used for container comparisons.

## Proto Matcher Wrappers

* `Approximately`(`matcher` [, `margin` [, `fraction`]])
  * `matcher` wrapper that allows to compare `double` and `float` values with a margin of error.
  * optional `margin` of error and a relative `fraction` of error which will make values match if
    satisfied.

* `TreatingNaNsAsEqual`(`matcher`)
  * `matcher` wrapper that compares floating-point fields such that NaNs are equal

* `IgnoringFields`(`ignore_fields`, `matcher`)
  * `matcher` wrapper that allows to ignore fields with different values.
  * `ignore_fields` list of fields to ignore. Fields are specified by their fully qualified names,
    i.e., the names corresponding to FieldDescriptor.full_name(). (e.g.
    testing.internal.FooProto2.member).

* `IgnoringFieldPaths`(`ignore_field_paths`, `matcher`)
  * `matcher` wrapper that allows to ignore fields with different values by their paths.
  * `ignore_field_paths` list of paths to ignore (e.g. 'field.sub_field.terminal_field').

* `IgnoringRepeatedFieldOrdering`(`matcher`)
  * `matcher` wrapper that allows to ignore the order in which repeated fields are presented.
  * E.g.: `IgnoringRepeatedFieldOrdering(EqualsProto(R"pb(x: 1 x: 2)pb"))`: While the provided
    proto has the repeated field `x` specified in the order `[1, 2]`, the matcher will also match
    if the argument proto has the order reversed.

* `Partially`(`matcher`)
  * `matcher` wrapper that compares only fields present in the expected protobuf. For example,
  * `Partially(EqualsProto(p))` will ignore any field that is not set in p when comparing the
    protobufs.

* `WhenDeserialized`(`matcher`)
  * `matcher` wrapper that matches a string that can be deserialized as a protobuf that matches
    `matcher`.

* `WhenDeserializedAs`<`Proto`>(`matcher`)
  * `matcher` wrapper that matches a string that can be deserialized as a protobuf of type `Proto`
     that matches `matcher`.

## Usage

BUILD.bazel:

```
cc_test(
    name = "test",
    srcs = ["test.cc"],
    deps = ["@com_helly25_proto_matchers//mbo/proto:matchers"],
)
```

Source test.cc:

```.cc
#include "mbo/proto/matchers.h"

using ::mbo::proto::EqualsProto;
using ::mbo::proto::IgnoringRepeatedFieldOrdering;

TEST(Foo, EqualsProto) {
    MyProto msg;
    msg.set_field("name");
    EXPECT_THAT(msg, EqualsProto(R"pb(field: "name")pb"));
}

TEST(Foo, Wrapper) {
    MyProto msg;
    msg.add_number(1);
    msg.add_number(3);
    msg.add_number(2);
    EXPECT_THAT(msg, IgnoringRepeatedFieldOrdering(EqualsProto(R"pb(
      number: 1
      number: 2
      number: 3
    )pb")));
}
```

# Clone

## Parse Proto

The clone was made from Google's [CPP-proto-builder](https://github.com/google/cpp-proto-builder).

The following files were cloned:

```
cp ../cpp-proto-builder/proto_builder/oss/parse_proto_text.* proto/mbo/proto
cp ../cpp-proto-builder/proto_builder/oss/parse_proto_text_test.cc proto/mbo/proto
cp ../cpp-proto-builder/proto_builder/oss/tests/simple_message.proto proto/mbo/proto
patch <proto/mbo/proto/parse_proto_text.diff
```

## Proto Matchers

The clone was made from Google's [CPP-proto-builder](https://github.com/google/cpp-proto-builder).
Alternatively this could have been done from:

* [FHIR](https://github.com/google/fhir),
* [nucleus](https://github.com/google/nucleus),
* [inazarenko's clone](https://github.com/inazarenko/protobuf-matchers).

The FHIR sources are stripped and the nucleus sources are older and finally inazarenko's clone was
modified to remove other Google dependencies which creates the issue that the GoogleTest docs do not
apply as for instance the regular expression library is different.

The following files were cloned:

```
cp ../cpp-proto-builder/proto_builder/oss/testing/proto_test_util.* proto/mbo/testing/proto
patch <proto/mbo/testing/proto_test_util.diff
```

The include guards were updated and the namespace changed to `testing::proto` which allows to
import the whole namespace easily. Further logging was switched directly to
[Abseil logging](https://abseil.io/docs/cpp/guides/logging) (this was not an option when I wrote
the proto Builder or when it was open sourced).

This clone was established 2023.07.15. The source has since been moved and modified but remains as
close to the original source as possible.
