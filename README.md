This package contains a collection of utilities around Google's [Protocolbuffer](https://github.com/protocolbuffers/protobuf). The functions offered in this packages are widely used across Google's C++ code base and have saved tens of thousands of engineering hours. Some of these functons were originally implemented by the author and later re-implemented or cloned (see below).

The project works with Google's proto library version 27, 28, 29 and 30. Packages are available at [Bazel Central Registry](https://registry.bazel.build/modules/helly25_proto) and [Github](https://github.com/helly25/proto/releases).

[![Test](https://github.com/helly25/proto/actions/workflows/main.yml/badge.svg)](https://github.com/helly25/proto/actions/workflows/main.yml)

# Parse Proto

* rule: `@com_helly25_proto//mbo/proto:parse_text_proto_cc`
* namespace: `mbo::proto`

* `ParseTextProtoOrDie`(`text_proto` [, `std::source_location`])
  * `text_proto` is a text proto best identified as a raw-string with marker `pb` or `proto`.
  * If `text_proto` cannot be parsed into the receiving proto type, then the function will fail.
  * Prefer this function over template function `ParseTextOrDie`.

* `ParseTextOrDie`<`Proto`>(`text_proto` [, `std::source_location`])
  * `text_proto` is a text proto best identified as a raw-string with marker `pb` or `proto`.
  * `Proto` is the type to produce.
  * If `text_proto` cannot be parsed as a `Proto`, then the function will fail.
  * Use this only if the `Proto` type cannot be inferred by `ParserTextProtoOrDie`.

* `ParseTest`<`Proto`>(`text_proto` [, `std::source_location`]) -> `absl::StatusOr`<`Proto`>
  * `text_proto` is a text proto best identified as a raw-string with marker `pb` or `proto`.
  * `Proto` is the type to produce.
  * If `text_proto` cannot be parse as a `Proto`, then the function returns a non-`absl::OkStatus`.
  * Use this function in cases where errors are expected.

## Usage

BUILD.bazel:

```
cc_test(
    name = "test",
    srcs = ["test.cc"],
    deps = ["@com_helly25_proto//mbo/proto:parse_text_proto",]
)
```

Source test.cc:

```.cc
#include "mbo/proto/parse_text_proto.h"

using ::mbo::proto::ParseTextProtoOrDie;

TEST(Foo, Test) {
    MyProto msg = ParseTextProtoOrDie(R"pb(
      field: "name"
      what: "Just dump the plain text-proto as C++ raw-string."
      )pb");
    // ...
}
```

Note:
* In the above example the proto is not manually constructed field by field.
* Instead the text-proto output is directly used as a C++ raw-string.
* Further the C++ raw-string is enclosed in `pb` markers which some tidy tools identify and use to correctly format the text-proto.
* One of the biggest advantages of these parse function is that their result can be assigned into a const variable.

The `ParseTextProtoOrDie` function dies if the input text-proto is not valid. That is done because the function emulates type safety this way. That is the author will likely only have to fix this once while many people will read the code. Further, this is test input that is supposed to be correct as is. If the input is of dynamic nature, then `ParseText<ProtoType>(std::string_view)` has to be used.

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
    deps = ["@com_helly25_proto//mbo/proto:matchers"],
)
```

Source test.cc:

```.cc
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mbo/proto/matchers.h"

using ::mbo::proto::EqualsProto;
using ::mbo::proto::IgnoringRepeatedFieldOrdering;

TEST(Foo, EqualsProto) {
    MyProto msg;
    msg.set_field("name");
    EXPECT_THAT(msg, EqualsProto(R"pb(
      field: "name"
      )pb"));
}
```

In the above example `EqualsProto` takes the text-proto as a C++ raw-string.

The matchers can of course be combined with the parse functions. The below shows how a `FunctionUnderTest` can be tested. It receives the proto input directly from the parse function and the matcher compares it directly to the expected golden result text-proto. Note how there is no field-by-field processing anywhere. No dstraction from what is being tested and what the expectations are. Or in other words the test avoids misleading and error prone in-test logic. And becasue the function-under-test is called inside the EXPECT_THAT macro the gtest failure messages will show what actually failed (and not something like "Input: temp_var").

```.cc
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mbo/proto/matchers.h"
#include "mbo/proto/parse_text_proto.h"

using ::mbo::proto::EqualsProto;
using ::mbo::proto::IgnoringRepeatedFieldOrdering;
using ::mbo::proto::ParseTextProtoOrDie;

MyProto FunctionUnderTest(const MyProto& proto) {
  return proto;
}

TEST(Foo, Wrapper) {
    const MyProto input = ParseTextProtoOrDie(R"pb(
      number: 1
      number: 2
      number: 3
    )pb");
    EXPECT_THAT(
      FunctionUnderTest(input),
      IgnoringRepeatedFieldOrdering(EqualsProto(R"pb(
        number: 1
        number: 2
        number: 3
      )pb")));
}
```

# Installation and requirements

This repository requires a C++20 compiler (in case of MacOS XCode 15 is needed). The project's CI tests a combination of Clang and GCC compilers on Linux/Ubuntu and MacOS. The project can be used with Google's proto libraries in versions [27, 28, 29, 30].

The reliance on a C++20 compiler is because it uses `std::source_location` since Google's Abseil `absl::SourceLocation` has not been open sourced.

The project only comes with a Bazel BUILD.bazel file and can be added to other Bazel projects.

The project is formatted with specific clang-format settings which require clang 16+ (in case of MacOs LLVM 16+ can be installed using brew). For simplicity in dev mode the project pulls the appropriate clang tools and can be compiled with those tools using `bazel [build|test] --config=clang ...`.

## WORKSPACE

Checkout [Releases](https://github.com/helly25/proto/releases) or use head ref as follows:

```
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
  name = "com_helly25_proto",
  url = "https://github.com/helly25/proto/releases/download/0.0.0/proto-0.0.0.tar.gz",
  sha256 = "", # See https://github.com/helly25/proto/releases for releases versions and SHA codes.
)
```

## MODULES.bazel

The [BCR](https://registry.bazel.build/modules/helly25_proto) version has its dependencies pushed down to the lowest supported versions but those can be bumped locally. For each supported proto version there is a separate `MODULE.proto<version>.bazel` file that contains the minimum requirements of the necessary support repos.

Check [Releases](https://registry.bazel.build/modules/helly25_proto) for details. All that is needed is a `bazel_dep` instruction with the correct version.

```
bazel_dep(name = "helly25_proto", version = "0.0.0", repo_name = "com_helly25_proto")
```

# Clone

The clone was made from Google's [CPP-proto-builder](https://github.com/google/cpp-proto-builder), of which the project lead is the original author and lead for over a decade. That includes in particular the parse_proto components which were invented in their original form around 2012 and used widely throughout Google.

## Parse Proto

The following files were cloned:

```
cp ../cpp-proto-builder/proto_builder/oss/parse_proto_text.* proto/mbo/proto
cp ../cpp-proto-builder/proto_builder/oss/parse_proto_text_test.cc proto/mbo/proto
cp ../cpp-proto-builder/proto_builder/oss/tests/simple_message.proto proto/mbo/proto
patch <proto/mbo/proto/parse_proto_text.diff
```

The diff files are available in the repository history.

## Proto Matchers

The matchers are part of Google's [CPP-proto-builder](https://github.com/google/cpp-proto-builder), (see above).
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

The diff files are available in the repository history.

The include guards were updated and the namespace changed to `testing::proto` which allows to
import the whole namespace easily. Further logging was switched directly to
[Abseil logging](https://abseil.io/docs/cpp/guides/logging) (this was not an option when I wrote
the proto Builder or when it was open sourced).

This clone was established 2023.07.15. The source has since been moved and modified but remains as
close to the original source as possible.
