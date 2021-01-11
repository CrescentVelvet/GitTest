// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: log_labels.proto

#ifndef PROTOBUF_log_5flabels_2eproto__INCLUDED
#define PROTOBUF_log_5flabels_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3003000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3003000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
class BallPossessionLabel;
class BallPossessionLabelDefaultTypeInternal;
extern BallPossessionLabelDefaultTypeInternal _BallPossessionLabel_default_instance_;
class DribblingLabel;
class DribblingLabelDefaultTypeInternal;
extern DribblingLabelDefaultTypeInternal _DribblingLabel_default_instance_;
class GoalShotLabel;
class GoalShotLabelDefaultTypeInternal;
extern GoalShotLabelDefaultTypeInternal _GoalShotLabel_default_instance_;
class Labels;
class LabelsDefaultTypeInternal;
extern LabelsDefaultTypeInternal _Labels_default_instance_;
class PassingLabel;
class PassingLabelDefaultTypeInternal;
extern PassingLabelDefaultTypeInternal _PassingLabel_default_instance_;

namespace protobuf_log_5flabels_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static void InitDefaultsImpl();
  static void Shutdown();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_log_5flabels_2eproto

enum BallPossessionLabel_State {
  BallPossessionLabel_State_NONE = 0,
  BallPossessionLabel_State_YELLOW_POSSES = 1,
  BallPossessionLabel_State_BLUE_POSSES = 2,
  BallPossessionLabel_State_BallPossessionLabel_State_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  BallPossessionLabel_State_BallPossessionLabel_State_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool BallPossessionLabel_State_IsValid(int value);
const BallPossessionLabel_State BallPossessionLabel_State_State_MIN = BallPossessionLabel_State_NONE;
const BallPossessionLabel_State BallPossessionLabel_State_State_MAX = BallPossessionLabel_State_BLUE_POSSES;
const int BallPossessionLabel_State_State_ARRAYSIZE = BallPossessionLabel_State_State_MAX + 1;

const ::google::protobuf::EnumDescriptor* BallPossessionLabel_State_descriptor();
inline const ::std::string& BallPossessionLabel_State_Name(BallPossessionLabel_State value) {
  return ::google::protobuf::internal::NameOfEnum(
    BallPossessionLabel_State_descriptor(), value);
}
inline bool BallPossessionLabel_State_Parse(
    const ::std::string& name, BallPossessionLabel_State* value) {
  return ::google::protobuf::internal::ParseNamedEnum<BallPossessionLabel_State>(
    BallPossessionLabel_State_descriptor(), name, value);
}
enum TeamColor {
  TeamYELLOW = 0,
  TeamBLUE = 1,
  TeamColor_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  TeamColor_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool TeamColor_IsValid(int value);
const TeamColor TeamColor_MIN = TeamYELLOW;
const TeamColor TeamColor_MAX = TeamBLUE;
const int TeamColor_ARRAYSIZE = TeamColor_MAX + 1;

const ::google::protobuf::EnumDescriptor* TeamColor_descriptor();
inline const ::std::string& TeamColor_Name(TeamColor value) {
  return ::google::protobuf::internal::NameOfEnum(
    TeamColor_descriptor(), value);
}
inline bool TeamColor_Parse(
    const ::std::string& name, TeamColor* value) {
  return ::google::protobuf::internal::ParseNamedEnum<TeamColor>(
    TeamColor_descriptor(), name, value);
}
// ===================================================================

class DribblingLabel : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:DribblingLabel) */ {
 public:
  DribblingLabel();
  virtual ~DribblingLabel();

  DribblingLabel(const DribblingLabel& from);

  inline DribblingLabel& operator=(const DribblingLabel& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const DribblingLabel& default_instance();

  static inline const DribblingLabel* internal_default_instance() {
    return reinterpret_cast<const DribblingLabel*>(
               &_DribblingLabel_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(DribblingLabel* other);

  // implements Message ----------------------------------------------

  inline DribblingLabel* New() const PROTOBUF_FINAL { return New(NULL); }

  DribblingLabel* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const DribblingLabel& from);
  void MergeFrom(const DribblingLabel& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(DribblingLabel* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // bool is_dribbling = 1;
  void clear_is_dribbling();
  static const int kIsDribblingFieldNumber = 1;
  bool is_dribbling() const;
  void set_is_dribbling(bool value);

  // uint32 robot_id = 2;
  void clear_robot_id();
  static const int kRobotIdFieldNumber = 2;
  ::google::protobuf::uint32 robot_id() const;
  void set_robot_id(::google::protobuf::uint32 value);

  // .TeamColor team = 3;
  void clear_team();
  static const int kTeamFieldNumber = 3;
  ::TeamColor team() const;
  void set_team(::TeamColor value);

  // @@protoc_insertion_point(class_scope:DribblingLabel)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool is_dribbling_;
  ::google::protobuf::uint32 robot_id_;
  int team_;
  mutable int _cached_size_;
  friend struct protobuf_log_5flabels_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class BallPossessionLabel : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:BallPossessionLabel) */ {
 public:
  BallPossessionLabel();
  virtual ~BallPossessionLabel();

  BallPossessionLabel(const BallPossessionLabel& from);

  inline BallPossessionLabel& operator=(const BallPossessionLabel& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const BallPossessionLabel& default_instance();

  static inline const BallPossessionLabel* internal_default_instance() {
    return reinterpret_cast<const BallPossessionLabel*>(
               &_BallPossessionLabel_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(BallPossessionLabel* other);

  // implements Message ----------------------------------------------

  inline BallPossessionLabel* New() const PROTOBUF_FINAL { return New(NULL); }

  BallPossessionLabel* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const BallPossessionLabel& from);
  void MergeFrom(const BallPossessionLabel& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(BallPossessionLabel* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  typedef BallPossessionLabel_State State;
  static const State NONE =
    BallPossessionLabel_State_NONE;
  static const State YELLOW_POSSES =
    BallPossessionLabel_State_YELLOW_POSSES;
  static const State BLUE_POSSES =
    BallPossessionLabel_State_BLUE_POSSES;
  static inline bool State_IsValid(int value) {
    return BallPossessionLabel_State_IsValid(value);
  }
  static const State State_MIN =
    BallPossessionLabel_State_State_MIN;
  static const State State_MAX =
    BallPossessionLabel_State_State_MAX;
  static const int State_ARRAYSIZE =
    BallPossessionLabel_State_State_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  State_descriptor() {
    return BallPossessionLabel_State_descriptor();
  }
  static inline const ::std::string& State_Name(State value) {
    return BallPossessionLabel_State_Name(value);
  }
  static inline bool State_Parse(const ::std::string& name,
      State* value) {
    return BallPossessionLabel_State_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // .BallPossessionLabel.State state = 1;
  void clear_state();
  static const int kStateFieldNumber = 1;
  ::BallPossessionLabel_State state() const;
  void set_state(::BallPossessionLabel_State value);

  // uint32 robot_id = 2;
  void clear_robot_id();
  static const int kRobotIdFieldNumber = 2;
  ::google::protobuf::uint32 robot_id() const;
  void set_robot_id(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:BallPossessionLabel)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  int state_;
  ::google::protobuf::uint32 robot_id_;
  mutable int _cached_size_;
  friend struct protobuf_log_5flabels_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class PassingLabel : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:PassingLabel) */ {
 public:
  PassingLabel();
  virtual ~PassingLabel();

  PassingLabel(const PassingLabel& from);

  inline PassingLabel& operator=(const PassingLabel& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const PassingLabel& default_instance();

  static inline const PassingLabel* internal_default_instance() {
    return reinterpret_cast<const PassingLabel*>(
               &_PassingLabel_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    2;

  void Swap(PassingLabel* other);

  // implements Message ----------------------------------------------

  inline PassingLabel* New() const PROTOBUF_FINAL { return New(NULL); }

  PassingLabel* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const PassingLabel& from);
  void MergeFrom(const PassingLabel& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(PassingLabel* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // uint64 start_frame = 1;
  void clear_start_frame();
  static const int kStartFrameFieldNumber = 1;
  ::google::protobuf::uint64 start_frame() const;
  void set_start_frame(::google::protobuf::uint64 value);

  // uint64 end_frame = 2;
  void clear_end_frame();
  static const int kEndFrameFieldNumber = 2;
  ::google::protobuf::uint64 end_frame() const;
  void set_end_frame(::google::protobuf::uint64 value);

  // bool successful = 3;
  void clear_successful();
  static const int kSuccessfulFieldNumber = 3;
  bool successful() const;
  void set_successful(bool value);

  // uint32 passer_id = 4;
  void clear_passer_id();
  static const int kPasserIdFieldNumber = 4;
  ::google::protobuf::uint32 passer_id() const;
  void set_passer_id(::google::protobuf::uint32 value);

  // .TeamColor passer_team = 5;
  void clear_passer_team();
  static const int kPasserTeamFieldNumber = 5;
  ::TeamColor passer_team() const;
  void set_passer_team(::TeamColor value);

  // uint32 receiver_id = 6;
  void clear_receiver_id();
  static const int kReceiverIdFieldNumber = 6;
  ::google::protobuf::uint32 receiver_id() const;
  void set_receiver_id(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:PassingLabel)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint64 start_frame_;
  ::google::protobuf::uint64 end_frame_;
  bool successful_;
  ::google::protobuf::uint32 passer_id_;
  int passer_team_;
  ::google::protobuf::uint32 receiver_id_;
  mutable int _cached_size_;
  friend struct protobuf_log_5flabels_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class GoalShotLabel : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:GoalShotLabel) */ {
 public:
  GoalShotLabel();
  virtual ~GoalShotLabel();

  GoalShotLabel(const GoalShotLabel& from);

  inline GoalShotLabel& operator=(const GoalShotLabel& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const GoalShotLabel& default_instance();

  static inline const GoalShotLabel* internal_default_instance() {
    return reinterpret_cast<const GoalShotLabel*>(
               &_GoalShotLabel_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    3;

  void Swap(GoalShotLabel* other);

  // implements Message ----------------------------------------------

  inline GoalShotLabel* New() const PROTOBUF_FINAL { return New(NULL); }

  GoalShotLabel* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const GoalShotLabel& from);
  void MergeFrom(const GoalShotLabel& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(GoalShotLabel* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // uint64 start_frame = 1;
  void clear_start_frame();
  static const int kStartFrameFieldNumber = 1;
  ::google::protobuf::uint64 start_frame() const;
  void set_start_frame(::google::protobuf::uint64 value);

  // uint64 end_frame = 2;
  void clear_end_frame();
  static const int kEndFrameFieldNumber = 2;
  ::google::protobuf::uint64 end_frame() const;
  void set_end_frame(::google::protobuf::uint64 value);

  // bool successful = 3;
  void clear_successful();
  static const int kSuccessfulFieldNumber = 3;
  bool successful() const;
  void set_successful(bool value);

  // uint32 shooter_id = 4;
  void clear_shooter_id();
  static const int kShooterIdFieldNumber = 4;
  ::google::protobuf::uint32 shooter_id() const;
  void set_shooter_id(::google::protobuf::uint32 value);

  // .TeamColor shooter_team = 5;
  void clear_shooter_team();
  static const int kShooterTeamFieldNumber = 5;
  ::TeamColor shooter_team() const;
  void set_shooter_team(::TeamColor value);

  // @@protoc_insertion_point(class_scope:GoalShotLabel)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint64 start_frame_;
  ::google::protobuf::uint64 end_frame_;
  bool successful_;
  ::google::protobuf::uint32 shooter_id_;
  int shooter_team_;
  mutable int _cached_size_;
  friend struct protobuf_log_5flabels_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Labels : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Labels) */ {
 public:
  Labels();
  virtual ~Labels();

  Labels(const Labels& from);

  inline Labels& operator=(const Labels& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Labels& default_instance();

  static inline const Labels* internal_default_instance() {
    return reinterpret_cast<const Labels*>(
               &_Labels_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    4;

  void Swap(Labels* other);

  // implements Message ----------------------------------------------

  inline Labels* New() const PROTOBUF_FINAL { return New(NULL); }

  Labels* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Labels& from);
  void MergeFrom(const Labels& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(Labels* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .DribblingLabel dribbling_labels = 1;
  int dribbling_labels_size() const;
  void clear_dribbling_labels();
  static const int kDribblingLabelsFieldNumber = 1;
  const ::DribblingLabel& dribbling_labels(int index) const;
  ::DribblingLabel* mutable_dribbling_labels(int index);
  ::DribblingLabel* add_dribbling_labels();
  ::google::protobuf::RepeatedPtrField< ::DribblingLabel >*
      mutable_dribbling_labels();
  const ::google::protobuf::RepeatedPtrField< ::DribblingLabel >&
      dribbling_labels() const;

  // repeated .BallPossessionLabel ball_possession_labels = 2;
  int ball_possession_labels_size() const;
  void clear_ball_possession_labels();
  static const int kBallPossessionLabelsFieldNumber = 2;
  const ::BallPossessionLabel& ball_possession_labels(int index) const;
  ::BallPossessionLabel* mutable_ball_possession_labels(int index);
  ::BallPossessionLabel* add_ball_possession_labels();
  ::google::protobuf::RepeatedPtrField< ::BallPossessionLabel >*
      mutable_ball_possession_labels();
  const ::google::protobuf::RepeatedPtrField< ::BallPossessionLabel >&
      ball_possession_labels() const;

  // repeated .PassingLabel passing_labels = 3;
  int passing_labels_size() const;
  void clear_passing_labels();
  static const int kPassingLabelsFieldNumber = 3;
  const ::PassingLabel& passing_labels(int index) const;
  ::PassingLabel* mutable_passing_labels(int index);
  ::PassingLabel* add_passing_labels();
  ::google::protobuf::RepeatedPtrField< ::PassingLabel >*
      mutable_passing_labels();
  const ::google::protobuf::RepeatedPtrField< ::PassingLabel >&
      passing_labels() const;

  // repeated .GoalShotLabel goal_shot_labels = 4;
  int goal_shot_labels_size() const;
  void clear_goal_shot_labels();
  static const int kGoalShotLabelsFieldNumber = 4;
  const ::GoalShotLabel& goal_shot_labels(int index) const;
  ::GoalShotLabel* mutable_goal_shot_labels(int index);
  ::GoalShotLabel* add_goal_shot_labels();
  ::google::protobuf::RepeatedPtrField< ::GoalShotLabel >*
      mutable_goal_shot_labels();
  const ::google::protobuf::RepeatedPtrField< ::GoalShotLabel >&
      goal_shot_labels() const;

  // @@protoc_insertion_point(class_scope:Labels)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::DribblingLabel > dribbling_labels_;
  ::google::protobuf::RepeatedPtrField< ::BallPossessionLabel > ball_possession_labels_;
  ::google::protobuf::RepeatedPtrField< ::PassingLabel > passing_labels_;
  ::google::protobuf::RepeatedPtrField< ::GoalShotLabel > goal_shot_labels_;
  mutable int _cached_size_;
  friend struct protobuf_log_5flabels_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// DribblingLabel

// bool is_dribbling = 1;
inline void DribblingLabel::clear_is_dribbling() {
  is_dribbling_ = false;
}
inline bool DribblingLabel::is_dribbling() const {
  // @@protoc_insertion_point(field_get:DribblingLabel.is_dribbling)
  return is_dribbling_;
}
inline void DribblingLabel::set_is_dribbling(bool value) {
  
  is_dribbling_ = value;
  // @@protoc_insertion_point(field_set:DribblingLabel.is_dribbling)
}

// uint32 robot_id = 2;
inline void DribblingLabel::clear_robot_id() {
  robot_id_ = 0u;
}
inline ::google::protobuf::uint32 DribblingLabel::robot_id() const {
  // @@protoc_insertion_point(field_get:DribblingLabel.robot_id)
  return robot_id_;
}
inline void DribblingLabel::set_robot_id(::google::protobuf::uint32 value) {
  
  robot_id_ = value;
  // @@protoc_insertion_point(field_set:DribblingLabel.robot_id)
}

// .TeamColor team = 3;
inline void DribblingLabel::clear_team() {
  team_ = 0;
}
inline ::TeamColor DribblingLabel::team() const {
  // @@protoc_insertion_point(field_get:DribblingLabel.team)
  return static_cast< ::TeamColor >(team_);
}
inline void DribblingLabel::set_team(::TeamColor value) {
  
  team_ = value;
  // @@protoc_insertion_point(field_set:DribblingLabel.team)
}

// -------------------------------------------------------------------

// BallPossessionLabel

// .BallPossessionLabel.State state = 1;
inline void BallPossessionLabel::clear_state() {
  state_ = 0;
}
inline ::BallPossessionLabel_State BallPossessionLabel::state() const {
  // @@protoc_insertion_point(field_get:BallPossessionLabel.state)
  return static_cast< ::BallPossessionLabel_State >(state_);
}
inline void BallPossessionLabel::set_state(::BallPossessionLabel_State value) {
  
  state_ = value;
  // @@protoc_insertion_point(field_set:BallPossessionLabel.state)
}

// uint32 robot_id = 2;
inline void BallPossessionLabel::clear_robot_id() {
  robot_id_ = 0u;
}
inline ::google::protobuf::uint32 BallPossessionLabel::robot_id() const {
  // @@protoc_insertion_point(field_get:BallPossessionLabel.robot_id)
  return robot_id_;
}
inline void BallPossessionLabel::set_robot_id(::google::protobuf::uint32 value) {
  
  robot_id_ = value;
  // @@protoc_insertion_point(field_set:BallPossessionLabel.robot_id)
}

// -------------------------------------------------------------------

// PassingLabel

// uint64 start_frame = 1;
inline void PassingLabel::clear_start_frame() {
  start_frame_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 PassingLabel::start_frame() const {
  // @@protoc_insertion_point(field_get:PassingLabel.start_frame)
  return start_frame_;
}
inline void PassingLabel::set_start_frame(::google::protobuf::uint64 value) {
  
  start_frame_ = value;
  // @@protoc_insertion_point(field_set:PassingLabel.start_frame)
}

// uint64 end_frame = 2;
inline void PassingLabel::clear_end_frame() {
  end_frame_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 PassingLabel::end_frame() const {
  // @@protoc_insertion_point(field_get:PassingLabel.end_frame)
  return end_frame_;
}
inline void PassingLabel::set_end_frame(::google::protobuf::uint64 value) {
  
  end_frame_ = value;
  // @@protoc_insertion_point(field_set:PassingLabel.end_frame)
}

// bool successful = 3;
inline void PassingLabel::clear_successful() {
  successful_ = false;
}
inline bool PassingLabel::successful() const {
  // @@protoc_insertion_point(field_get:PassingLabel.successful)
  return successful_;
}
inline void PassingLabel::set_successful(bool value) {
  
  successful_ = value;
  // @@protoc_insertion_point(field_set:PassingLabel.successful)
}

// uint32 passer_id = 4;
inline void PassingLabel::clear_passer_id() {
  passer_id_ = 0u;
}
inline ::google::protobuf::uint32 PassingLabel::passer_id() const {
  // @@protoc_insertion_point(field_get:PassingLabel.passer_id)
  return passer_id_;
}
inline void PassingLabel::set_passer_id(::google::protobuf::uint32 value) {
  
  passer_id_ = value;
  // @@protoc_insertion_point(field_set:PassingLabel.passer_id)
}

// .TeamColor passer_team = 5;
inline void PassingLabel::clear_passer_team() {
  passer_team_ = 0;
}
inline ::TeamColor PassingLabel::passer_team() const {
  // @@protoc_insertion_point(field_get:PassingLabel.passer_team)
  return static_cast< ::TeamColor >(passer_team_);
}
inline void PassingLabel::set_passer_team(::TeamColor value) {
  
  passer_team_ = value;
  // @@protoc_insertion_point(field_set:PassingLabel.passer_team)
}

// uint32 receiver_id = 6;
inline void PassingLabel::clear_receiver_id() {
  receiver_id_ = 0u;
}
inline ::google::protobuf::uint32 PassingLabel::receiver_id() const {
  // @@protoc_insertion_point(field_get:PassingLabel.receiver_id)
  return receiver_id_;
}
inline void PassingLabel::set_receiver_id(::google::protobuf::uint32 value) {
  
  receiver_id_ = value;
  // @@protoc_insertion_point(field_set:PassingLabel.receiver_id)
}

// -------------------------------------------------------------------

// GoalShotLabel

// uint64 start_frame = 1;
inline void GoalShotLabel::clear_start_frame() {
  start_frame_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 GoalShotLabel::start_frame() const {
  // @@protoc_insertion_point(field_get:GoalShotLabel.start_frame)
  return start_frame_;
}
inline void GoalShotLabel::set_start_frame(::google::protobuf::uint64 value) {
  
  start_frame_ = value;
  // @@protoc_insertion_point(field_set:GoalShotLabel.start_frame)
}

// uint64 end_frame = 2;
inline void GoalShotLabel::clear_end_frame() {
  end_frame_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 GoalShotLabel::end_frame() const {
  // @@protoc_insertion_point(field_get:GoalShotLabel.end_frame)
  return end_frame_;
}
inline void GoalShotLabel::set_end_frame(::google::protobuf::uint64 value) {
  
  end_frame_ = value;
  // @@protoc_insertion_point(field_set:GoalShotLabel.end_frame)
}

// bool successful = 3;
inline void GoalShotLabel::clear_successful() {
  successful_ = false;
}
inline bool GoalShotLabel::successful() const {
  // @@protoc_insertion_point(field_get:GoalShotLabel.successful)
  return successful_;
}
inline void GoalShotLabel::set_successful(bool value) {
  
  successful_ = value;
  // @@protoc_insertion_point(field_set:GoalShotLabel.successful)
}

// uint32 shooter_id = 4;
inline void GoalShotLabel::clear_shooter_id() {
  shooter_id_ = 0u;
}
inline ::google::protobuf::uint32 GoalShotLabel::shooter_id() const {
  // @@protoc_insertion_point(field_get:GoalShotLabel.shooter_id)
  return shooter_id_;
}
inline void GoalShotLabel::set_shooter_id(::google::protobuf::uint32 value) {
  
  shooter_id_ = value;
  // @@protoc_insertion_point(field_set:GoalShotLabel.shooter_id)
}

// .TeamColor shooter_team = 5;
inline void GoalShotLabel::clear_shooter_team() {
  shooter_team_ = 0;
}
inline ::TeamColor GoalShotLabel::shooter_team() const {
  // @@protoc_insertion_point(field_get:GoalShotLabel.shooter_team)
  return static_cast< ::TeamColor >(shooter_team_);
}
inline void GoalShotLabel::set_shooter_team(::TeamColor value) {
  
  shooter_team_ = value;
  // @@protoc_insertion_point(field_set:GoalShotLabel.shooter_team)
}

// -------------------------------------------------------------------

// Labels

// repeated .DribblingLabel dribbling_labels = 1;
inline int Labels::dribbling_labels_size() const {
  return dribbling_labels_.size();
}
inline void Labels::clear_dribbling_labels() {
  dribbling_labels_.Clear();
}
inline const ::DribblingLabel& Labels::dribbling_labels(int index) const {
  // @@protoc_insertion_point(field_get:Labels.dribbling_labels)
  return dribbling_labels_.Get(index);
}
inline ::DribblingLabel* Labels::mutable_dribbling_labels(int index) {
  // @@protoc_insertion_point(field_mutable:Labels.dribbling_labels)
  return dribbling_labels_.Mutable(index);
}
inline ::DribblingLabel* Labels::add_dribbling_labels() {
  // @@protoc_insertion_point(field_add:Labels.dribbling_labels)
  return dribbling_labels_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::DribblingLabel >*
Labels::mutable_dribbling_labels() {
  // @@protoc_insertion_point(field_mutable_list:Labels.dribbling_labels)
  return &dribbling_labels_;
}
inline const ::google::protobuf::RepeatedPtrField< ::DribblingLabel >&
Labels::dribbling_labels() const {
  // @@protoc_insertion_point(field_list:Labels.dribbling_labels)
  return dribbling_labels_;
}

// repeated .BallPossessionLabel ball_possession_labels = 2;
inline int Labels::ball_possession_labels_size() const {
  return ball_possession_labels_.size();
}
inline void Labels::clear_ball_possession_labels() {
  ball_possession_labels_.Clear();
}
inline const ::BallPossessionLabel& Labels::ball_possession_labels(int index) const {
  // @@protoc_insertion_point(field_get:Labels.ball_possession_labels)
  return ball_possession_labels_.Get(index);
}
inline ::BallPossessionLabel* Labels::mutable_ball_possession_labels(int index) {
  // @@protoc_insertion_point(field_mutable:Labels.ball_possession_labels)
  return ball_possession_labels_.Mutable(index);
}
inline ::BallPossessionLabel* Labels::add_ball_possession_labels() {
  // @@protoc_insertion_point(field_add:Labels.ball_possession_labels)
  return ball_possession_labels_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::BallPossessionLabel >*
Labels::mutable_ball_possession_labels() {
  // @@protoc_insertion_point(field_mutable_list:Labels.ball_possession_labels)
  return &ball_possession_labels_;
}
inline const ::google::protobuf::RepeatedPtrField< ::BallPossessionLabel >&
Labels::ball_possession_labels() const {
  // @@protoc_insertion_point(field_list:Labels.ball_possession_labels)
  return ball_possession_labels_;
}

// repeated .PassingLabel passing_labels = 3;
inline int Labels::passing_labels_size() const {
  return passing_labels_.size();
}
inline void Labels::clear_passing_labels() {
  passing_labels_.Clear();
}
inline const ::PassingLabel& Labels::passing_labels(int index) const {
  // @@protoc_insertion_point(field_get:Labels.passing_labels)
  return passing_labels_.Get(index);
}
inline ::PassingLabel* Labels::mutable_passing_labels(int index) {
  // @@protoc_insertion_point(field_mutable:Labels.passing_labels)
  return passing_labels_.Mutable(index);
}
inline ::PassingLabel* Labels::add_passing_labels() {
  // @@protoc_insertion_point(field_add:Labels.passing_labels)
  return passing_labels_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::PassingLabel >*
Labels::mutable_passing_labels() {
  // @@protoc_insertion_point(field_mutable_list:Labels.passing_labels)
  return &passing_labels_;
}
inline const ::google::protobuf::RepeatedPtrField< ::PassingLabel >&
Labels::passing_labels() const {
  // @@protoc_insertion_point(field_list:Labels.passing_labels)
  return passing_labels_;
}

// repeated .GoalShotLabel goal_shot_labels = 4;
inline int Labels::goal_shot_labels_size() const {
  return goal_shot_labels_.size();
}
inline void Labels::clear_goal_shot_labels() {
  goal_shot_labels_.Clear();
}
inline const ::GoalShotLabel& Labels::goal_shot_labels(int index) const {
  // @@protoc_insertion_point(field_get:Labels.goal_shot_labels)
  return goal_shot_labels_.Get(index);
}
inline ::GoalShotLabel* Labels::mutable_goal_shot_labels(int index) {
  // @@protoc_insertion_point(field_mutable:Labels.goal_shot_labels)
  return goal_shot_labels_.Mutable(index);
}
inline ::GoalShotLabel* Labels::add_goal_shot_labels() {
  // @@protoc_insertion_point(field_add:Labels.goal_shot_labels)
  return goal_shot_labels_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::GoalShotLabel >*
Labels::mutable_goal_shot_labels() {
  // @@protoc_insertion_point(field_mutable_list:Labels.goal_shot_labels)
  return &goal_shot_labels_;
}
inline const ::google::protobuf::RepeatedPtrField< ::GoalShotLabel >&
Labels::goal_shot_labels() const {
  // @@protoc_insertion_point(field_list:Labels.goal_shot_labels)
  return goal_shot_labels_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::BallPossessionLabel_State> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::BallPossessionLabel_State>() {
  return ::BallPossessionLabel_State_descriptor();
}
template <> struct is_proto_enum< ::TeamColor> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::TeamColor>() {
  return ::TeamColor_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_log_5flabels_2eproto__INCLUDED