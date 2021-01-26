// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: grSim_Replacement.proto

#ifndef PROTOBUF_grSim_5fReplacement_2eproto__INCLUDED
#define PROTOBUF_grSim_5fReplacement_2eproto__INCLUDED

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
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
class grSim_BallReplacement;
class grSim_BallReplacementDefaultTypeInternal;
extern grSim_BallReplacementDefaultTypeInternal _grSim_BallReplacement_default_instance_;
class grSim_Replacement;
class grSim_ReplacementDefaultTypeInternal;
extern grSim_ReplacementDefaultTypeInternal _grSim_Replacement_default_instance_;
class grSim_RobotReplacement;
class grSim_RobotReplacementDefaultTypeInternal;
extern grSim_RobotReplacementDefaultTypeInternal _grSim_RobotReplacement_default_instance_;

namespace protobuf_grSim_5fReplacement_2eproto {
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
}  // namespace protobuf_grSim_5fReplacement_2eproto

// ===================================================================

class grSim_RobotReplacement : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:grSim_RobotReplacement) */ {
 public:
  grSim_RobotReplacement();
  virtual ~grSim_RobotReplacement();

  grSim_RobotReplacement(const grSim_RobotReplacement& from);

  inline grSim_RobotReplacement& operator=(const grSim_RobotReplacement& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const grSim_RobotReplacement& default_instance();

  static inline const grSim_RobotReplacement* internal_default_instance() {
    return reinterpret_cast<const grSim_RobotReplacement*>(
               &_grSim_RobotReplacement_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(grSim_RobotReplacement* other);

  // implements Message ----------------------------------------------

  inline grSim_RobotReplacement* New() const PROTOBUF_FINAL { return New(NULL); }

  grSim_RobotReplacement* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const grSim_RobotReplacement& from);
  void MergeFrom(const grSim_RobotReplacement& from);
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
  void InternalSwap(grSim_RobotReplacement* other);
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

  // double x = 1;
  void clear_x();
  static const int kXFieldNumber = 1;
  double x() const;
  void set_x(double value);

  // double y = 2;
  void clear_y();
  static const int kYFieldNumber = 2;
  double y() const;
  void set_y(double value);

  // double dir = 3;
  void clear_dir();
  static const int kDirFieldNumber = 3;
  double dir() const;
  void set_dir(double value);

  // uint32 id = 4;
  void clear_id();
  static const int kIdFieldNumber = 4;
  ::google::protobuf::uint32 id() const;
  void set_id(::google::protobuf::uint32 value);

  // bool yellowteam = 5;
  void clear_yellowteam();
  static const int kYellowteamFieldNumber = 5;
  bool yellowteam() const;
  void set_yellowteam(bool value);

  // bool turnon = 6;
  void clear_turnon();
  static const int kTurnonFieldNumber = 6;
  bool turnon() const;
  void set_turnon(bool value);

  // @@protoc_insertion_point(class_scope:grSim_RobotReplacement)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  double x_;
  double y_;
  double dir_;
  ::google::protobuf::uint32 id_;
  bool yellowteam_;
  bool turnon_;
  mutable int _cached_size_;
  friend struct protobuf_grSim_5fReplacement_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class grSim_BallReplacement : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:grSim_BallReplacement) */ {
 public:
  grSim_BallReplacement();
  virtual ~grSim_BallReplacement();

  grSim_BallReplacement(const grSim_BallReplacement& from);

  inline grSim_BallReplacement& operator=(const grSim_BallReplacement& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const grSim_BallReplacement& default_instance();

  static inline const grSim_BallReplacement* internal_default_instance() {
    return reinterpret_cast<const grSim_BallReplacement*>(
               &_grSim_BallReplacement_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(grSim_BallReplacement* other);

  // implements Message ----------------------------------------------

  inline grSim_BallReplacement* New() const PROTOBUF_FINAL { return New(NULL); }

  grSim_BallReplacement* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const grSim_BallReplacement& from);
  void MergeFrom(const grSim_BallReplacement& from);
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
  void InternalSwap(grSim_BallReplacement* other);
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

  // double x = 1;
  void clear_x();
  static const int kXFieldNumber = 1;
  double x() const;
  void set_x(double value);

  // double y = 2;
  void clear_y();
  static const int kYFieldNumber = 2;
  double y() const;
  void set_y(double value);

  // double vx = 3;
  void clear_vx();
  static const int kVxFieldNumber = 3;
  double vx() const;
  void set_vx(double value);

  // double vy = 4;
  void clear_vy();
  static const int kVyFieldNumber = 4;
  double vy() const;
  void set_vy(double value);

  // @@protoc_insertion_point(class_scope:grSim_BallReplacement)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  double x_;
  double y_;
  double vx_;
  double vy_;
  mutable int _cached_size_;
  friend struct protobuf_grSim_5fReplacement_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class grSim_Replacement : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:grSim_Replacement) */ {
 public:
  grSim_Replacement();
  virtual ~grSim_Replacement();

  grSim_Replacement(const grSim_Replacement& from);

  inline grSim_Replacement& operator=(const grSim_Replacement& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const grSim_Replacement& default_instance();

  static inline const grSim_Replacement* internal_default_instance() {
    return reinterpret_cast<const grSim_Replacement*>(
               &_grSim_Replacement_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    2;

  void Swap(grSim_Replacement* other);

  // implements Message ----------------------------------------------

  inline grSim_Replacement* New() const PROTOBUF_FINAL { return New(NULL); }

  grSim_Replacement* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const grSim_Replacement& from);
  void MergeFrom(const grSim_Replacement& from);
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
  void InternalSwap(grSim_Replacement* other);
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

  // repeated .grSim_RobotReplacement robots = 2;
  int robots_size() const;
  void clear_robots();
  static const int kRobotsFieldNumber = 2;
  const ::grSim_RobotReplacement& robots(int index) const;
  ::grSim_RobotReplacement* mutable_robots(int index);
  ::grSim_RobotReplacement* add_robots();
  ::google::protobuf::RepeatedPtrField< ::grSim_RobotReplacement >*
      mutable_robots();
  const ::google::protobuf::RepeatedPtrField< ::grSim_RobotReplacement >&
      robots() const;

  // .grSim_BallReplacement ball = 1;
  bool has_ball() const;
  void clear_ball();
  static const int kBallFieldNumber = 1;
  const ::grSim_BallReplacement& ball() const;
  ::grSim_BallReplacement* mutable_ball();
  ::grSim_BallReplacement* release_ball();
  void set_allocated_ball(::grSim_BallReplacement* ball);

  // @@protoc_insertion_point(class_scope:grSim_Replacement)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::grSim_RobotReplacement > robots_;
  ::grSim_BallReplacement* ball_;
  mutable int _cached_size_;
  friend struct protobuf_grSim_5fReplacement_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// grSim_RobotReplacement

// double x = 1;
inline void grSim_RobotReplacement::clear_x() {
  x_ = 0;
}
inline double grSim_RobotReplacement::x() const {
  // @@protoc_insertion_point(field_get:grSim_RobotReplacement.x)
  return x_;
}
inline void grSim_RobotReplacement::set_x(double value) {
  
  x_ = value;
  // @@protoc_insertion_point(field_set:grSim_RobotReplacement.x)
}

// double y = 2;
inline void grSim_RobotReplacement::clear_y() {
  y_ = 0;
}
inline double grSim_RobotReplacement::y() const {
  // @@protoc_insertion_point(field_get:grSim_RobotReplacement.y)
  return y_;
}
inline void grSim_RobotReplacement::set_y(double value) {
  
  y_ = value;
  // @@protoc_insertion_point(field_set:grSim_RobotReplacement.y)
}

// double dir = 3;
inline void grSim_RobotReplacement::clear_dir() {
  dir_ = 0;
}
inline double grSim_RobotReplacement::dir() const {
  // @@protoc_insertion_point(field_get:grSim_RobotReplacement.dir)
  return dir_;
}
inline void grSim_RobotReplacement::set_dir(double value) {
  
  dir_ = value;
  // @@protoc_insertion_point(field_set:grSim_RobotReplacement.dir)
}

// uint32 id = 4;
inline void grSim_RobotReplacement::clear_id() {
  id_ = 0u;
}
inline ::google::protobuf::uint32 grSim_RobotReplacement::id() const {
  // @@protoc_insertion_point(field_get:grSim_RobotReplacement.id)
  return id_;
}
inline void grSim_RobotReplacement::set_id(::google::protobuf::uint32 value) {
  
  id_ = value;
  // @@protoc_insertion_point(field_set:grSim_RobotReplacement.id)
}

// bool yellowteam = 5;
inline void grSim_RobotReplacement::clear_yellowteam() {
  yellowteam_ = false;
}
inline bool grSim_RobotReplacement::yellowteam() const {
  // @@protoc_insertion_point(field_get:grSim_RobotReplacement.yellowteam)
  return yellowteam_;
}
inline void grSim_RobotReplacement::set_yellowteam(bool value) {
  
  yellowteam_ = value;
  // @@protoc_insertion_point(field_set:grSim_RobotReplacement.yellowteam)
}

// bool turnon = 6;
inline void grSim_RobotReplacement::clear_turnon() {
  turnon_ = false;
}
inline bool grSim_RobotReplacement::turnon() const {
  // @@protoc_insertion_point(field_get:grSim_RobotReplacement.turnon)
  return turnon_;
}
inline void grSim_RobotReplacement::set_turnon(bool value) {
  
  turnon_ = value;
  // @@protoc_insertion_point(field_set:grSim_RobotReplacement.turnon)
}

// -------------------------------------------------------------------

// grSim_BallReplacement

// double x = 1;
inline void grSim_BallReplacement::clear_x() {
  x_ = 0;
}
inline double grSim_BallReplacement::x() const {
  // @@protoc_insertion_point(field_get:grSim_BallReplacement.x)
  return x_;
}
inline void grSim_BallReplacement::set_x(double value) {
  
  x_ = value;
  // @@protoc_insertion_point(field_set:grSim_BallReplacement.x)
}

// double y = 2;
inline void grSim_BallReplacement::clear_y() {
  y_ = 0;
}
inline double grSim_BallReplacement::y() const {
  // @@protoc_insertion_point(field_get:grSim_BallReplacement.y)
  return y_;
}
inline void grSim_BallReplacement::set_y(double value) {
  
  y_ = value;
  // @@protoc_insertion_point(field_set:grSim_BallReplacement.y)
}

// double vx = 3;
inline void grSim_BallReplacement::clear_vx() {
  vx_ = 0;
}
inline double grSim_BallReplacement::vx() const {
  // @@protoc_insertion_point(field_get:grSim_BallReplacement.vx)
  return vx_;
}
inline void grSim_BallReplacement::set_vx(double value) {
  
  vx_ = value;
  // @@protoc_insertion_point(field_set:grSim_BallReplacement.vx)
}

// double vy = 4;
inline void grSim_BallReplacement::clear_vy() {
  vy_ = 0;
}
inline double grSim_BallReplacement::vy() const {
  // @@protoc_insertion_point(field_get:grSim_BallReplacement.vy)
  return vy_;
}
inline void grSim_BallReplacement::set_vy(double value) {
  
  vy_ = value;
  // @@protoc_insertion_point(field_set:grSim_BallReplacement.vy)
}

// -------------------------------------------------------------------

// grSim_Replacement

// .grSim_BallReplacement ball = 1;
inline bool grSim_Replacement::has_ball() const {
  return this != internal_default_instance() && ball_ != NULL;
}
inline void grSim_Replacement::clear_ball() {
  if (GetArenaNoVirtual() == NULL && ball_ != NULL) delete ball_;
  ball_ = NULL;
}
inline const ::grSim_BallReplacement& grSim_Replacement::ball() const {
  // @@protoc_insertion_point(field_get:grSim_Replacement.ball)
  return ball_ != NULL ? *ball_
                         : *::grSim_BallReplacement::internal_default_instance();
}
inline ::grSim_BallReplacement* grSim_Replacement::mutable_ball() {
  
  if (ball_ == NULL) {
    ball_ = new ::grSim_BallReplacement;
  }
  // @@protoc_insertion_point(field_mutable:grSim_Replacement.ball)
  return ball_;
}
inline ::grSim_BallReplacement* grSim_Replacement::release_ball() {
  // @@protoc_insertion_point(field_release:grSim_Replacement.ball)
  
  ::grSim_BallReplacement* temp = ball_;
  ball_ = NULL;
  return temp;
}
inline void grSim_Replacement::set_allocated_ball(::grSim_BallReplacement* ball) {
  delete ball_;
  ball_ = ball;
  if (ball) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:grSim_Replacement.ball)
}

// repeated .grSim_RobotReplacement robots = 2;
inline int grSim_Replacement::robots_size() const {
  return robots_.size();
}
inline void grSim_Replacement::clear_robots() {
  robots_.Clear();
}
inline const ::grSim_RobotReplacement& grSim_Replacement::robots(int index) const {
  // @@protoc_insertion_point(field_get:grSim_Replacement.robots)
  return robots_.Get(index);
}
inline ::grSim_RobotReplacement* grSim_Replacement::mutable_robots(int index) {
  // @@protoc_insertion_point(field_mutable:grSim_Replacement.robots)
  return robots_.Mutable(index);
}
inline ::grSim_RobotReplacement* grSim_Replacement::add_robots() {
  // @@protoc_insertion_point(field_add:grSim_Replacement.robots)
  return robots_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::grSim_RobotReplacement >*
grSim_Replacement::mutable_robots() {
  // @@protoc_insertion_point(field_mutable_list:grSim_Replacement.robots)
  return &robots_;
}
inline const ::google::protobuf::RepeatedPtrField< ::grSim_RobotReplacement >&
grSim_Replacement::robots() const {
  // @@protoc_insertion_point(field_list:grSim_Replacement.robots)
  return robots_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_grSim_5fReplacement_2eproto__INCLUDED