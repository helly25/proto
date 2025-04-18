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

#ifndef MBO_PROTO_MATCHERS_H_
#define MBO_PROTO_MATCHERS_H_

#include <initializer_list>
#include <limits>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "gmock/gmock.h"
#include "google/protobuf/message.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"

namespace mbo::proto {
namespace internal {

// Utilities.

// How to compare two fields (equal vs. equivalent).
typedef ::google::protobuf::util::MessageDifferencer::MessageFieldComparison ProtoFieldComparison;

// How to compare two floating-points (exact vs. approximate).
typedef ::google::protobuf::util::DefaultFieldComparator::FloatComparison ProtoFloatComparison;

// How to compare repeated fields (whether the order of elements matters).
typedef ::google::protobuf::util::MessageDifferencer::RepeatedFieldComparison RepeatedFieldComparison;

// Whether to compare all fields (full) or only fields present in the
// expected protobuf (partial).
typedef ::google::protobuf::util::MessageDifferencer::Scope ProtoComparisonScope;

const ProtoFieldComparison kProtoEqual = ::google::protobuf::util::MessageDifferencer::EQUAL;
const ProtoFieldComparison kProtoEquiv = google::protobuf::util::MessageDifferencer::EQUIVALENT;
const ProtoFloatComparison kProtoExact = ::google::protobuf::util::DefaultFieldComparator::EXACT;
const ProtoFloatComparison kProtoApproximate = ::google::protobuf::util::DefaultFieldComparator::APPROXIMATE;
const RepeatedFieldComparison kProtoCompareRepeatedFieldsRespectOrdering =
    ::google::protobuf::util::MessageDifferencer::AS_LIST;
const RepeatedFieldComparison kProtoCompareRepeatedFieldsIgnoringOrdering =
    ::google::protobuf::util::MessageDifferencer::AS_SET;
const ProtoComparisonScope kProtoFull = ::google::protobuf::util::MessageDifferencer::FULL;
const ProtoComparisonScope kProtoPartial = ::google::protobuf::util::MessageDifferencer::PARTIAL;

// Options for comparing two protobufs.
struct ProtoComparison {
  ProtoFieldComparison field_comp = kProtoEqual;
  ProtoFloatComparison float_comp = kProtoExact;
  bool treating_nan_as_equal = false;
  bool has_custom_margin = false;    // only used when float_comp = APPROXIMATE
  bool has_custom_fraction = false;  // only used when float_comp = APPROXIMATE
  RepeatedFieldComparison repeated_field_comp = kProtoCompareRepeatedFieldsRespectOrdering;
  ProtoComparisonScope scope = kProtoFull;
  double float_margin = 0.0;    // only used when has_custom_margin is set.
  double float_fraction = 0.0;  // only used when has_custom_fraction is set.
  std::vector<std::string> ignore_fields;
  std::vector<std::string> ignore_field_paths;
};

// Whether the protobuf must be initialized.
const bool kMustBeInitialized = true;
const bool kMayBeUninitialized = false;

// Parses the TextFormat representation of a protobuf, allowing required fields
// to be missing.  Returns true iff successful.
bool ParsePartialFromAscii(const std::string& pb_ascii, ::google::protobuf::Message* proto, std::string* error_text);

// Returns a protobuf of type Proto by parsing the given TextFormat
// representation of it.  Required fields can be missing, in which case the
// returned protobuf will not be fully initialized.
template<class Proto>
Proto MakePartialProtoFromAscii(const std::string& str) {
  Proto proto;
  std::string error_text;
  CHECK(ParsePartialFromAscii(str, &proto, &error_text))
      << "Failed to parse \"" << str << "\" as a " << proto.GetDescriptor()->full_name() << ":\n"
      << error_text;
  return proto;
}

// Returns true iff `lhs` and `rhs` can be compared (i.e. have the same descriptor).
bool ProtoComparable(const ::google::protobuf::Message& lhs, const ::google::protobuf::Message& rhs);

// Returns true iff actual and expected are comparable and match.  The
// comp argument specifies how the two are compared.
bool ProtoCompare(
    const ProtoComparison& comp,
    const ::google::protobuf::Message& actual,
    const ::google::protobuf::Message& expected);

// Overload for ProtoCompare where the expected message is specified as a text
// proto.  If the text cannot be parsed as a message of the same type as the
// actual message, a CHECK failure will cause the test to fail and no subsequent
// tests will be run.
template<typename Proto>
inline bool ProtoCompare(const ProtoComparison& comp, const Proto& actual, const std::string& expected) {
  return ProtoCompare(comp, actual, MakePartialProtoFromAscii<Proto>(expected));
}

// Describes the types of the expected and the actual protocol buffer.
std::string DescribeTypes(const ::google::protobuf::Message& expected, const ::google::protobuf::Message& actual);

// Prints the protocol buffer pointed to by proto.
std::string PrintProtoPointee(const ::google::protobuf::Message* proto);

// Describes the differences between the two protocol buffers.
std::string DescribeDiff(
    const ProtoComparison& comp,
    const ::google::protobuf::Message& actual,
    const ::google::protobuf::Message& expected);

// Common code for implementing EqualsProto.
class ProtoMatcherBase {
 public:
  ProtoMatcherBase(
      bool must_be_initialized,     // Must the argument be fully initialized?
      const ProtoComparison& comp)  // How to compare the two protobufs.
      : must_be_initialized_(must_be_initialized), comp_(new auto(comp)) {}

  ProtoMatcherBase(const ProtoMatcherBase& other)
      : must_be_initialized_(other.must_be_initialized_), comp_(new auto(*other.comp_)) {}

  ProtoMatcherBase& operator=(const ProtoMatcherBase& other) = delete;

  ProtoMatcherBase(ProtoMatcherBase&& other) = default;
  ProtoMatcherBase& operator=(ProtoMatcherBase&& other) = delete;

  virtual ~ProtoMatcherBase() = default;

  // Prints the expected protocol buffer.
  virtual void PrintExpectedTo(std::ostream* os) const = 0;

  // Returns the expected value as a protobuf object; if the object
  // cannot be created (e.g. in ProtoStringMatcher), explains why to
  // 'listener' and returns nullptr.  The caller must call
  // DeleteExpectedProto() on the returned value later.
  virtual const ::google::protobuf::Message* CreateExpectedProto(
      const ::google::protobuf::Message& arg,  // For determining the type of the
                                               // expected protobuf.
      ::testing::MatchResultListener* listener) const = 0;

  // Deletes the given expected protobuf, which must be obtained from
  // a call to CreateExpectedProto() earlier.
  virtual void DeleteExpectedProto(const ::google::protobuf::Message* expected) const = 0;

  // Makes this matcher compare floating-points approximately.
  void SetCompareApproximately() { comp_->float_comp = kProtoApproximate; }

  // Makes this matcher treating NaNs as equal when comparing floating-points.
  void SetCompareTreatingNaNsAsEqual() { comp_->treating_nan_as_equal = true; }

  // Makes this matcher ignore string elements specified by their fully
  // qualified names, i.e., names corresponding to FieldDescriptor.full_name().
  template<class Iterator>
  void AddCompareIgnoringFields(Iterator first, Iterator last) {
    comp_->ignore_fields.insert(comp_->ignore_fields.end(), first, last);
  }

  // Makes this matcher ignore string elements specified by their relative
  // FieldPath.
  template<class Iterator>
  void AddCompareIgnoringFieldPaths(Iterator first, Iterator last) {
    comp_->ignore_field_paths.insert(comp_->ignore_field_paths.end(), first, last);
  }

  // Makes this matcher compare repeated fields ignoring ordering of elements.
  void SetCompareRepeatedFieldsIgnoringOrdering() {
    comp_->repeated_field_comp = kProtoCompareRepeatedFieldsIgnoringOrdering;
  }

  // Sets the margin of error for approximate floating point comparison.
  void SetMargin(double margin) {
    ABSL_CHECK_GE(margin, 0.0) << "Using a negative margin for Approximately";
    comp_->has_custom_margin = true;
    comp_->float_margin = margin;
  }

  // Sets the relative fraction of error for approximate floating point
  // comparison.
  void SetFraction(double fraction) {
    ABSL_CHECK(0.0 <= fraction && fraction < 1.0) << "Fraction for Approximately must be >= 0.0 and < 1.0";
    comp_->has_custom_fraction = true;
    comp_->float_fraction = fraction;
  }

  // Makes this matcher compare protobufs partially.
  void SetComparePartially() { comp_->scope = kProtoPartial; }

  bool MatchAndExplain(const ::google::protobuf::Message& arg, ::testing::MatchResultListener* listener) const {
    return MatchAndExplain(arg, false, listener);
  }

  bool MatchAndExplain(const ::google::protobuf::Message* arg, ::testing::MatchResultListener* listener) const {
    return (arg != nullptr) && MatchAndExplain(*arg, true, listener);
  }

  // Describes the expected relation between the actual protobuf and
  // the expected one.
  void DescribeRelationToExpectedProto(std::ostream* os) const {
    if (comp_->repeated_field_comp == kProtoCompareRepeatedFieldsIgnoringOrdering) {
      *os << "(ignoring repeated field ordering) ";
    }
    if (!comp_->ignore_fields.empty()) {
      *os << "(ignoring fields: ";
      const char* sep = "";
      for (size_t i = 0; i < comp_->ignore_fields.size(); ++i, sep = ", ") {
        *os << sep << comp_->ignore_fields[i];
      }
      *os << ") ";
    }
    if (comp_->float_comp == kProtoApproximate) {
      *os << "approximately ";
      if (comp_->has_custom_margin || comp_->has_custom_fraction) {
        *os << "(";
        if (comp_->has_custom_margin) {
          std::stringstream sss;
          sss << std::setprecision(std::numeric_limits<double>::digits10 + 2) << comp_->float_margin;
          *os << "absolute error of float or double fields <= " << sss.str();
        }
        if (comp_->has_custom_margin && comp_->has_custom_fraction) {
          *os << " or ";
        }
        if (comp_->has_custom_fraction) {
          std::stringstream sss;
          sss << std::setprecision(std::numeric_limits<double>::digits10 + 2) << comp_->float_fraction;
          *os << "relative error of float or double fields <= " << sss.str();
        }
        *os << ") ";
      }
    }

    *os << (comp_->scope == kProtoPartial ? "partially " : "")
        << (comp_->field_comp == kProtoEqual ? "equal" : "equivalent")
        << (comp_->treating_nan_as_equal ? " (treating NaNs as equal)" : "") << " to ";
    PrintExpectedTo(os);
  }

  void DescribeTo(std::ostream* os) const {
    *os << "is " << (must_be_initialized_ ? "fully initialized and " : "");
    DescribeRelationToExpectedProto(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "is " << (must_be_initialized_ ? "not fully initialized or " : "") << "not ";
    DescribeRelationToExpectedProto(os);
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  bool must_be_initialized() const { return must_be_initialized_; }

  // NOLINTNEXTLINE(readability-identifier-naming)
  const ProtoComparison& comp() const { return *comp_; }

 private:
  bool MatchAndExplain(
      const ::google::protobuf::Message& arg,
      bool is_matcher_for_pointer,
      ::testing::MatchResultListener* listener) const;

  const bool must_be_initialized_;
  std::unique_ptr<ProtoComparison> comp_;
};

// Returns a copy of the given ::proto2 message.
inline ::google::protobuf::Message* CloneProto2(const ::google::protobuf::Message& src) {
  ::google::protobuf::Message* clone = src.New();
  clone->CopyFrom(src);
  return clone;
}

// Implements EqualsProto, where the matcher parameter is a protobuf.
class ProtoMatcher : public ProtoMatcherBase {
 public:
  ProtoMatcher(
      const ::google::protobuf::Message& expected,  // The expected protobuf.
      bool must_be_initialized,                     // Must the argument be fully initialized?
      const ProtoComparison& comp)                  // How to compare the two protobufs.
      : ProtoMatcherBase(must_be_initialized, comp), expected_(CloneProto2(expected)) {
    if (must_be_initialized) {
      ABSL_CHECK(expected.IsInitialized()) << "The protocol buffer given to *InitializedProto() "
                                           << "must itself be initialized, but the following required fields "
                                           << "are missing: " << expected.InitializationErrorString() << ".";
    }
  }

  void PrintExpectedTo(std::ostream* os) const override {
    *os << expected_->GetDescriptor()->full_name() << " ";
    ::testing::internal::UniversalPrint(*expected_, os);
  }

  const ::google::protobuf::Message* CreateExpectedProto(
      const ::google::protobuf::Message& /* arg */,
      ::testing::MatchResultListener* /* listener */) const override {
    return expected_.get();
  }

  void DeleteExpectedProto(const ::google::protobuf::Message* expected) const override {}

  // NOLINTNEXTLINE(readability-identifier-naming)
  const std::shared_ptr<const ::google::protobuf::Message>& expected() const { return expected_; }

 private:
  const std::shared_ptr<const ::google::protobuf::Message> expected_;
};

// Implements EqualsProto, where the matcher parameter is a string.
class ProtoStringMatcher : public ProtoMatcherBase {
 public:
  ProtoStringMatcher(
      std::string_view expected,    // The text representing the expected protobuf.
      bool must_be_initialized,     // Must the argument be fully initialized?
      const ProtoComparison& comp)  // How to compare the two protobufs.
      : ProtoMatcherBase(must_be_initialized, comp), expected_(expected) {}

  // Parses the expected string as a protobuf of the same type as arg,
  // and returns the parsed protobuf (or nullptr when the parse fails).
  // The caller must call DeleteExpectedProto() on the return value
  // later.
  const ::google::protobuf::Message* CreateExpectedProto(
      const ::google::protobuf::Message& arg,
      ::testing::MatchResultListener* listener) const override {
    ::google::protobuf::Message* expected_proto = arg.New();
    std::string error_text;
    if (ParsePartialFromAscii(expected_, expected_proto, &error_text)) {
      return expected_proto;
    } else {
      delete expected_proto;  // NOLINT(cppcoreguidelines-owning-memory)
      if (listener->IsInterested()) {
        *listener << "where ";
        PrintExpectedTo(listener->stream());
        *listener << " doesn't parse as a " << arg.GetDescriptor()->full_name() << ":\n" << error_text;
      }
      return nullptr;
    }
  }

  void DeleteExpectedProto(const ::google::protobuf::Message* expected) const override {
    delete expected;  // NOLINT(cppcoreguidelines-owning-memory)
  }

  void PrintExpectedTo(std::ostream* os) const override { *os << "<" << expected_ << ">"; }

 private:
  const std::string expected_;
};

using PolymorphicProtoMatcher = ::testing::PolymorphicMatcher<ProtoMatcher>;

// Common code for implementing WhenDeserialized(proto_matcher) and
// WhenDeserializedAs<PB>(proto_matcher).
template<class Proto>
class WhenDeserializedMatcherBase {
 public:
  using InnerMatcher = ::testing::Matcher<const Proto&>;

  WhenDeserializedMatcherBase() = delete;

  explicit WhenDeserializedMatcherBase(InnerMatcher&& proto_matcher) : proto_matcher_(std::move(proto_matcher)) {}

  WhenDeserializedMatcherBase(const WhenDeserializedMatcherBase&) = default;
  WhenDeserializedMatcherBase& operator=(const WhenDeserializedMatcherBase&) = delete;
  WhenDeserializedMatcherBase(WhenDeserializedMatcherBase&&) = default;
  WhenDeserializedMatcherBase& operator=(WhenDeserializedMatcherBase&&) = delete;

  virtual ~WhenDeserializedMatcherBase() = default;

  // Creates an empty protobuf with the expected type.
  virtual Proto* MakeEmptyProto() const = 0;

  // Type name of the expected protobuf.
  virtual std::string ExpectedTypeName() const = 0;

  // Name of the type argument given to WhenDeserializedAs<>(), or
  // "protobuf" for WhenDeserialized().
  virtual std::string TypeArgName() const = 0;

  // Deserializes the string as a protobuf of the same type as the expected
  // protobuf.
  Proto* Deserialize(::google::protobuf::io::ZeroCopyInputStream* input) const {
    Proto* proto = MakeEmptyProto();
    if (proto->ParsePartialFromZeroCopyStream(input)) {
      return proto;
    } else {
      delete proto;  // NOLINT(cppcoreguidelines-owning-memory)
      return nullptr;
    }
  }

  void DescribeTo(std::ostream* os) const {
    *os << "can be deserialized as a " << TypeArgName() << " that ";
    proto_matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const {
    *os << "cannot be deserialized as a " << TypeArgName() << " that ";
    proto_matcher_.DescribeTo(os);
  }

  bool MatchAndExplain(::google::protobuf::io::ZeroCopyInputStream* arg, ::testing::MatchResultListener* listener)
      const {
    // Deserializes the string arg as a protobuf of the same type as the
    // expected protobuf.
    std::unique_ptr<const Proto> deserialized_arg(Deserialize(arg));
    if (!listener->IsInterested()) {
      // No need to explain the match result.
      return (deserialized_arg != nullptr) && proto_matcher_.Matches(*deserialized_arg);
    }

    std::ostream* const os = listener->stream();
    if (deserialized_arg == nullptr) {
      *os << "which cannot be deserialized as a " << ExpectedTypeName();
      return false;
    }

    *os << "which deserializes to ";
    ::testing::internal::UniversalPrint(*deserialized_arg, os);

    ::testing::StringMatchResultListener inner_listener;
    const bool match = proto_matcher_.MatchAndExplain(*deserialized_arg, &inner_listener);
    const std::string explain = inner_listener.str();
    if (!explain.empty()) {
      *os << ",\n" << explain;
    }

    return match;
  }

  bool MatchAndExplain(const std::string& str, ::testing::MatchResultListener* listener) const {
    // NOLINTNEXTLINE(*-narrowing-conversions)
    ::google::protobuf::io::ArrayInputStream input(str.data(), str.size());
    return MatchAndExplain(&input, listener);
  }

  bool MatchAndExplain(std::string_view str, ::testing::MatchResultListener* listener) const {
    // NOLINTNEXTLINE(*-narrowing-conversions)
    ::google::protobuf::io::ArrayInputStream input(str.data(), str.size());
    return MatchAndExplain(&input, listener);
  }

  bool MatchAndExplain(const char* str, ::testing::MatchResultListener* listener) const {
    // NOLINTNEXTLINE(*-narrowing-conversions)
    ::google::protobuf::io::ArrayInputStream input(str, strlen(str));
    return MatchAndExplain(&input, listener);
  }

 private:
  const InnerMatcher proto_matcher_;
};

// Implements WhenDeserialized(proto_matcher).
class WhenDeserializedMatcher : public WhenDeserializedMatcherBase<::google::protobuf::Message> {
 public:
  explicit WhenDeserializedMatcher(const PolymorphicProtoMatcher& proto_matcher)
      : WhenDeserializedMatcherBase<::google::protobuf::Message>(proto_matcher),
        expected_proto_(proto_matcher.impl().expected()) {}

  ::google::protobuf::Message* MakeEmptyProto() const override { return expected_proto_->New(); }

  std::string ExpectedTypeName() const override { return std::string{expected_proto_->GetDescriptor()->full_name()}; }

  std::string TypeArgName() const override { return "protobuf"; }

 private:
  // The expected protobuf specified in the inner matcher
  // (proto_matcher_).  We only need a std::shared_ptr to it instead of
  // making a copy, as the expected protobuf will never be changed
  // once created.
  const std::shared_ptr<const ::google::protobuf::Message> expected_proto_;
};

// Implements WhenDeserializedAs<Proto>(proto_matcher).
template<class Proto>
class WhenDeserializedAsMatcher : public WhenDeserializedMatcherBase<Proto> {
 public:
  using InnerMatcher = ::testing::Matcher<const Proto&>;

  explicit WhenDeserializedAsMatcher(const InnerMatcher& inner_matcher)
      : WhenDeserializedMatcherBase<Proto>(inner_matcher) {}

  virtual Proto* MakeEmptyProto() const {
    return new Proto;  // NOLINT(cppcoreguidelines-owning-memory)
  }

  virtual std::string ExpectedTypeName() const { return Proto().GetDescriptor()->full_name(); }

  virtual std::string TypeArgName() const { return ExpectedTypeName(); }
};

// Implements EqualsProto for 2-tuple matchers.
class TupleProtoMatcher {
 public:
  explicit TupleProtoMatcher(const ProtoComparison& comp) : comp_(new auto(comp)) {}

  TupleProtoMatcher(const TupleProtoMatcher& other) : comp_(new auto(*other.comp_)) {}

  TupleProtoMatcher& operator=(const TupleProtoMatcher& other) = delete;

  TupleProtoMatcher(TupleProtoMatcher&& other) = default;
  TupleProtoMatcher& operator=(TupleProtoMatcher&& other) = default;

  ~TupleProtoMatcher() noexcept = default;

  template<typename T1, typename T2>
  explicit operator ::testing::Matcher<::testing::tuple<T1, T2>>() const {
    return MakeMatcher(new Impl<::testing::tuple<T1, T2>>(*comp_));
  }

  template<typename T1, typename T2>
  explicit operator ::testing::Matcher<const ::testing::tuple<T1, T2>&>() const {
    return MakeMatcher(new Impl<const ::testing::tuple<T1, T2>&>(*comp_));
  }

  // Allows matcher transformers, e.g., Approximately(), Partially(), etc. to
  // change the behavior of this 2-tuple matcher.
  TupleProtoMatcher& MutableImpl() { return *this; }

  // Makes this matcher compare floating-points approximately.
  void SetCompareApproximately() { comp_->float_comp = kProtoApproximate; }

  // Makes this matcher treating NaNs as equal when comparing floating-points.
  void SetCompareTreatingNaNsAsEqual() { comp_->treating_nan_as_equal = true; }

  // Makes this matcher ignore string elements specified by their fully
  // qualified names, i.e., names corresponding to FieldDescriptor.full_name().
  template<class Iterator>
  void AddCompareIgnoringFields(Iterator first, Iterator last) {
    comp_->ignore_fields.insert(comp_->ignore_fields.end(), first, last);
  }

  // Makes this matcher ignore string elements specified by their relative
  // FieldPath.
  template<class Iterator>
  void AddCompareIgnoringFieldPaths(Iterator first, Iterator last) {
    comp_->ignore_field_paths.insert(comp_->ignore_field_paths.end(), first, last);
  }

  // Makes this matcher compare repeated fields ignoring ordering of elements.
  void SetCompareRepeatedFieldsIgnoringOrdering() {
    comp_->repeated_field_comp = kProtoCompareRepeatedFieldsIgnoringOrdering;
  }

  // Sets the margin of error for approximate floating point comparison.
  void SetMargin(double margin) {
    ABSL_CHECK_GE(margin, 0.0) << "Using a negative margin for Approximately";
    comp_->has_custom_margin = true;
    comp_->float_margin = margin;
  }

  // Sets the relative fraction of error for approximate floating point
  // comparison.
  void SetFraction(double fraction) {
    ABSL_CHECK(0.0 <= fraction && fraction <= 1.0) << "Fraction for Relatively must be >= 0.0 and < 1.0";
    comp_->has_custom_fraction = true;
    comp_->float_fraction = fraction;
  }

  // Makes this matcher compares protobufs partially.
  void SetComparePartially() { comp_->scope = kProtoPartial; }

 private:
  template<typename Tuple>
  class Impl : public ::testing::MatcherInterface<Tuple> {
   public:
    explicit Impl(const ProtoComparison& comp) : comp_(comp) {}

    virtual bool MatchAndExplain(Tuple args, ::testing::MatchResultListener* /* listener */) const {
      using ::testing::get;
      return ProtoCompare(comp_, get<0>(args), get<1>(args));
    }

    virtual void DescribeTo(std::ostream* os) const {
      *os << (comp_.field_comp == kProtoEqual ? "are equal" : "are equivalent");
    }

    virtual void DescribeNegationTo(std::ostream* os) const {
      *os << (comp_.field_comp == kProtoEqual ? "are not equal" : "are not equivalent");
    }

   private:
    const ProtoComparison comp_;
  };

  std::unique_ptr<ProtoComparison> comp_;
};

}  // namespace internal

// Creates a polymorphic matcher that matches a 2-tuple where
// first.Equals(second) is true.
inline internal::TupleProtoMatcher EqualsProto() {
  internal::ProtoComparison comp;
  comp.field_comp = internal::kProtoEqual;
  return internal::TupleProtoMatcher(comp);
}

// Constructs a matcher that matches the argument if
// argument.Equals(x) or argument->Equals(x) returns true.
inline internal::PolymorphicProtoMatcher EqualsProto(const ::google::protobuf::Message& rhs) {
  internal::ProtoComparison comp;
  comp.field_comp = internal::kProtoEqual;
  return ::testing::MakePolymorphicMatcher(internal::ProtoMatcher(rhs, internal::kMayBeUninitialized, comp));
}

inline ::testing::PolymorphicMatcher<internal::ProtoStringMatcher> EqualsProto(const std::string& rhs) {
  internal::ProtoComparison comp;
  comp.field_comp = internal::kProtoEqual;
  return ::testing::MakePolymorphicMatcher(internal::ProtoStringMatcher(rhs, internal::kMayBeUninitialized, comp));
}

template<class Proto>
inline internal::PolymorphicProtoMatcher EqualsProto(const std::string& str) {
  return EqualsProto(internal::MakePartialProtoFromAscii<Proto>(str));
}

// Creates a polymorphic matcher that matches a 2-tuple where
// first.Equivalent(second) is true.
inline internal::TupleProtoMatcher EquivToProto() {
  internal::ProtoComparison comp;
  comp.field_comp = internal::kProtoEquiv;
  return internal::TupleProtoMatcher(comp);
}

// Constructs a matcher that matches the argument if
// argument.Equals(x) or argument->Equals(x) returns true.
inline internal::PolymorphicProtoMatcher EquivToProto(const ::google::protobuf::Message& rhs) {
  internal::ProtoComparison comp;
  comp.field_comp = internal::kProtoEquiv;
  return ::testing::MakePolymorphicMatcher(internal::ProtoMatcher(rhs, internal::kMayBeUninitialized, comp));
}

inline ::testing::PolymorphicMatcher<internal::ProtoStringMatcher> EquivToProto(const std::string& rhs) {
  internal::ProtoComparison comp;
  comp.field_comp = internal::kProtoEquiv;
  return ::testing::MakePolymorphicMatcher(internal::ProtoStringMatcher(rhs, internal::kMayBeUninitialized, comp));
}

template<class Proto>
inline internal::PolymorphicProtoMatcher EquivToProto(const std::string& str) {
  return EqualsProto(internal::MakePartialProtoFromAscii<Proto>(str));
}

// Approximately(m) returns a matcher that is the same as m, except
// that it compares floating-point fields approximately (using
// ::google::protobuf::util::MessageDifferencer's APPROXIMATE comparison
// option). The inner matcher m can be any of the Equals* and EquivTo* protobuf
// matchers above.
template<class InnerProtoMatcher>
inline InnerProtoMatcher Approximately(InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().SetCompareApproximately();
  return inner_proto_matcher;
}

// Alternative version of Approximately which takes an explicit margin of error.
template<class InnerProtoMatcher>
inline InnerProtoMatcher Approximately(InnerProtoMatcher inner_proto_matcher, double margin) {
  inner_proto_matcher.mutable_impl().SetCompareApproximately();
  inner_proto_matcher.mutable_impl().SetMargin(margin);
  return inner_proto_matcher;
}

// Alternative version of Approximately which takes an explicit margin of error
// and a relative fraction of error and will match if either is satisfied.
template<class InnerProtoMatcher>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline InnerProtoMatcher Approximately(InnerProtoMatcher inner_proto_matcher, double margin, double fraction) {
  inner_proto_matcher.mutable_impl().SetCompareApproximately();
  inner_proto_matcher.mutable_impl().SetMargin(margin);
  inner_proto_matcher.mutable_impl().SetFraction(fraction);
  return inner_proto_matcher;
}

// TreatingNaNsAsEqual(m) returns a matcher that is the same as m, except that
// it compares floating-point fields such that NaNs are equal.
// The inner matcher m can be any of the Equals* and EquivTo* protobuf matchers
// above.
template<class InnerProtoMatcher>
inline InnerProtoMatcher TreatingNaNsAsEqual(InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().SetCompareTreatingNaNsAsEqual();
  return inner_proto_matcher;
}

// IgnoringFields(fields, m) returns a matcher that is the same as m, except the
// specified fields will be ignored when matching
// (using ::google::protobuf::util::MessageDifferencer::IgnoreField). Each
// element in fields are specified by their fully qualified names, i.e., the
// names corresponding to FieldDescriptor.full_name(). (e.g.
// testing.internal.FooProto2.member). m can be any of the Equals* and EquivTo*
// protobuf matchers above. It can also be any of the transformer matchers
// listed here (e.g. Approximately, TreatingNaNsAsEqual) as long as the intent
// of the each concatenated matcher is mutually exclusive (e.g. using
// IgnoringFields in conjunction with Partially can have different results
// depending on whether the fields specified in IgnoringFields is part of the
// fields covered by Partially).
template<class InnerProtoMatcher, class Container>
inline InnerProtoMatcher IgnoringFields(const Container& ignore_fields, InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().AddCompareIgnoringFields(ignore_fields.begin(), ignore_fields.end());
  return inner_proto_matcher;
}

// See top comment.
template<class InnerProtoMatcher, class Container>
inline InnerProtoMatcher IgnoringFieldPaths(
    const Container& ignore_field_paths,
    InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().AddCompareIgnoringFieldPaths(ignore_field_paths.begin(), ignore_field_paths.end());
  return inner_proto_matcher;
}

template<class InnerProtoMatcher, class T>
inline InnerProtoMatcher IgnoringFields(std::initializer_list<T> fields, InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().AddCompareIgnoringFields(fields.begin(), fields.end());
  return inner_proto_matcher;
}

template<class InnerProtoMatcher, class T>
inline InnerProtoMatcher IgnoringFieldPaths(std::initializer_list<T> paths, InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().AddCompareIgnoringFieldPaths(paths.begin(), paths.end());
  return inner_proto_matcher;
}

// IgnoringRepeatedFieldOrdering(m) returns a matcher that is the same as m,
// except that it ignores the relative ordering of elements within each repeated
// field in m. See ::google::protobuf::MessageDifferencer::TreatAsSet() for more
// details.
template<class InnerProtoMatcher>
inline InnerProtoMatcher IgnoringRepeatedFieldOrdering(InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().SetCompareRepeatedFieldsIgnoringOrdering();
  return inner_proto_matcher;
}

// Partially(m) returns a matcher that is the same as m, except that
// only fields present in the expected protobuf are considered (using
// ::google::protobuf::util::MessageDifferencer's PARTIAL comparison option).
// For example, Partially(EqualsProto(p)) will ignore any field that's not set
// in p when comparing the protobufs. The inner matcher m can be any of the
// Equals* and EquivTo* protobuf matchers above.
template<class InnerProtoMatcher>
inline InnerProtoMatcher Partially(InnerProtoMatcher inner_proto_matcher) {
  inner_proto_matcher.mutable_impl().SetComparePartially();
  return inner_proto_matcher;
}

// WhenDeserialized(m) is a matcher that matches a string that can be
// deserialized as a protobuf that matches m.  m must be a protobuf
// matcher where the expected protobuf type is known at run time.
inline ::testing::PolymorphicMatcher<internal::WhenDeserializedMatcher> WhenDeserialized(
    const internal::PolymorphicProtoMatcher& proto_matcher) {
  return ::testing::MakePolymorphicMatcher(internal::WhenDeserializedMatcher(proto_matcher));
}

// WhenDeserializedAs<Proto>(m) is a matcher that matches a string
// that can be deserialized as a protobuf of type Proto that matches
// m, which can be any valid protobuf matcher.
template<class Proto, class InnerMatcher>
inline ::testing::PolymorphicMatcher<internal::WhenDeserializedAsMatcher<Proto>> WhenDeserializedAs(
    const InnerMatcher& inner_matcher) {
  return ::testing::MakePolymorphicMatcher(
      internal::WhenDeserializedAsMatcher<Proto>(::testing::SafeMatcherCast<const Proto&>(inner_matcher)));
}

}  // namespace mbo::proto

#endif  // MBO_PROTO_MATCHERS_H_
