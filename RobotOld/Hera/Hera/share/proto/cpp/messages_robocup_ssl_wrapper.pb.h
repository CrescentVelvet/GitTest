// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: messages_robocup_ssl_wrapper.proto

#ifndef PROTOBUF_messages_5frobocup_5fssl_5fwrapper_2eproto__INCLUDED
#define PROTOBUF_messages_5frobocup_5fssl_5fwrapper_2eproto__INCLUDED

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
#include "messages_robocup_ssl_detection.pb.h"
#include "messages_robocup_ssl_geometry.pb.h"
// @@protoc_insertion_point(includes)
class SSL_DetectionBall;
class SSL_DetectionBallDefaultTypeInternal;
extern SSL_DetectionBallDefaultTypeInternal _SSL_DetectionBall_default_instance_;
class SSL_DetectionFrame;
class SSL_DetectionFrameDefaultTypeInternal;
extern SSL_DetectionFrameDefaultTypeInternal _SSL_DetectionFrame_default_instance_;
class SSL_DetectionRobot;
class SSL_DetectionRobotDefaultTypeInternal;
extern SSL_DetectionRobotDefaultTypeInternal _SSL_DetectionRobot_default_instance_;
class SSL_FieldCicularArc;
class SSL_FieldCicularArcDefaultTypeInternal;
extern SSL_FieldCicularArcDefaultTypeInternal _SSL_FieldCicularArc_default_instance_;
class SSL_FieldLineSegment;
class SSL_FieldLineSegmentDefaultTypeInternal;
extern SSL_FieldLineSegmentDefaultTypeInternal _SSL_FieldLineSegment_default_instance_;
class SSL_GeometryCameraCalibration;
class SSL_GeometryCameraCalibrationDefaultTypeInternal;
extern SSL_GeometryCameraCalibrationDefaultTypeInternal _SSL_GeometryCameraCalibration_default_instance_;
class SSL_GeometryData;
class SSL_GeometryDataDefaultTypeInternal;
extern SSL_GeometryDataDefaultTypeInternal _SSL_GeometryData_default_instance_;
class SSL_GeometryFieldSize;
class SSL_GeometryFieldSizeDefaultTypeInternal;
extern SSL_GeometryFieldSizeDefaultTypeInternal _SSL_GeometryFieldSize_default_instance_;
class SSL_WrapperPacket;
class SSL_WrapperPacketDefaultTypeInternal;
extern SSL_WrapperPacketDefaultTypeInternal _SSL_WrapperPacket_default_instance_;
class Vector2f;
class Vector2fDefaultTypeInternal;
extern Vector2fDefaultTypeInternal _Vector2f_default_instance_;

namespace protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto {
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
}  // namespace protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto

// ===================================================================

class SSL_WrapperPacket : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:SSL_WrapperPacket) */ {
 public:
  SSL_WrapperPacket();
  virtual ~SSL_WrapperPacket();

  SSL_WrapperPacket(const SSL_WrapperPacket& from);

  inline SSL_WrapperPacket& operator=(const SSL_WrapperPacket& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const SSL_WrapperPacket& default_instance();

  static inline const SSL_WrapperPacket* internal_default_instance() {
    return reinterpret_cast<const SSL_WrapperPacket*>(
               &_SSL_WrapperPacket_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(SSL_WrapperPacket* other);

  // implements Message ----------------------------------------------

  inline SSL_WrapperPacket* New() const PROTOBUF_FINAL { return New(NULL); }

  SSL_WrapperPacket* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const SSL_WrapperPacket& from);
  void MergeFrom(const SSL_WrapperPacket& from);
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
  void InternalSwap(SSL_WrapperPacket* other);
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

  // optional .SSL_DetectionFrame detection = 1;
  bool has_detection() const;
  void clear_detection();
  static const int kDetectionFieldNumber = 1;
  const ::SSL_DetectionFrame& detection() const;
  ::SSL_DetectionFrame* mutable_detection();
  ::SSL_DetectionFrame* release_detection();
  void set_allocated_detection(::SSL_DetectionFrame* detection);

  // optional .SSL_GeometryData geometry = 2;
  bool has_geometry() const;
  void clear_geometry();
  static const int kGeometryFieldNumber = 2;
  const ::SSL_GeometryData& geometry() const;
  ::SSL_GeometryData* mutable_geometry();
  ::SSL_GeometryData* release_geometry();
  void set_allocated_geometry(::SSL_GeometryData* geometry);

  // @@protoc_insertion_point(class_scope:SSL_WrapperPacket)
 private:
  void set_has_detection();
  void clear_has_detection();
  void set_has_geometry();
  void clear_has_geometry();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::SSL_DetectionFrame* detection_;
  ::SSL_GeometryData* geometry_;
  friend struct protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// SSL_WrapperPacket

// optional .SSL_DetectionFrame detection = 1;
inline bool SSL_WrapperPacket::has_detection() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void SSL_WrapperPacket::set_has_detection() {
  _has_bits_[0] |= 0x00000001u;
}
inline void SSL_WrapperPacket::clear_has_detection() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void SSL_WrapperPacket::clear_detection() {
  if (detection_ != NULL) detection_->::SSL_DetectionFrame::Clear();
  clear_has_detection();
}
inline const ::SSL_DetectionFrame& SSL_WrapperPacket::detection() const {
  // @@protoc_insertion_point(field_get:SSL_WrapperPacket.detection)
  return detection_ != NULL ? *detection_
                         : *::SSL_DetectionFrame::internal_default_instance();
}
inline ::SSL_DetectionFrame* SSL_WrapperPacket::mutable_detection() {
  set_has_detection();
  if (detection_ == NULL) {
    detection_ = new ::SSL_DetectionFrame;
  }
  // @@protoc_insertion_point(field_mutable:SSL_WrapperPacket.detection)
  return detection_;
}
inline ::SSL_DetectionFrame* SSL_WrapperPacket::release_detection() {
  // @@protoc_insertion_point(field_release:SSL_WrapperPacket.detection)
  clear_has_detection();
  ::SSL_DetectionFrame* temp = detection_;
  detection_ = NULL;
  return temp;
}
inline void SSL_WrapperPacket::set_allocated_detection(::SSL_DetectionFrame* detection) {
  delete detection_;
  detection_ = detection;
  if (detection) {
    set_has_detection();
  } else {
    clear_has_detection();
  }
  // @@protoc_insertion_point(field_set_allocated:SSL_WrapperPacket.detection)
}

// optional .SSL_GeometryData geometry = 2;
inline bool SSL_WrapperPacket::has_geometry() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void SSL_WrapperPacket::set_has_geometry() {
  _has_bits_[0] |= 0x00000002u;
}
inline void SSL_WrapperPacket::clear_has_geometry() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void SSL_WrapperPacket::clear_geometry() {
  if (geometry_ != NULL) geometry_->::SSL_GeometryData::Clear();
  clear_has_geometry();
}
inline const ::SSL_GeometryData& SSL_WrapperPacket::geometry() const {
  // @@protoc_insertion_point(field_get:SSL_WrapperPacket.geometry)
  return geometry_ != NULL ? *geometry_
                         : *::SSL_GeometryData::internal_default_instance();
}
inline ::SSL_GeometryData* SSL_WrapperPacket::mutable_geometry() {
  set_has_geometry();
  if (geometry_ == NULL) {
    geometry_ = new ::SSL_GeometryData;
  }
  // @@protoc_insertion_point(field_mutable:SSL_WrapperPacket.geometry)
  return geometry_;
}
inline ::SSL_GeometryData* SSL_WrapperPacket::release_geometry() {
  // @@protoc_insertion_point(field_release:SSL_WrapperPacket.geometry)
  clear_has_geometry();
  ::SSL_GeometryData* temp = geometry_;
  geometry_ = NULL;
  return temp;
}
inline void SSL_WrapperPacket::set_allocated_geometry(::SSL_GeometryData* geometry) {
  delete geometry_;
  geometry_ = geometry;
  if (geometry) {
    set_has_geometry();
  } else {
    clear_has_geometry();
  }
  // @@protoc_insertion_point(field_set_allocated:SSL_WrapperPacket.geometry)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_messages_5frobocup_5fssl_5fwrapper_2eproto__INCLUDED
