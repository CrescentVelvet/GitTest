// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: zss_train.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_zss_5ftrain_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_zss_5ftrain_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3010000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3010000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_zss_5ftrain_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_zss_5ftrain_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[3]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_zss_5ftrain_2eproto;
namespace ZSS {
namespace Protocol {
class MessiInput;
class MessiInputDefaultTypeInternal;
extern MessiInputDefaultTypeInternal _MessiInput_default_instance_;
class MessiOutput;
class MessiOutputDefaultTypeInternal;
extern MessiOutputDefaultTypeInternal _MessiOutput_default_instance_;
class TrainControl;
class TrainControlDefaultTypeInternal;
extern TrainControlDefaultTypeInternal _TrainControl_default_instance_;
}  // namespace Protocol
}  // namespace ZSS
PROTOBUF_NAMESPACE_OPEN
template<> ::ZSS::Protocol::MessiInput* Arena::CreateMaybeMessage<::ZSS::Protocol::MessiInput>(Arena*);
template<> ::ZSS::Protocol::MessiOutput* Arena::CreateMaybeMessage<::ZSS::Protocol::MessiOutput>(Arena*);
template<> ::ZSS::Protocol::TrainControl* Arena::CreateMaybeMessage<::ZSS::Protocol::TrainControl>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace ZSS {
namespace Protocol {

enum TrainControl_State : int {
  TrainControl_State_READY = 0,
  TrainControl_State_RUN = 1,
  TrainControl_State_CALC_REWARD = 2,
  TrainControl_State_RESET = 3
};
bool TrainControl_State_IsValid(int value);
constexpr TrainControl_State TrainControl_State_State_MIN = TrainControl_State_READY;
constexpr TrainControl_State TrainControl_State_State_MAX = TrainControl_State_RESET;
constexpr int TrainControl_State_State_ARRAYSIZE = TrainControl_State_State_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* TrainControl_State_descriptor();
template<typename T>
inline const std::string& TrainControl_State_Name(T enum_t_value) {
  static_assert(::std::is_same<T, TrainControl_State>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function TrainControl_State_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    TrainControl_State_descriptor(), enum_t_value);
}
inline bool TrainControl_State_Parse(
    const std::string& name, TrainControl_State* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<TrainControl_State>(
    TrainControl_State_descriptor(), name, value);
}
// ===================================================================

class TrainControl :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:ZSS.Protocol.TrainControl) */ {
 public:
  TrainControl();
  virtual ~TrainControl();

  TrainControl(const TrainControl& from);
  TrainControl(TrainControl&& from) noexcept
    : TrainControl() {
    *this = ::std::move(from);
  }

  inline TrainControl& operator=(const TrainControl& from) {
    CopyFrom(from);
    return *this;
  }
  inline TrainControl& operator=(TrainControl&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const TrainControl& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const TrainControl* internal_default_instance() {
    return reinterpret_cast<const TrainControl*>(
               &_TrainControl_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(TrainControl& a, TrainControl& b) {
    a.Swap(&b);
  }
  inline void Swap(TrainControl* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline TrainControl* New() const final {
    return CreateMaybeMessage<TrainControl>(nullptr);
  }

  TrainControl* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<TrainControl>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const TrainControl& from);
  void MergeFrom(const TrainControl& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(TrainControl* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "ZSS.Protocol.TrainControl";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_zss_5ftrain_2eproto);
    return ::descriptor_table_zss_5ftrain_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  typedef TrainControl_State State;
  static constexpr State READY =
    TrainControl_State_READY;
  static constexpr State RUN =
    TrainControl_State_RUN;
  static constexpr State CALC_REWARD =
    TrainControl_State_CALC_REWARD;
  static constexpr State RESET =
    TrainControl_State_RESET;
  static inline bool State_IsValid(int value) {
    return TrainControl_State_IsValid(value);
  }
  static constexpr State State_MIN =
    TrainControl_State_State_MIN;
  static constexpr State State_MAX =
    TrainControl_State_State_MAX;
  static constexpr int State_ARRAYSIZE =
    TrainControl_State_State_ARRAYSIZE;
  static inline const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor*
  State_descriptor() {
    return TrainControl_State_descriptor();
  }
  template<typename T>
  static inline const std::string& State_Name(T enum_t_value) {
    static_assert(::std::is_same<T, State>::value ||
      ::std::is_integral<T>::value,
      "Incorrect type passed to function State_Name.");
    return TrainControl_State_Name(enum_t_value);
  }
  static inline bool State_Parse(const std::string& name,
      State* value) {
    return TrainControl_State_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  enum : int {
    kStateFieldNumber = 1,
  };
  // optional .ZSS.Protocol.TrainControl.State state = 1;
  bool has_state() const;
  private:
  bool _internal_has_state() const;
  public:
  void clear_state();
  ::ZSS::Protocol::TrainControl_State state() const;
  void set_state(::ZSS::Protocol::TrainControl_State value);
  private:
  ::ZSS::Protocol::TrainControl_State _internal_state() const;
  void _internal_set_state(::ZSS::Protocol::TrainControl_State value);
  public:

  // @@protoc_insertion_point(class_scope:ZSS.Protocol.TrainControl)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  int state_;
  friend struct ::TableStruct_zss_5ftrain_2eproto;
};
// -------------------------------------------------------------------

class MessiInput :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:ZSS.Protocol.MessiInput) */ {
 public:
  MessiInput();
  virtual ~MessiInput();

  MessiInput(const MessiInput& from);
  MessiInput(MessiInput&& from) noexcept
    : MessiInput() {
    *this = ::std::move(from);
  }

  inline MessiInput& operator=(const MessiInput& from) {
    CopyFrom(from);
    return *this;
  }
  inline MessiInput& operator=(MessiInput&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const MessiInput& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const MessiInput* internal_default_instance() {
    return reinterpret_cast<const MessiInput*>(
               &_MessiInput_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(MessiInput& a, MessiInput& b) {
    a.Swap(&b);
  }
  inline void Swap(MessiInput* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline MessiInput* New() const final {
    return CreateMaybeMessage<MessiInput>(nullptr);
  }

  MessiInput* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<MessiInput>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const MessiInput& from);
  void MergeFrom(const MessiInput& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(MessiInput* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "ZSS.Protocol.MessiInput";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_zss_5ftrain_2eproto);
    return ::descriptor_table_zss_5ftrain_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kEnemyFieldNumber = 1,
    kDistFieldNumber = 2,
    kAngelFieldNumber = 3,
  };
  // optional int32 enemy = 1;
  bool has_enemy() const;
  private:
  bool _internal_has_enemy() const;
  public:
  void clear_enemy();
  ::PROTOBUF_NAMESPACE_ID::int32 enemy() const;
  void set_enemy(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_enemy() const;
  void _internal_set_enemy(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // optional float dist = 2;
  bool has_dist() const;
  private:
  bool _internal_has_dist() const;
  public:
  void clear_dist();
  float dist() const;
  void set_dist(float value);
  private:
  float _internal_dist() const;
  void _internal_set_dist(float value);
  public:

  // optional float angel = 3;
  bool has_angel() const;
  private:
  bool _internal_has_angel() const;
  public:
  void clear_angel();
  float angel() const;
  void set_angel(float value);
  private:
  float _internal_angel() const;
  void _internal_set_angel(float value);
  public:

  // @@protoc_insertion_point(class_scope:ZSS.Protocol.MessiInput)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  ::PROTOBUF_NAMESPACE_ID::int32 enemy_;
  float dist_;
  float angel_;
  friend struct ::TableStruct_zss_5ftrain_2eproto;
};
// -------------------------------------------------------------------

class MessiOutput :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:ZSS.Protocol.MessiOutput) */ {
 public:
  MessiOutput();
  virtual ~MessiOutput();

  MessiOutput(const MessiOutput& from);
  MessiOutput(MessiOutput&& from) noexcept
    : MessiOutput() {
    *this = ::std::move(from);
  }

  inline MessiOutput& operator=(const MessiOutput& from) {
    CopyFrom(from);
    return *this;
  }
  inline MessiOutput& operator=(MessiOutput&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const MessiOutput& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const MessiOutput* internal_default_instance() {
    return reinterpret_cast<const MessiOutput*>(
               &_MessiOutput_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  friend void swap(MessiOutput& a, MessiOutput& b) {
    a.Swap(&b);
  }
  inline void Swap(MessiOutput* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline MessiOutput* New() const final {
    return CreateMaybeMessage<MessiOutput>(nullptr);
  }

  MessiOutput* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<MessiOutput>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const MessiOutput& from);
  void MergeFrom(const MessiOutput& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(MessiOutput* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "ZSS.Protocol.MessiOutput";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_zss_5ftrain_2eproto);
    return ::descriptor_table_zss_5ftrain_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kVelXFieldNumber = 1,
    kVelYFieldNumber = 2,
    kVelWFieldNumber = 3,
  };
  // optional float vel_x = 1;
  bool has_vel_x() const;
  private:
  bool _internal_has_vel_x() const;
  public:
  void clear_vel_x();
  float vel_x() const;
  void set_vel_x(float value);
  private:
  float _internal_vel_x() const;
  void _internal_set_vel_x(float value);
  public:

  // optional float vel_y = 2;
  bool has_vel_y() const;
  private:
  bool _internal_has_vel_y() const;
  public:
  void clear_vel_y();
  float vel_y() const;
  void set_vel_y(float value);
  private:
  float _internal_vel_y() const;
  void _internal_set_vel_y(float value);
  public:

  // optional float vel_w = 3;
  bool has_vel_w() const;
  private:
  bool _internal_has_vel_w() const;
  public:
  void clear_vel_w();
  float vel_w() const;
  void set_vel_w(float value);
  private:
  float _internal_vel_w() const;
  void _internal_set_vel_w(float value);
  public:

  // @@protoc_insertion_point(class_scope:ZSS.Protocol.MessiOutput)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  float vel_x_;
  float vel_y_;
  float vel_w_;
  friend struct ::TableStruct_zss_5ftrain_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// TrainControl

// optional .ZSS.Protocol.TrainControl.State state = 1;
inline bool TrainControl::_internal_has_state() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool TrainControl::has_state() const {
  return _internal_has_state();
}
inline void TrainControl::clear_state() {
  state_ = 0;
  _has_bits_[0] &= ~0x00000001u;
}
inline ::ZSS::Protocol::TrainControl_State TrainControl::_internal_state() const {
  return static_cast< ::ZSS::Protocol::TrainControl_State >(state_);
}
inline ::ZSS::Protocol::TrainControl_State TrainControl::state() const {
  // @@protoc_insertion_point(field_get:ZSS.Protocol.TrainControl.state)
  return _internal_state();
}
inline void TrainControl::_internal_set_state(::ZSS::Protocol::TrainControl_State value) {
  assert(::ZSS::Protocol::TrainControl_State_IsValid(value));
  _has_bits_[0] |= 0x00000001u;
  state_ = value;
}
inline void TrainControl::set_state(::ZSS::Protocol::TrainControl_State value) {
  _internal_set_state(value);
  // @@protoc_insertion_point(field_set:ZSS.Protocol.TrainControl.state)
}

// -------------------------------------------------------------------

// MessiInput

// optional int32 enemy = 1;
inline bool MessiInput::_internal_has_enemy() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool MessiInput::has_enemy() const {
  return _internal_has_enemy();
}
inline void MessiInput::clear_enemy() {
  enemy_ = 0;
  _has_bits_[0] &= ~0x00000001u;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 MessiInput::_internal_enemy() const {
  return enemy_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 MessiInput::enemy() const {
  // @@protoc_insertion_point(field_get:ZSS.Protocol.MessiInput.enemy)
  return _internal_enemy();
}
inline void MessiInput::_internal_set_enemy(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _has_bits_[0] |= 0x00000001u;
  enemy_ = value;
}
inline void MessiInput::set_enemy(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_enemy(value);
  // @@protoc_insertion_point(field_set:ZSS.Protocol.MessiInput.enemy)
}

// optional float dist = 2;
inline bool MessiInput::_internal_has_dist() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool MessiInput::has_dist() const {
  return _internal_has_dist();
}
inline void MessiInput::clear_dist() {
  dist_ = 0;
  _has_bits_[0] &= ~0x00000002u;
}
inline float MessiInput::_internal_dist() const {
  return dist_;
}
inline float MessiInput::dist() const {
  // @@protoc_insertion_point(field_get:ZSS.Protocol.MessiInput.dist)
  return _internal_dist();
}
inline void MessiInput::_internal_set_dist(float value) {
  _has_bits_[0] |= 0x00000002u;
  dist_ = value;
}
inline void MessiInput::set_dist(float value) {
  _internal_set_dist(value);
  // @@protoc_insertion_point(field_set:ZSS.Protocol.MessiInput.dist)
}

// optional float angel = 3;
inline bool MessiInput::_internal_has_angel() const {
  bool value = (_has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool MessiInput::has_angel() const {
  return _internal_has_angel();
}
inline void MessiInput::clear_angel() {
  angel_ = 0;
  _has_bits_[0] &= ~0x00000004u;
}
inline float MessiInput::_internal_angel() const {
  return angel_;
}
inline float MessiInput::angel() const {
  // @@protoc_insertion_point(field_get:ZSS.Protocol.MessiInput.angel)
  return _internal_angel();
}
inline void MessiInput::_internal_set_angel(float value) {
  _has_bits_[0] |= 0x00000004u;
  angel_ = value;
}
inline void MessiInput::set_angel(float value) {
  _internal_set_angel(value);
  // @@protoc_insertion_point(field_set:ZSS.Protocol.MessiInput.angel)
}

// -------------------------------------------------------------------

// MessiOutput

// optional float vel_x = 1;
inline bool MessiOutput::_internal_has_vel_x() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool MessiOutput::has_vel_x() const {
  return _internal_has_vel_x();
}
inline void MessiOutput::clear_vel_x() {
  vel_x_ = 0;
  _has_bits_[0] &= ~0x00000001u;
}
inline float MessiOutput::_internal_vel_x() const {
  return vel_x_;
}
inline float MessiOutput::vel_x() const {
  // @@protoc_insertion_point(field_get:ZSS.Protocol.MessiOutput.vel_x)
  return _internal_vel_x();
}
inline void MessiOutput::_internal_set_vel_x(float value) {
  _has_bits_[0] |= 0x00000001u;
  vel_x_ = value;
}
inline void MessiOutput::set_vel_x(float value) {
  _internal_set_vel_x(value);
  // @@protoc_insertion_point(field_set:ZSS.Protocol.MessiOutput.vel_x)
}

// optional float vel_y = 2;
inline bool MessiOutput::_internal_has_vel_y() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool MessiOutput::has_vel_y() const {
  return _internal_has_vel_y();
}
inline void MessiOutput::clear_vel_y() {
  vel_y_ = 0;
  _has_bits_[0] &= ~0x00000002u;
}
inline float MessiOutput::_internal_vel_y() const {
  return vel_y_;
}
inline float MessiOutput::vel_y() const {
  // @@protoc_insertion_point(field_get:ZSS.Protocol.MessiOutput.vel_y)
  return _internal_vel_y();
}
inline void MessiOutput::_internal_set_vel_y(float value) {
  _has_bits_[0] |= 0x00000002u;
  vel_y_ = value;
}
inline void MessiOutput::set_vel_y(float value) {
  _internal_set_vel_y(value);
  // @@protoc_insertion_point(field_set:ZSS.Protocol.MessiOutput.vel_y)
}

// optional float vel_w = 3;
inline bool MessiOutput::_internal_has_vel_w() const {
  bool value = (_has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool MessiOutput::has_vel_w() const {
  return _internal_has_vel_w();
}
inline void MessiOutput::clear_vel_w() {
  vel_w_ = 0;
  _has_bits_[0] &= ~0x00000004u;
}
inline float MessiOutput::_internal_vel_w() const {
  return vel_w_;
}
inline float MessiOutput::vel_w() const {
  // @@protoc_insertion_point(field_get:ZSS.Protocol.MessiOutput.vel_w)
  return _internal_vel_w();
}
inline void MessiOutput::_internal_set_vel_w(float value) {
  _has_bits_[0] |= 0x00000004u;
  vel_w_ = value;
}
inline void MessiOutput::set_vel_w(float value) {
  _internal_set_vel_w(value);
  // @@protoc_insertion_point(field_set:ZSS.Protocol.MessiOutput.vel_w)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace Protocol
}  // namespace ZSS

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::ZSS::Protocol::TrainControl_State> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::ZSS::Protocol::TrainControl_State>() {
  return ::ZSS::Protocol::TrainControl_State_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_zss_5ftrain_2eproto
