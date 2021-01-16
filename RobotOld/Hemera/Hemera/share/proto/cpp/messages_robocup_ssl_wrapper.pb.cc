// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: messages_robocup_ssl_wrapper.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "messages_robocup_ssl_wrapper.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
class SSL_WrapperPacketDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<SSL_WrapperPacket> {
} _SSL_WrapperPacket_default_instance_;

namespace protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto {


namespace {

::google::protobuf::Metadata file_level_metadata[1];

}  // namespace

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTableField
    const TableStruct::entries[] = {
  {0, 0, 0, ::google::protobuf::internal::kInvalidMask, 0, 0},
};

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::AuxillaryParseTableField
    const TableStruct::aux[] = {
  ::google::protobuf::internal::AuxillaryParseTableField(),
};
PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTable const
    TableStruct::schema[] = {
  { NULL, NULL, 0, -1, -1, false },
};

const ::google::protobuf::uint32 TableStruct::offsets[] = {
  ~0u,  // no _has_bits_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SSL_WrapperPacket, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SSL_WrapperPacket, detection_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(SSL_WrapperPacket, geometry_),
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, -1, sizeof(SSL_WrapperPacket)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_SSL_WrapperPacket_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "messages_robocup_ssl_wrapper.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 1);
}

}  // namespace

void TableStruct::Shutdown() {
  _SSL_WrapperPacket_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  ::protobuf_messages_5frobocup_5fssl_5fdetection_2eproto::InitDefaults();
  ::protobuf_messages_5frobocup_5fssl_5fgeometry_2eproto::InitDefaults();
  _SSL_WrapperPacket_default_instance_.DefaultConstruct();
  _SSL_WrapperPacket_default_instance_.get_mutable()->detection_ = const_cast< ::SSL_DetectionFrame*>(
      ::SSL_DetectionFrame::internal_default_instance());
  _SSL_WrapperPacket_default_instance_.get_mutable()->geometry_ = const_cast< ::SSL_GeometryData*>(
      ::SSL_GeometryData::internal_default_instance());
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n\"messages_robocup_ssl_wrapper.proto\032$me"
      "ssages_robocup_ssl_detection.proto\032#mess"
      "ages_robocup_ssl_geometry.proto\"`\n\021SSL_W"
      "rapperPacket\022&\n\tdetection\030\001 \001(\0132\023.SSL_De"
      "tectionFrame\022#\n\010geometry\030\002 \001(\0132\021.SSL_Geo"
      "metryDatab\006proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 217);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "messages_robocup_ssl_wrapper.proto", &protobuf_RegisterTypes);
  ::protobuf_messages_5frobocup_5fssl_5fdetection_2eproto::AddDescriptors();
  ::protobuf_messages_5frobocup_5fssl_5fgeometry_2eproto::AddDescriptors();
  ::google::protobuf::internal::OnShutdown(&TableStruct::Shutdown);
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;

}  // namespace protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int SSL_WrapperPacket::kDetectionFieldNumber;
const int SSL_WrapperPacket::kGeometryFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

SSL_WrapperPacket::SSL_WrapperPacket()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:SSL_WrapperPacket)
}
SSL_WrapperPacket::SSL_WrapperPacket(const SSL_WrapperPacket& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  if (from.has_detection()) {
    detection_ = new ::SSL_DetectionFrame(*from.detection_);
  } else {
    detection_ = NULL;
  }
  if (from.has_geometry()) {
    geometry_ = new ::SSL_GeometryData(*from.geometry_);
  } else {
    geometry_ = NULL;
  }
  // @@protoc_insertion_point(copy_constructor:SSL_WrapperPacket)
}

void SSL_WrapperPacket::SharedCtor() {
  ::memset(&detection_, 0, reinterpret_cast<char*>(&geometry_) -
    reinterpret_cast<char*>(&detection_) + sizeof(geometry_));
  _cached_size_ = 0;
}

SSL_WrapperPacket::~SSL_WrapperPacket() {
  // @@protoc_insertion_point(destructor:SSL_WrapperPacket)
  SharedDtor();
}

void SSL_WrapperPacket::SharedDtor() {
  if (this != internal_default_instance()) {
    delete detection_;
  }
  if (this != internal_default_instance()) {
    delete geometry_;
  }
}

void SSL_WrapperPacket::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* SSL_WrapperPacket::descriptor() {
  protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const SSL_WrapperPacket& SSL_WrapperPacket::default_instance() {
  protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto::InitDefaults();
  return *internal_default_instance();
}

SSL_WrapperPacket* SSL_WrapperPacket::New(::google::protobuf::Arena* arena) const {
  SSL_WrapperPacket* n = new SSL_WrapperPacket;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void SSL_WrapperPacket::Clear() {
// @@protoc_insertion_point(message_clear_start:SSL_WrapperPacket)
  if (GetArenaNoVirtual() == NULL && detection_ != NULL) {
    delete detection_;
  }
  detection_ = NULL;
  if (GetArenaNoVirtual() == NULL && geometry_ != NULL) {
    delete geometry_;
  }
  geometry_ = NULL;
}

bool SSL_WrapperPacket::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:SSL_WrapperPacket)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // .SSL_DetectionFrame detection = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_detection()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // .SSL_GeometryData geometry = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(18u)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_geometry()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormatLite::SkipField(input, tag));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:SSL_WrapperPacket)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:SSL_WrapperPacket)
  return false;
#undef DO_
}

void SSL_WrapperPacket::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:SSL_WrapperPacket)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .SSL_DetectionFrame detection = 1;
  if (this->has_detection()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      1, *this->detection_, output);
  }

  // .SSL_GeometryData geometry = 2;
  if (this->has_geometry()) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      2, *this->geometry_, output);
  }

  // @@protoc_insertion_point(serialize_end:SSL_WrapperPacket)
}

::google::protobuf::uint8* SSL_WrapperPacket::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:SSL_WrapperPacket)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .SSL_DetectionFrame detection = 1;
  if (this->has_detection()) {
    target = ::google::protobuf::internal::WireFormatLite::
      InternalWriteMessageNoVirtualToArray(
        1, *this->detection_, deterministic, target);
  }

  // .SSL_GeometryData geometry = 2;
  if (this->has_geometry()) {
    target = ::google::protobuf::internal::WireFormatLite::
      InternalWriteMessageNoVirtualToArray(
        2, *this->geometry_, deterministic, target);
  }

  // @@protoc_insertion_point(serialize_to_array_end:SSL_WrapperPacket)
  return target;
}

size_t SSL_WrapperPacket::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:SSL_WrapperPacket)
  size_t total_size = 0;

  // .SSL_DetectionFrame detection = 1;
  if (this->has_detection()) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->detection_);
  }

  // .SSL_GeometryData geometry = 2;
  if (this->has_geometry()) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        *this->geometry_);
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void SSL_WrapperPacket::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:SSL_WrapperPacket)
  GOOGLE_DCHECK_NE(&from, this);
  const SSL_WrapperPacket* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const SSL_WrapperPacket>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:SSL_WrapperPacket)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:SSL_WrapperPacket)
    MergeFrom(*source);
  }
}

void SSL_WrapperPacket::MergeFrom(const SSL_WrapperPacket& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:SSL_WrapperPacket)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.has_detection()) {
    mutable_detection()->::SSL_DetectionFrame::MergeFrom(from.detection());
  }
  if (from.has_geometry()) {
    mutable_geometry()->::SSL_GeometryData::MergeFrom(from.geometry());
  }
}

void SSL_WrapperPacket::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:SSL_WrapperPacket)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void SSL_WrapperPacket::CopyFrom(const SSL_WrapperPacket& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:SSL_WrapperPacket)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool SSL_WrapperPacket::IsInitialized() const {
  return true;
}

void SSL_WrapperPacket::Swap(SSL_WrapperPacket* other) {
  if (other == this) return;
  InternalSwap(other);
}
void SSL_WrapperPacket::InternalSwap(SSL_WrapperPacket* other) {
  std::swap(detection_, other->detection_);
  std::swap(geometry_, other->geometry_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata SSL_WrapperPacket::GetMetadata() const {
  protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_messages_5frobocup_5fssl_5fwrapper_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// SSL_WrapperPacket

// .SSL_DetectionFrame detection = 1;
bool SSL_WrapperPacket::has_detection() const {
  return this != internal_default_instance() && detection_ != NULL;
}
void SSL_WrapperPacket::clear_detection() {
  if (GetArenaNoVirtual() == NULL && detection_ != NULL) delete detection_;
  detection_ = NULL;
}
const ::SSL_DetectionFrame& SSL_WrapperPacket::detection() const {
  // @@protoc_insertion_point(field_get:SSL_WrapperPacket.detection)
  return detection_ != NULL ? *detection_
                         : *::SSL_DetectionFrame::internal_default_instance();
}
::SSL_DetectionFrame* SSL_WrapperPacket::mutable_detection() {
  
  if (detection_ == NULL) {
    detection_ = new ::SSL_DetectionFrame;
  }
  // @@protoc_insertion_point(field_mutable:SSL_WrapperPacket.detection)
  return detection_;
}
::SSL_DetectionFrame* SSL_WrapperPacket::release_detection() {
  // @@protoc_insertion_point(field_release:SSL_WrapperPacket.detection)
  
  ::SSL_DetectionFrame* temp = detection_;
  detection_ = NULL;
  return temp;
}
void SSL_WrapperPacket::set_allocated_detection(::SSL_DetectionFrame* detection) {
  delete detection_;
  detection_ = detection;
  if (detection) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:SSL_WrapperPacket.detection)
}

// .SSL_GeometryData geometry = 2;
bool SSL_WrapperPacket::has_geometry() const {
  return this != internal_default_instance() && geometry_ != NULL;
}
void SSL_WrapperPacket::clear_geometry() {
  if (GetArenaNoVirtual() == NULL && geometry_ != NULL) delete geometry_;
  geometry_ = NULL;
}
const ::SSL_GeometryData& SSL_WrapperPacket::geometry() const {
  // @@protoc_insertion_point(field_get:SSL_WrapperPacket.geometry)
  return geometry_ != NULL ? *geometry_
                         : *::SSL_GeometryData::internal_default_instance();
}
::SSL_GeometryData* SSL_WrapperPacket::mutable_geometry() {
  
  if (geometry_ == NULL) {
    geometry_ = new ::SSL_GeometryData;
  }
  // @@protoc_insertion_point(field_mutable:SSL_WrapperPacket.geometry)
  return geometry_;
}
::SSL_GeometryData* SSL_WrapperPacket::release_geometry() {
  // @@protoc_insertion_point(field_release:SSL_WrapperPacket.geometry)
  
  ::SSL_GeometryData* temp = geometry_;
  geometry_ = NULL;
  return temp;
}
void SSL_WrapperPacket::set_allocated_geometry(::SSL_GeometryData* geometry) {
  delete geometry_;
  geometry_ = geometry;
  if (geometry) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:SSL_WrapperPacket.geometry)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)
