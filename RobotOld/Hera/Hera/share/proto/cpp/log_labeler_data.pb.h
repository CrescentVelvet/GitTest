// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: log_labeler_data.proto

#ifndef PROTOBUF_log_5flabeler_5fdata_2eproto__INCLUDED
#define PROTOBUF_log_5flabeler_5fdata_2eproto__INCLUDED

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
#include "messages_robocup_ssl_wrapper.pb.h"
#include "ssl_referee.pb.h"
// @@protoc_insertion_point(includes)
class LabelerFrame;
class LabelerFrameDefaultTypeInternal;
extern LabelerFrameDefaultTypeInternal _LabelerFrame_default_instance_;
class LabelerFrameGroup;
class LabelerFrameGroupDefaultTypeInternal;
extern LabelerFrameGroupDefaultTypeInternal _LabelerFrameGroup_default_instance_;
class LabelerMetadata;
class LabelerMetadataDefaultTypeInternal;
extern LabelerMetadataDefaultTypeInternal _LabelerMetadata_default_instance_;
class ProposedGameEvent;
class ProposedGameEventDefaultTypeInternal;
extern ProposedGameEventDefaultTypeInternal _ProposedGameEvent_default_instance_;
class Referee;
class RefereeDefaultTypeInternal;
extern RefereeDefaultTypeInternal _Referee_default_instance_;
class Referee_Point;
class Referee_PointDefaultTypeInternal;
extern Referee_PointDefaultTypeInternal _Referee_Point_default_instance_;
class Referee_TeamInfo;
class Referee_TeamInfoDefaultTypeInternal;
extern Referee_TeamInfoDefaultTypeInternal _Referee_TeamInfo_default_instance_;
class SSL_WrapperPacket;
class SSL_WrapperPacketDefaultTypeInternal;
extern SSL_WrapperPacketDefaultTypeInternal _SSL_WrapperPacket_default_instance_;

namespace protobuf_log_5flabeler_5fdata_2eproto {
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
}  // namespace protobuf_log_5flabeler_5fdata_2eproto

// ===================================================================

class LabelerMetadata : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:LabelerMetadata) */ {
 public:
  LabelerMetadata();
  virtual ~LabelerMetadata();

  LabelerMetadata(const LabelerMetadata& from);

  inline LabelerMetadata& operator=(const LabelerMetadata& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const LabelerMetadata& default_instance();

  static inline const LabelerMetadata* internal_default_instance() {
    return reinterpret_cast<const LabelerMetadata*>(
               &_LabelerMetadata_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(LabelerMetadata* other);

  // implements Message ----------------------------------------------

  inline LabelerMetadata* New() const PROTOBUF_FINAL { return New(NULL); }

  LabelerMetadata* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const LabelerMetadata& from);
  void MergeFrom(const LabelerMetadata& from);
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
  void InternalSwap(LabelerMetadata* other);
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

  // repeated uint64 message_offsets = 2;
  int message_offsets_size() const;
  void clear_message_offsets();
  static const int kMessageOffsetsFieldNumber = 2;
  ::google::protobuf::uint64 message_offsets(int index) const;
  void set_message_offsets(int index, ::google::protobuf::uint64 value);
  void add_message_offsets(::google::protobuf::uint64 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >&
      message_offsets() const;
  ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >*
      mutable_message_offsets();

  // uint32 num_cameras = 1;
  void clear_num_cameras();
  static const int kNumCamerasFieldNumber = 1;
  ::google::protobuf::uint32 num_cameras() const;
  void set_num_cameras(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:LabelerMetadata)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedField< ::google::protobuf::uint64 > message_offsets_;
  mutable int _message_offsets_cached_byte_size_;
  ::google::protobuf::uint32 num_cameras_;
  mutable int _cached_size_;
  friend struct protobuf_log_5flabeler_5fdata_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class LabelerFrame : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:LabelerFrame) */ {
 public:
  LabelerFrame();
  virtual ~LabelerFrame();

  LabelerFrame(const LabelerFrame& from);

  inline LabelerFrame& operator=(const LabelerFrame& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const LabelerFrame& default_instance();

  enum FrameCase {
    kVisionFrame = 2,
    kRefereeFrame = 3,
    FRAME_NOT_SET = 0,
  };

  static inline const LabelerFrame* internal_default_instance() {
    return reinterpret_cast<const LabelerFrame*>(
               &_LabelerFrame_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(LabelerFrame* other);

  // implements Message ----------------------------------------------

  inline LabelerFrame* New() const PROTOBUF_FINAL { return New(NULL); }

  LabelerFrame* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const LabelerFrame& from);
  void MergeFrom(const LabelerFrame& from);
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
  void InternalSwap(LabelerFrame* other);
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

  // uint64 timestamp = 1;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  ::google::protobuf::uint64 timestamp() const;
  void set_timestamp(::google::protobuf::uint64 value);

  // .SSL_WrapperPacket vision_frame = 2;
  bool has_vision_frame() const;
  void clear_vision_frame();
  static const int kVisionFrameFieldNumber = 2;
  const ::SSL_WrapperPacket& vision_frame() const;
  ::SSL_WrapperPacket* mutable_vision_frame();
  ::SSL_WrapperPacket* release_vision_frame();
  void set_allocated_vision_frame(::SSL_WrapperPacket* vision_frame);

  // .Referee referee_frame = 3;
  bool has_referee_frame() const;
  void clear_referee_frame();
  static const int kRefereeFrameFieldNumber = 3;
  const ::Referee& referee_frame() const;
  ::Referee* mutable_referee_frame();
  ::Referee* release_referee_frame();
  void set_allocated_referee_frame(::Referee* referee_frame);

  FrameCase frame_case() const;
  // @@protoc_insertion_point(class_scope:LabelerFrame)
 private:
  void set_has_vision_frame();
  void set_has_referee_frame();

  inline bool has_frame() const;
  void clear_frame();
  inline void clear_has_frame();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint64 timestamp_;
  union FrameUnion {
    FrameUnion() {}
    ::SSL_WrapperPacket* vision_frame_;
    ::Referee* referee_frame_;
  } frame_;
  mutable int _cached_size_;
  ::google::protobuf::uint32 _oneof_case_[1];

  friend struct protobuf_log_5flabeler_5fdata_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class LabelerFrameGroup : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:LabelerFrameGroup) */ {
 public:
  LabelerFrameGroup();
  virtual ~LabelerFrameGroup();

  LabelerFrameGroup(const LabelerFrameGroup& from);

  inline LabelerFrameGroup& operator=(const LabelerFrameGroup& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const LabelerFrameGroup& default_instance();

  static inline const LabelerFrameGroup* internal_default_instance() {
    return reinterpret_cast<const LabelerFrameGroup*>(
               &_LabelerFrameGroup_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    2;

  void Swap(LabelerFrameGroup* other);

  // implements Message ----------------------------------------------

  inline LabelerFrameGroup* New() const PROTOBUF_FINAL { return New(NULL); }

  LabelerFrameGroup* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const LabelerFrameGroup& from);
  void MergeFrom(const LabelerFrameGroup& from);
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
  void InternalSwap(LabelerFrameGroup* other);
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

  // repeated .LabelerFrame frames = 1;
  int frames_size() const;
  void clear_frames();
  static const int kFramesFieldNumber = 1;
  const ::LabelerFrame& frames(int index) const;
  ::LabelerFrame* mutable_frames(int index);
  ::LabelerFrame* add_frames();
  ::google::protobuf::RepeatedPtrField< ::LabelerFrame >*
      mutable_frames();
  const ::google::protobuf::RepeatedPtrField< ::LabelerFrame >&
      frames() const;

  // @@protoc_insertion_point(class_scope:LabelerFrameGroup)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::LabelerFrame > frames_;
  mutable int _cached_size_;
  friend struct protobuf_log_5flabeler_5fdata_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// LabelerMetadata

// uint32 num_cameras = 1;
inline void LabelerMetadata::clear_num_cameras() {
  num_cameras_ = 0u;
}
inline ::google::protobuf::uint32 LabelerMetadata::num_cameras() const {
  // @@protoc_insertion_point(field_get:LabelerMetadata.num_cameras)
  return num_cameras_;
}
inline void LabelerMetadata::set_num_cameras(::google::protobuf::uint32 value) {
  
  num_cameras_ = value;
  // @@protoc_insertion_point(field_set:LabelerMetadata.num_cameras)
}

// repeated uint64 message_offsets = 2;
inline int LabelerMetadata::message_offsets_size() const {
  return message_offsets_.size();
}
inline void LabelerMetadata::clear_message_offsets() {
  message_offsets_.Clear();
}
inline ::google::protobuf::uint64 LabelerMetadata::message_offsets(int index) const {
  // @@protoc_insertion_point(field_get:LabelerMetadata.message_offsets)
  return message_offsets_.Get(index);
}
inline void LabelerMetadata::set_message_offsets(int index, ::google::protobuf::uint64 value) {
  message_offsets_.Set(index, value);
  // @@protoc_insertion_point(field_set:LabelerMetadata.message_offsets)
}
inline void LabelerMetadata::add_message_offsets(::google::protobuf::uint64 value) {
  message_offsets_.Add(value);
  // @@protoc_insertion_point(field_add:LabelerMetadata.message_offsets)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >&
LabelerMetadata::message_offsets() const {
  // @@protoc_insertion_point(field_list:LabelerMetadata.message_offsets)
  return message_offsets_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::uint64 >*
LabelerMetadata::mutable_message_offsets() {
  // @@protoc_insertion_point(field_mutable_list:LabelerMetadata.message_offsets)
  return &message_offsets_;
}

// -------------------------------------------------------------------

// LabelerFrame

// uint64 timestamp = 1;
inline void LabelerFrame::clear_timestamp() {
  timestamp_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 LabelerFrame::timestamp() const {
  // @@protoc_insertion_point(field_get:LabelerFrame.timestamp)
  return timestamp_;
}
inline void LabelerFrame::set_timestamp(::google::protobuf::uint64 value) {
  
  timestamp_ = value;
  // @@protoc_insertion_point(field_set:LabelerFrame.timestamp)
}

// .SSL_WrapperPacket vision_frame = 2;
inline bool LabelerFrame::has_vision_frame() const {
  return frame_case() == kVisionFrame;
}
inline void LabelerFrame::set_has_vision_frame() {
  _oneof_case_[0] = kVisionFrame;
}
inline void LabelerFrame::clear_vision_frame() {
  if (has_vision_frame()) {
    delete frame_.vision_frame_;
    clear_has_frame();
  }
}
inline  const ::SSL_WrapperPacket& LabelerFrame::vision_frame() const {
  // @@protoc_insertion_point(field_get:LabelerFrame.vision_frame)
  return has_vision_frame()
      ? *frame_.vision_frame_
      : ::SSL_WrapperPacket::default_instance();
}
inline ::SSL_WrapperPacket* LabelerFrame::mutable_vision_frame() {
  if (!has_vision_frame()) {
    clear_frame();
    set_has_vision_frame();
    frame_.vision_frame_ = new ::SSL_WrapperPacket;
  }
  // @@protoc_insertion_point(field_mutable:LabelerFrame.vision_frame)
  return frame_.vision_frame_;
}
inline ::SSL_WrapperPacket* LabelerFrame::release_vision_frame() {
  // @@protoc_insertion_point(field_release:LabelerFrame.vision_frame)
  if (has_vision_frame()) {
    clear_has_frame();
    ::SSL_WrapperPacket* temp = frame_.vision_frame_;
    frame_.vision_frame_ = NULL;
    return temp;
  } else {
    return NULL;
  }
}
inline void LabelerFrame::set_allocated_vision_frame(::SSL_WrapperPacket* vision_frame) {
  clear_frame();
  if (vision_frame) {
    set_has_vision_frame();
    frame_.vision_frame_ = vision_frame;
  }
  // @@protoc_insertion_point(field_set_allocated:LabelerFrame.vision_frame)
}

// .Referee referee_frame = 3;
inline bool LabelerFrame::has_referee_frame() const {
  return frame_case() == kRefereeFrame;
}
inline void LabelerFrame::set_has_referee_frame() {
  _oneof_case_[0] = kRefereeFrame;
}
inline void LabelerFrame::clear_referee_frame() {
  if (has_referee_frame()) {
    delete frame_.referee_frame_;
    clear_has_frame();
  }
}
inline  const ::Referee& LabelerFrame::referee_frame() const {
  // @@protoc_insertion_point(field_get:LabelerFrame.referee_frame)
  return has_referee_frame()
      ? *frame_.referee_frame_
      : ::Referee::default_instance();
}
inline ::Referee* LabelerFrame::mutable_referee_frame() {
  if (!has_referee_frame()) {
    clear_frame();
    set_has_referee_frame();
    frame_.referee_frame_ = new ::Referee;
  }
  // @@protoc_insertion_point(field_mutable:LabelerFrame.referee_frame)
  return frame_.referee_frame_;
}
inline ::Referee* LabelerFrame::release_referee_frame() {
  // @@protoc_insertion_point(field_release:LabelerFrame.referee_frame)
  if (has_referee_frame()) {
    clear_has_frame();
    ::Referee* temp = frame_.referee_frame_;
    frame_.referee_frame_ = NULL;
    return temp;
  } else {
    return NULL;
  }
}
inline void LabelerFrame::set_allocated_referee_frame(::Referee* referee_frame) {
  clear_frame();
  if (referee_frame) {
    set_has_referee_frame();
    frame_.referee_frame_ = referee_frame;
  }
  // @@protoc_insertion_point(field_set_allocated:LabelerFrame.referee_frame)
}

inline bool LabelerFrame::has_frame() const {
  return frame_case() != FRAME_NOT_SET;
}
inline void LabelerFrame::clear_has_frame() {
  _oneof_case_[0] = FRAME_NOT_SET;
}
inline LabelerFrame::FrameCase LabelerFrame::frame_case() const {
  return LabelerFrame::FrameCase(_oneof_case_[0]);
}
// -------------------------------------------------------------------

// LabelerFrameGroup

// repeated .LabelerFrame frames = 1;
inline int LabelerFrameGroup::frames_size() const {
  return frames_.size();
}
inline void LabelerFrameGroup::clear_frames() {
  frames_.Clear();
}
inline const ::LabelerFrame& LabelerFrameGroup::frames(int index) const {
  // @@protoc_insertion_point(field_get:LabelerFrameGroup.frames)
  return frames_.Get(index);
}
inline ::LabelerFrame* LabelerFrameGroup::mutable_frames(int index) {
  // @@protoc_insertion_point(field_mutable:LabelerFrameGroup.frames)
  return frames_.Mutable(index);
}
inline ::LabelerFrame* LabelerFrameGroup::add_frames() {
  // @@protoc_insertion_point(field_add:LabelerFrameGroup.frames)
  return frames_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::LabelerFrame >*
LabelerFrameGroup::mutable_frames() {
  // @@protoc_insertion_point(field_mutable_list:LabelerFrameGroup.frames)
  return &frames_;
}
inline const ::google::protobuf::RepeatedPtrField< ::LabelerFrame >&
LabelerFrameGroup::frames() const {
  // @@protoc_insertion_point(field_list:LabelerFrameGroup.frames)
  return frames_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_log_5flabeler_5fdata_2eproto__INCLUDED