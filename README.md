This is a clone of Google's [Protocolbuffer](https://github.com/protocolbuffers/protobuf) matchers
which are missing from [GoogleTest](https://github.com/google/googletest).

# Proto Matchers

* EqualsProto(msg)
  * msg: protocolbuffer Message or string
  * Checks whether `msg` and the argument are the same proto.
  * If a string is used it is advisable to format the string as a raw-string
    with 'pb' marker as demonstrated above.

* EqualsProto()
  * 2-tuple polymorphic matcher that can be used for container comparisons.

* EquivToProto(m)
  * msg: protocolbuffer Message or string
  * Similar to `EqualsProto` but checks whether `msg` and the argument are equivalent.
  * Equivalent means that if one side sets a field to the default value and the other side does not
    have that field specified, then they are equivalent.

* EquivToProto()
  * 2-tuple polymorphic matcher that can be used for container comparisons.

## Proto Matcher Wrappers

* Approximately(matcher [, margin [, fraction]])
  * `matcher` wrapper that allows to compare `double` and `float` values with a margin of error.
  * optional `margin` of error and a relative `fraction` of error which will make values match if
    satisfied.

* TreatingNaNsAsEqual(matcher)
  * `matcher` wrapper that compares floating-point fields such that NaNs are equal

* IgnoringFields(ignore_fields, matcher)
  * `matcher` wrapper that allows to ignore fields with different values.
  * `ignore_fields` list of fields to ignore. Fields are specified by their fully qualified names,
    i.e., the names corresponding to FieldDescriptor.full_name(). (e.g.
    testing.internal.FooProto2.member).

* IgnoringFieldPaths(ignore_field_paths, matcher)
  * `matcher` wrapper that allows to ignore fields with different values by their paths.
  * `ignore_field_paths` list of paths to ignore (e.g. 'field.sub_field.terminal_field').

* IgnoringRepeatedFieldOrdering(matcher)
  * `matcher` wrapper that allows to ignore the order in which repeated fields are presented.
  * E.g.: `IgnoringRepeatedFieldOrdering(EqualsProto(R"pb(x: 1 x: 2)pb"))`: While the provided
    proto has the repeated field `x` specified in the order `[1, 2]`, the matcher will also match
    if the argument proto has the order reversed.

* Partially(matcher)
  * `matcher` wrapper that compares only fields present in the expected protobuf. For example,
  * `Partially(EqualsProto(p))` will ignore any field that is not set in p when comparing the
    protobufs.

* WhenDeserialized(matcher)
  * `matcher` wrapper that matches a string that can be deserialized as a protobuf that matches
    `matcher`.

* WhenDeserializedAs<Proto>(matcher)
  * `matcher` wrapper that matches a string that can be deserialized as a protobuf of type `Proto`
     that matches `matcher`.

## Usage

BUILD.bazel:

```
cc_test(
    name = "test",
    srcs = ["test.cc"],
    deps = ["@com_helly25_proto_matchers//mbo/testing/proto:proto_matchers",]
)
```

Source test.cc:

```.cc
#include "mbo/testing/proto/proto_matchers.h"

using mbo::testing::proto::EqualsProto;

TEST(Foo, Test) {
    MyProto msg;
    msg.set_field("name");
    EXPECT_THAT(msg, EqualsProto(R"pb(field: "name")pb"));
}
```

# Clone

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

This clone was last updated 2023.07.15.
