// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: vision_detection.proto

#ifndef PROTOBUF_vision_5fdetection_2eproto__INCLUDED
#define PROTOBUF_vision_5fdetection_2eproto__INCLUDED

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
class Vision_DetectionBall;
class Vision_DetectionBallDefaultTypeInternal;
extern Vision_DetectionBallDefaultTypeInternal _Vision_DetectionBall_default_instance_;
class Vision_DetectionFrame;
class Vision_DetectionFrameDefaultTypeInternal;
extern Vision_DetectionFrameDefaultTypeInternal _Vision_DetectionFrame_default_instance_;
class Vision_DetectionRobot;
class Vision_DetectionRobotDefaultTypeInternal;
extern Vision_DetectionRobotDefaultTypeInternal _Vision_DetectionRobot_default_instance_;

namespace protobuf_vision_5fdetection_2eproto {
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
}  // namespace protobuf_vision_5fdetection_2eproto

// ===================================================================

class Vision_DetectionBall : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Vision_DetectionBall) */ {
 public:
  Vision_DetectionBall();
  virtual ~Vision_DetectionBall();

  Vision_DetectionBall(const Vision_DetectionBall& from);

  inline Vision_DetectionBall& operator=(const Vision_DetectionBall& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Vision_DetectionBall& default_instance();

  static inline const Vision_DetectionBall* internal_default_instance() {
    return reinterpret_cast<const Vision_DetectionBall*>(
               &_Vision_DetectionBall_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(Vision_DetectionBall* other);

  // implements Message ----------------------------------------------

  inline Vision_DetectionBall* New() const PROTOBUF_FINAL { return New(NULL); }

  Vision_DetectionBall* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Vision_DetectionBall& from);
  void MergeFrom(const Vision_DetectionBall& from);
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
  void InternalSwap(Vision_DetectionBall* other);
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

  // float vel_x = 1;
  void clear_vel_x();
  static const int kVelXFieldNumber = 1;
  float vel_x() const;
  void set_vel_x(float value);

  // float vel_y = 2;
  void clear_vel_y();
  static const int kVelYFieldNumber = 2;
  float vel_y() const;
  void set_vel_y(float value);

  // uint32 area = 3;
  void clear_area();
  static const int kAreaFieldNumber = 3;
  ::google::protobuf::uint32 area() const;
  void set_area(::google::protobuf::uint32 value);

  // float x = 4;
  void clear_x();
  static const int kXFieldNumber = 4;
  float x() const;
  void set_x(float value);

  // float y = 5;
  void clear_y();
  static const int kYFieldNumber = 5;
  float y() const;
  void set_y(float value);

  // float height = 6;
  void clear_height();
  static const int kHeightFieldNumber = 6;
  float height() const;
  void set_height(float value);

  // uint32 ball_state = 7;
  void clear_ball_state();
  static const int kBallStateFieldNumber = 7;
  ::google::protobuf::uint32 ball_state() const;
  void set_ball_state(::google::protobuf::uint32 value);

  // uint32 last_touch = 8;
  void clear_last_touch();
  static const int kLastTouchFieldNumber = 8;
  ::google::protobuf::uint32 last_touch() const;
  void set_last_touch(::google::protobuf::uint32 value);

  // bool valid = 9;
  void clear_valid();
  static const int kValidFieldNumber = 9;
  bool valid() const;
  void set_valid(bool value);

  // @@protoc_insertion_point(class_scope:Vision_DetectionBall)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  float vel_x_;
  float vel_y_;
  ::google::protobuf::uint32 area_;
  float x_;
  float y_;
  float height_;
  ::google::protobuf::uint32 ball_state_;
  ::google::protobuf::uint32 last_touch_;
  bool valid_;
  mutable int _cached_size_;
  friend struct protobuf_vision_5fdetection_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Vision_DetectionRobot : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Vision_DetectionRobot) */ {
 public:
  Vision_DetectionRobot();
  virtual ~Vision_DetectionRobot();

  Vision_DetectionRobot(const Vision_DetectionRobot& from);

  inline Vision_DetectionRobot& operator=(const Vision_DetectionRobot& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Vision_DetectionRobot& default_instance();

  static inline const Vision_DetectionRobot* internal_default_instance() {
    return reinterpret_cast<const Vision_DetectionRobot*>(
               &_Vision_DetectionRobot_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(Vision_DetectionRobot* other);

  // implements Message ----------------------------------------------

  inline Vision_DetectionRobot* New() const PROTOBUF_FINAL { return New(NULL); }

  Vision_DetectionRobot* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Vision_DetectionRobot& from);
  void MergeFrom(const Vision_DetectionRobot& from);
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
  void InternalSwap(Vision_DetectionRobot* other);
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

  // float confidence = 1;
  void clear_confidence();
  static const int kConfidenceFieldNumber = 1;
  float confidence() const;
  void set_confidence(float value);

  // uint32 robot_id = 2;
  void clear_robot_id();
  static const int kRobotIdFieldNumber = 2;
  ::google::protobuf::uint32 robot_id() const;
  void set_robot_id(::google::protobuf::uint32 value);

  // float x = 3;
  void clear_x();
  static const int kXFieldNumber = 3;
  float x() const;
  void set_x(float value);

  // float y = 4;
  void clear_y();
  static const int kYFieldNumber = 4;
  float y() const;
  void set_y(float value);

  // float orientation = 5;
  void clear_orientation();
  static const int kOrientationFieldNumber = 5;
  float orientation() const;
  void set_orientation(float value);

  // float vel_x = 6;
  void clear_vel_x();
  static const int kVelXFieldNumber = 6;
  float vel_x() const;
  void set_vel_x(float value);

  // float vel_y = 7;
  void clear_vel_y();
  static const int kVelYFieldNumber = 7;
  float vel_y() const;
  void set_vel_y(float value);

  // float rotate_vel = 8;
  void clear_rotate_vel();
  static const int kRotateVelFieldNumber = 8;
  float rotate_vel() const;
  void set_rotate_vel(float value);

  // @@protoc_insertion_point(class_scope:Vision_DetectionRobot)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  float confidence_;
  ::google::protobuf::uint32 robot_id_;
  float x_;
  float y_;
  float orientation_;
  float vel_x_;
  float vel_y_;
  float rotate_vel_;
  mutable int _cached_size_;
  friend struct protobuf_vision_5fdetection_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Vision_DetectionFrame : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Vision_DetectionFrame) */ {
 public:
  Vision_DetectionFrame();
  virtual ~Vision_DetectionFrame();

  Vision_DetectionFrame(const Vision_DetectionFrame& from);

  inline Vision_DetectionFrame& operator=(const Vision_DetectionFrame& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Vision_DetectionFrame& default_instance();

  static inline const Vision_DetectionFrame* internal_default_instance() {
    return reinterpret_cast<const Vision_DetectionFrame*>(
               &_Vision_DetectionFrame_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    2;

  void Swap(Vision_DetectionFrame* other);

  // implements Message ----------------------------------------------

  inline Vision_DetectionFrame* New() const PROTOBUF_FINAL { return New(NULL); }

  Vision_DetectionFrame* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Vision_DetectionFrame& from);
  void MergeFrom(const Vision_DetectionFrame& from);
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
  void InternalSwap(Vision_DetectionFrame* other);
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

  // repeated .Vision_DetectionRobot robots_yellow = 2;
  int robots_yellow_size() const;
  void clear_robots_yellow();
  static const int kRobotsYellowFieldNumber = 2;
  const ::Vision_DetectionRobot& robots_yellow(int index) const;
  ::Vision_DetectionRobot* mutable_robots_yellow(int index);
  ::Vision_DetectionRobot* add_robots_yellow();
  ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >*
      mutable_robots_yellow();
  const ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >&
      robots_yellow() const;

  // repeated .Vision_DetectionRobot robots_blue = 3;
  int robots_blue_size() const;
  void clear_robots_blue();
  static const int kRobotsBlueFieldNumber = 3;
  const ::Vision_DetectionRobot& robots_blue(int index) const;
  ::Vision_DetectionRobot* mutable_robots_blue(int index);
  ::Vision_DetectionRobot* add_robots_blue();
  ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >*
      mutable_robots_blue();
  const ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >&
      robots_blue() const;

  // .Vision_DetectionBall balls = 1;
  bool has_balls() const;
  void clear_balls();
  static const int kBallsFieldNumber = 1;
  const ::Vision_DetectionBall& balls() const;
  ::Vision_DetectionBall* mutable_balls();
  ::Vision_DetectionBall* release_balls();
  void set_allocated_balls(::Vision_DetectionBall* balls);

  // @@protoc_insertion_point(class_scope:Vision_DetectionFrame)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot > robots_yellow_;
  ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot > robots_blue_;
  ::Vision_DetectionBall* balls_;
  mutable int _cached_size_;
  friend struct protobuf_vision_5fdetection_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Vision_DetectionBall

// float vel_x = 1;
inline void Vision_DetectionBall::clear_vel_x() {
  vel_x_ = 0;
}
inline float Vision_DetectionBall::vel_x() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.vel_x)
  return vel_x_;
}
inline void Vision_DetectionBall::set_vel_x(float value) {
  
  vel_x_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.vel_x)
}

// float vel_y = 2;
inline void Vision_DetectionBall::clear_vel_y() {
  vel_y_ = 0;
}
inline float Vision_DetectionBall::vel_y() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.vel_y)
  return vel_y_;
}
inline void Vision_DetectionBall::set_vel_y(float value) {
  
  vel_y_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.vel_y)
}

// uint32 area = 3;
inline void Vision_DetectionBall::clear_area() {
  area_ = 0u;
}
inline ::google::protobuf::uint32 Vision_DetectionBall::area() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.area)
  return area_;
}
inline void Vision_DetectionBall::set_area(::google::protobuf::uint32 value) {
  
  area_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.area)
}

// float x = 4;
inline void Vision_DetectionBall::clear_x() {
  x_ = 0;
}
inline float Vision_DetectionBall::x() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.x)
  return x_;
}
inline void Vision_DetectionBall::set_x(float value) {
  
  x_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.x)
}

// float y = 5;
inline void Vision_DetectionBall::clear_y() {
  y_ = 0;
}
inline float Vision_DetectionBall::y() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.y)
  return y_;
}
inline void Vision_DetectionBall::set_y(float value) {
  
  y_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.y)
}

// float height = 6;
inline void Vision_DetectionBall::clear_height() {
  height_ = 0;
}
inline float Vision_DetectionBall::height() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.height)
  return height_;
}
inline void Vision_DetectionBall::set_height(float value) {
  
  height_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.height)
}

// uint32 ball_state = 7;
inline void Vision_DetectionBall::clear_ball_state() {
  ball_state_ = 0u;
}
inline ::google::protobuf::uint32 Vision_DetectionBall::ball_state() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.ball_state)
  return ball_state_;
}
inline void Vision_DetectionBall::set_ball_state(::google::protobuf::uint32 value) {
  
  ball_state_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.ball_state)
}

// uint32 last_touch = 8;
inline void Vision_DetectionBall::clear_last_touch() {
  last_touch_ = 0u;
}
inline ::google::protobuf::uint32 Vision_DetectionBall::last_touch() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.last_touch)
  return last_touch_;
}
inline void Vision_DetectionBall::set_last_touch(::google::protobuf::uint32 value) {
  
  last_touch_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.last_touch)
}

// bool valid = 9;
inline void Vision_DetectionBall::clear_valid() {
  valid_ = false;
}
inline bool Vision_DetectionBall::valid() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionBall.valid)
  return valid_;
}
inline void Vision_DetectionBall::set_valid(bool value) {
  
  valid_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionBall.valid)
}

// -------------------------------------------------------------------

// Vision_DetectionRobot

// float confidence = 1;
inline void Vision_DetectionRobot::clear_confidence() {
  confidence_ = 0;
}
inline float Vision_DetectionRobot::confidence() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.confidence)
  return confidence_;
}
inline void Vision_DetectionRobot::set_confidence(float value) {
  
  confidence_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.confidence)
}

// uint32 robot_id = 2;
inline void Vision_DetectionRobot::clear_robot_id() {
  robot_id_ = 0u;
}
inline ::google::protobuf::uint32 Vision_DetectionRobot::robot_id() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.robot_id)
  return robot_id_;
}
inline void Vision_DetectionRobot::set_robot_id(::google::protobuf::uint32 value) {
  
  robot_id_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.robot_id)
}

// float x = 3;
inline void Vision_DetectionRobot::clear_x() {
  x_ = 0;
}
inline float Vision_DetectionRobot::x() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.x)
  return x_;
}
inline void Vision_DetectionRobot::set_x(float value) {
  
  x_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.x)
}

// float y = 4;
inline void Vision_DetectionRobot::clear_y() {
  y_ = 0;
}
inline float Vision_DetectionRobot::y() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.y)
  return y_;
}
inline void Vision_DetectionRobot::set_y(float value) {
  
  y_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.y)
}

// float orientation = 5;
inline void Vision_DetectionRobot::clear_orientation() {
  orientation_ = 0;
}
inline float Vision_DetectionRobot::orientation() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.orientation)
  return orientation_;
}
inline void Vision_DetectionRobot::set_orientation(float value) {
  
  orientation_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.orientation)
}

// float vel_x = 6;
inline void Vision_DetectionRobot::clear_vel_x() {
  vel_x_ = 0;
}
inline float Vision_DetectionRobot::vel_x() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.vel_x)
  return vel_x_;
}
inline void Vision_DetectionRobot::set_vel_x(float value) {
  
  vel_x_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.vel_x)
}

// float vel_y = 7;
inline void Vision_DetectionRobot::clear_vel_y() {
  vel_y_ = 0;
}
inline float Vision_DetectionRobot::vel_y() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.vel_y)
  return vel_y_;
}
inline void Vision_DetectionRobot::set_vel_y(float value) {
  
  vel_y_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.vel_y)
}

// float rotate_vel = 8;
inline void Vision_DetectionRobot::clear_rotate_vel() {
  rotate_vel_ = 0;
}
inline float Vision_DetectionRobot::rotate_vel() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionRobot.rotate_vel)
  return rotate_vel_;
}
inline void Vision_DetectionRobot::set_rotate_vel(float value) {
  
  rotate_vel_ = value;
  // @@protoc_insertion_point(field_set:Vision_DetectionRobot.rotate_vel)
}

// -------------------------------------------------------------------

// Vision_DetectionFrame

// .Vision_DetectionBall balls = 1;
inline bool Vision_DetectionFrame::has_balls() const {
  return this != internal_default_instance() && balls_ != NULL;
}
inline void Vision_DetectionFrame::clear_balls() {
  if (GetArenaNoVirtual() == NULL && balls_ != NULL) delete balls_;
  balls_ = NULL;
}
inline const ::Vision_DetectionBall& Vision_DetectionFrame::balls() const {
  // @@protoc_insertion_point(field_get:Vision_DetectionFrame.balls)
  return balls_ != NULL ? *balls_
                         : *::Vision_DetectionBall::internal_default_instance();
}
inline ::Vision_DetectionBall* Vision_DetectionFrame::mutable_balls() {
  
  if (balls_ == NULL) {
    balls_ = new ::Vision_DetectionBall;
  }
  // @@protoc_insertion_point(field_mutable:Vision_DetectionFrame.balls)
  return balls_;
}
inline ::Vision_DetectionBall* Vision_DetectionFrame::release_balls() {
  // @@protoc_insertion_point(field_release:Vision_DetectionFrame.balls)
  
  ::Vision_DetectionBall* temp = balls_;
  balls_ = NULL;
  return temp;
}
inline void Vision_DetectionFrame::set_allocated_balls(::Vision_DetectionBall* balls) {
  delete balls_;
  balls_ = balls;
  if (balls) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:Vision_DetectionFrame.balls)
}

// repeated .Vision_DetectionRobot robots_yellow = 2;
inline int Vision_DetectionFrame::robots_yellow_size() const {
  return robots_yellow_.size();
}
inline void Vision_DetectionFrame::clear_robots_yellow() {
  robots_yellow_.Clear();
}
inline const ::Vision_DetectionRobot& Vision_DetectionFrame::robots_yellow(int index) const {
  // @@protoc_insertion_point(field_get:Vision_DetectionFrame.robots_yellow)
  return robots_yellow_.Get(index);
}
inline ::Vision_DetectionRobot* Vision_DetectionFrame::mutable_robots_yellow(int index) {
  // @@protoc_insertion_point(field_mutable:Vision_DetectionFrame.robots_yellow)
  return robots_yellow_.Mutable(index);
}
inline ::Vision_DetectionRobot* Vision_DetectionFrame::add_robots_yellow() {
  // @@protoc_insertion_point(field_add:Vision_DetectionFrame.robots_yellow)
  return robots_yellow_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >*
Vision_DetectionFrame::mutable_robots_yellow() {
  // @@protoc_insertion_point(field_mutable_list:Vision_DetectionFrame.robots_yellow)
  return &robots_yellow_;
}
inline const ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >&
Vision_DetectionFrame::robots_yellow() const {
  // @@protoc_insertion_point(field_list:Vision_DetectionFrame.robots_yellow)
  return robots_yellow_;
}

// repeated .Vision_DetectionRobot robots_blue = 3;
inline int Vision_DetectionFrame::robots_blue_size() const {
  return robots_blue_.size();
}
inline void Vision_DetectionFrame::clear_robots_blue() {
  robots_blue_.Clear();
}
inline const ::Vision_DetectionRobot& Vision_DetectionFrame::robots_blue(int index) const {
  // @@protoc_insertion_point(field_get:Vision_DetectionFrame.robots_blue)
  return robots_blue_.Get(index);
}
inline ::Vision_DetectionRobot* Vision_DetectionFrame::mutable_robots_blue(int index) {
  // @@protoc_insertion_point(field_mutable:Vision_DetectionFrame.robots_blue)
  return robots_blue_.Mutable(index);
}
inline ::Vision_DetectionRobot* Vision_DetectionFrame::add_robots_blue() {
  // @@protoc_insertion_point(field_add:Vision_DetectionFrame.robots_blue)
  return robots_blue_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >*
Vision_DetectionFrame::mutable_robots_blue() {
  // @@protoc_insertion_point(field_mutable_list:Vision_DetectionFrame.robots_blue)
  return &robots_blue_;
}
inline const ::google::protobuf::RepeatedPtrField< ::Vision_DetectionRobot >&
Vision_DetectionFrame::robots_blue() const {
  // @@protoc_insertion_point(field_list:Vision_DetectionFrame.robots_blue)
  return robots_blue_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_vision_5fdetection_2eproto__INCLUDED
