// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: google/protobuf/util/internal/testdata/proto3.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2futil_2finternal_2ftestdata_2fproto3_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2futil_2finternal_2ftestdata_2fproto3_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3020000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3020003 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_google_2fprotobuf_2futil_2finternal_2ftestdata_2fproto3_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_google_2fprotobuf_2futil_2finternal_2ftestdata_2fproto3_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_google_2fprotobuf_2futil_2finternal_2ftestdata_2fproto3_2eproto;
namespace proto_util_converter {
namespace testing {
class Proto3Message;
struct Proto3MessageDefaultTypeInternal;
extern Proto3MessageDefaultTypeInternal _Proto3Message_default_instance_;
}  // namespace testing
}  // namespace proto_util_converter
PROTOBUF_NAMESPACE_OPEN
template<> ::proto_util_converter::testing::Proto3Message* Arena::CreateMaybeMessage<::proto_util_converter::testing::Proto3Message>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace proto_util_converter {
namespace testing {

enum Proto3Message_NestedEnum : int {
  Proto3Message_NestedEnum_FOO = 0,
  Proto3Message_NestedEnum_BAR = 1,
  Proto3Message_NestedEnum_BAZ = 2,
  Proto3Message_NestedEnum_Proto3Message_NestedEnum_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::min(),
  Proto3Message_NestedEnum_Proto3Message_NestedEnum_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::max()
};
bool Proto3Message_NestedEnum_IsValid(int value);
constexpr Proto3Message_NestedEnum Proto3Message_NestedEnum_NestedEnum_MIN = Proto3Message_NestedEnum_FOO;
constexpr Proto3Message_NestedEnum Proto3Message_NestedEnum_NestedEnum_MAX = Proto3Message_NestedEnum_BAZ;
constexpr int Proto3Message_NestedEnum_NestedEnum_ARRAYSIZE = Proto3Message_NestedEnum_NestedEnum_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* Proto3Message_NestedEnum_descriptor();
template<typename T>
inline const std::string& Proto3Message_NestedEnum_Name(T enum_t_value) {
  static_assert(::std::is_same<T, Proto3Message_NestedEnum>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function Proto3Message_NestedEnum_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    Proto3Message_NestedEnum_descriptor(), enum_t_value);
}
inline bool Proto3Message_NestedEnum_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, Proto3Message_NestedEnum* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<Proto3Message_NestedEnum>(
    Proto3Message_NestedEnum_descriptor(), name, value);
}
// ===================================================================

class Proto3Message final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:proto_util_converter.testing.Proto3Message) */ {
 public:
  inline Proto3Message() : Proto3Message(nullptr) {}
  ~Proto3Message() override;
  explicit PROTOBUF_CONSTEXPR Proto3Message(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Proto3Message(const Proto3Message& from);
  Proto3Message(Proto3Message&& from) noexcept
    : Proto3Message() {
    *this = ::std::move(from);
  }

  inline Proto3Message& operator=(const Proto3Message& from) {
    CopyFrom(from);
    return *this;
  }
  inline Proto3Message& operator=(Proto3Message&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const Proto3Message& default_instance() {
    return *internal_default_instance();
  }
  static inline const Proto3Message* internal_default_instance() {
    return reinterpret_cast<const Proto3Message*>(
               &_Proto3Message_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(Proto3Message& a, Proto3Message& b) {
    a.Swap(&b);
  }
  inline void Swap(Proto3Message* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(Proto3Message* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Proto3Message* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Proto3Message>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Proto3Message& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const Proto3Message& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to, const ::PROTOBUF_NAMESPACE_ID::Message& from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Proto3Message* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "proto_util_converter.testing.Proto3Message";
  }
  protected:
  explicit Proto3Message(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  typedef Proto3Message_NestedEnum NestedEnum;
  static constexpr NestedEnum FOO =
    Proto3Message_NestedEnum_FOO;
  static constexpr NestedEnum BAR =
    Proto3Message_NestedEnum_BAR;
  static constexpr NestedEnum BAZ =
    Proto3Message_NestedEnum_BAZ;
  static inline bool NestedEnum_IsValid(int value) {
    return Proto3Message_NestedEnum_IsValid(value);
  }
  static constexpr NestedEnum NestedEnum_MIN =
    Proto3Message_NestedEnum_NestedEnum_MIN;
  static constexpr NestedEnum NestedEnum_MAX =
    Proto3Message_NestedEnum_NestedEnum_MAX;
  static constexpr int NestedEnum_ARRAYSIZE =
    Proto3Message_NestedEnum_NestedEnum_ARRAYSIZE;
  static inline const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor*
  NestedEnum_descriptor() {
    return Proto3Message_NestedEnum_descriptor();
  }
  template<typename T>
  static inline const std::string& NestedEnum_Name(T enum_t_value) {
    static_assert(::std::is_same<T, NestedEnum>::value ||
      ::std::is_integral<T>::value,
      "Incorrect type passed to function NestedEnum_Name.");
    return Proto3Message_NestedEnum_Name(enum_t_value);
  }
  static inline bool NestedEnum_Parse(::PROTOBUF_NAMESPACE_ID::ConstStringParam name,
      NestedEnum* value) {
    return Proto3Message_NestedEnum_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  enum : int {
    kEnumValueFieldNumber = 1,
  };
  // .proto_util_converter.testing.Proto3Message.NestedEnum enum_value = 1;
  void clear_enum_value();
  ::proto_util_converter::testing::Proto3Message_NestedEnum enum_value() const;
  void set_enum_value(::proto_util_converter::testing::Proto3Message_NestedEnum value);
  private:
  ::proto_util_converter::testing::Proto3Message_NestedEnum _internal_enum_value() const;
  void _internal_set_enum_value(::proto_util_converter::testing::Proto3Message_NestedEnum value);
  public:

  // @@protoc_insertion_point(class_scope:proto_util_converter.testing.Proto3Message)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  int enum_value_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_google_2fprotobuf_2futil_2finternal_2ftestdata_2fproto3_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Proto3Message

// .proto_util_converter.testing.Proto3Message.NestedEnum enum_value = 1;
inline void Proto3Message::clear_enum_value() {
  enum_value_ = 0;
}
inline ::proto_util_converter::testing::Proto3Message_NestedEnum Proto3Message::_internal_enum_value() const {
  return static_cast< ::proto_util_converter::testing::Proto3Message_NestedEnum >(enum_value_);
}
inline ::proto_util_converter::testing::Proto3Message_NestedEnum Proto3Message::enum_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.Proto3Message.enum_value)
  return _internal_enum_value();
}
inline void Proto3Message::_internal_set_enum_value(::proto_util_converter::testing::Proto3Message_NestedEnum value) {
  
  enum_value_ = value;
}
inline void Proto3Message::set_enum_value(::proto_util_converter::testing::Proto3Message_NestedEnum value) {
  _internal_set_enum_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.Proto3Message.enum_value)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace testing
}  // namespace proto_util_converter

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::proto_util_converter::testing::Proto3Message_NestedEnum> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::proto_util_converter::testing::Proto3Message_NestedEnum>() {
  return ::proto_util_converter::testing::Proto3Message_NestedEnum_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2futil_2finternal_2ftestdata_2fproto3_2eproto