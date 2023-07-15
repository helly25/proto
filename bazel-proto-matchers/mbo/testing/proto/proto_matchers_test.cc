#include "mbo/testing/proto/proto_matchers.h"

#include <iostream>
#include <string>
#include <type_traits>

#include "gmock/gmock.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "mbo/testing/proto/test.pb.h"

namespace mbo::testing::proto {
namespace {

using ::testing::Each;
using ::testing::EndsWith;
using ::testing::HasSubstr;
using ::testing::Matches;
using ::testing::Not;
using ::testing::SafeMatcherCast;

template <typename T, typename M>
inline std::string GetExplanation(M matcher, const T& value) {
  std::stringstream ss;
  SafeMatcherCast<const T&>(matcher).ExplainMatchResultTo(value, &ss);
  return ss.str();
}

TEST(Matchers, EqualsProto) {
  TestMessage m;
  m.set_num(42);
  m.set_name("name");

  EXPECT_THAT(m, EqualsProto(m));
  EXPECT_THAT(m, EqualsProto(R"pb(num: 42 name: "name")pb"));
  EXPECT_THAT(GetExplanation(EqualsProto(R"pb(num: 43 name: "name")pb"), m),
              EndsWith(R"pb(modified: num: 43 -> 42)pb"));
}

TEST(Matchers, EquivToProto) {
  TestMessage m1;
  m1.set_name("name");

  TestMessage m2;
  m2 = m1;
  m2.set_num(0);
  EXPECT_THAT(m1, EquivToProto(m1));
  EXPECT_THAT(m1, EquivToProto(m2));
  EXPECT_THAT(m1, Not(EqualsProto(m2)));
  EXPECT_THAT(m1, EquivToProto(R"pb(num: 0 name: "name")pb"));
  EXPECT_THAT(m1, EquivToProto(R"pb(name: "name")pb"));
  m2.set_num(2);
  EXPECT_THAT(m1, Not(EquivToProto(m2)));
}

TEST(Matchers, Approximately) {
  TestMessage m;
  m.set_val(1.0);

  EXPECT_THAT(m, Approximately(EqualsProto(R"pb(val: 0.992)pb"), 0.01));
  EXPECT_THAT(
      GetExplanation(Approximately(EqualsProto(R"pb(val: 0.9)pb"), 0.01), m),
      HasSubstr("modified: val: 0.9 -> 1"));
}

TEST(Matchers, TreatingNaNsAsEqual) {
  TestMessage m;
  m.set_val(0.0 / 0.0);

  EXPECT_THAT(m, TreatingNaNsAsEqual(EqualsProto(R"pb(val: nan)pb")));
  EXPECT_THAT(GetExplanation(EqualsProto(R"pb(val: nan)pb"), m),
              HasSubstr("val: nan -> nan"));
}

TEST(Matchers, IgnoringFields) {
  TestMessage m;
  m.set_name("name");
  m.set_num(25);

  EXPECT_THAT(m, IgnoringFields({"mbo.testing.proto.TestMessage.num"},
                                EqualsProto(R"pb(name: "name" num:42)pb")));
  EXPECT_THAT(m, IgnoringFields({"mbo.testing.proto.TestMessage.name"},
                                EqualsProto("num: 25")));
}

TEST(Matchers, IgnoringFieldPaths) {
  TestMessage m;
  m.set_name("name");
  m.set_num(42);

  EXPECT_THAT(m, IgnoringFieldPaths({"num"}, EqualsProto(R"pb(name: "name" num: 13)pb")));
  EXPECT_THAT(m, IgnoringFieldPaths({"name"}, EqualsProto(R"pb(num: 42)pb")));
}

TEST(Matchers, IgnoringFieldPathsNested) {
  TestMessage2 c;
  c.mutable_one()->set_num(13);
  c.mutable_one()->set_name("name");

  EXPECT_THAT(
      c, IgnoringFieldPaths({"one.name"},
                            EqualsProto(R"pb(one { num: 13 name: "different" })pb")));
}

TEST(Matchers, IgnoringFieldPathsRepeatedIndex) {
  TestMessage2 c;
  c.add_more()->set_num(10);
  c.add_more()->set_num(20);

  EXPECT_THAT(
      c, IgnoringFieldPaths({"more[0].num"},
                            EqualsProto(R"pb(more {num: 11} more {num: 20})pb")));
}

TEST(Matchers, IgnoringFieldPathsRepeatedNested) {
  TestMessage2 c;
  c.add_more()->set_num(10);
  c.add_more()->set_num(20);

  EXPECT_THAT(
      c, IgnoringFieldPaths({"more.num"},
                            EqualsProto(R"pb(more {num: 20} more {num: 10})pb")));
}

TEST(Matchers, IgnoringFieldPathsTerminalIndex) {
  TestMessage2 c;
  c.add_num(1);
  c.add_num(2);

  EXPECT_DEATH(Matches(IgnoringFieldPaths({"num[0]"}, EqualsProto(R"pb(num: 1 num: 2)pb")))(c),
               "Check failed: field_path.back\\(\\).index == -1 "
               "Terminally ignoring fields by index is currently not supported \\('num\\[0\\]'\\)");
}

TEST(Matchers, IgnoringRepeatedFieldOrdering) {
  TestMessage2 c;
  c.add_num(10);
  c.add_num(20);

  EXPECT_THAT(c, IgnoringRepeatedFieldOrdering(EqualsProto(R"pb(num: 20 num: 10)pb")));
}

TEST(Matchers, IgnoringRepeatedFieldOrderingNested) {
  TestMessage2 c;
  c.add_more()->set_num(10);
  c.add_more()->set_num(20);

  EXPECT_THAT(c, IgnoringRepeatedFieldOrdering(
                     EqualsProto(R"pb(more {num: 20} more {num: 10})pb")));
}

TEST(Matchers, Partially) {
  TestMessage m;
  m.set_name("foo");
  m.set_num(12);

  EXPECT_THAT(m, Partially(EqualsProto(R"pb(num: 12)pb")));
  EXPECT_THAT(m, Partially(EqualsProto(R"pb(name: "foo")pb")));
}

}  // namespace
}  // namespace mbo::testing::proto
