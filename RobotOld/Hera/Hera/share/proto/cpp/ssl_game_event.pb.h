// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ssl_game_event.proto

#ifndef PROTOBUF_ssl_5fgame_5fevent_2eproto__INCLUDED
#define PROTOBUF_ssl_5fgame_5fevent_2eproto__INCLUDED

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
class Game_Event;
class Game_EventDefaultTypeInternal;
extern Game_EventDefaultTypeInternal _Game_Event_default_instance_;
class Game_Event_Originator;
class Game_Event_OriginatorDefaultTypeInternal;
extern Game_Event_OriginatorDefaultTypeInternal _Game_Event_Originator_default_instance_;

namespace protobuf_ssl_5fgame_5fevent_2eproto {
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
}  // namespace protobuf_ssl_5fgame_5fevent_2eproto

enum Game_Event_GameEventType {
  Game_Event_GameEventType_UNKNOWN = 0,
  Game_Event_GameEventType_CUSTOM = 1,
  Game_Event_GameEventType_NUMBER_OF_PLAYERS = 2,
  Game_Event_GameEventType_BALL_LEFT_FIELD = 3,
  Game_Event_GameEventType_GOAL = 4,
  Game_Event_GameEventType_KICK_TIMEOUT = 5,
  Game_Event_GameEventType_NO_PROGRESS_IN_GAME = 6,
  Game_Event_GameEventType_BOT_COLLISION = 7,
  Game_Event_GameEventType_MULTIPLE_DEFENDER = 8,
  Game_Event_GameEventType_MULTIPLE_DEFENDER_PARTIALLY = 9,
  Game_Event_GameEventType_ATTACKER_IN_DEFENSE_AREA = 10,
  Game_Event_GameEventType_ICING = 11,
  Game_Event_GameEventType_BALL_SPEED = 12,
  Game_Event_GameEventType_ROBOT_STOP_SPEED = 13,
  Game_Event_GameEventType_BALL_DRIBBLING = 14,
  Game_Event_GameEventType_ATTACKER_TOUCH_KEEPER = 15,
  Game_Event_GameEventType_DOUBLE_TOUCH = 16,
  Game_Event_GameEventType_ATTACKER_TO_DEFENCE_AREA = 17,
  Game_Event_GameEventType_DEFENDER_TO_KICK_POINT_DISTANCE = 18,
  Game_Event_GameEventType_BALL_HOLDING = 19,
  Game_Event_GameEventType_INDIRECT_GOAL = 20,
  Game_Event_GameEventType_BALL_PLACEMENT_FAILED = 21,
  Game_Event_GameEventType_CHIP_ON_GOAL = 22
};
bool Game_Event_GameEventType_IsValid(int value);
const Game_Event_GameEventType Game_Event_GameEventType_GameEventType_MIN = Game_Event_GameEventType_UNKNOWN;
const Game_Event_GameEventType Game_Event_GameEventType_GameEventType_MAX = Game_Event_GameEventType_CHIP_ON_GOAL;
const int Game_Event_GameEventType_GameEventType_ARRAYSIZE = Game_Event_GameEventType_GameEventType_MAX + 1;

const ::google::protobuf::EnumDescriptor* Game_Event_GameEventType_descriptor();
inline const ::std::string& Game_Event_GameEventType_Name(Game_Event_GameEventType value) {
  return ::google::protobuf::internal::NameOfEnum(
    Game_Event_GameEventType_descriptor(), value);
}
inline bool Game_Event_GameEventType_Parse(
    const ::std::string& name, Game_Event_GameEventType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Game_Event_GameEventType>(
    Game_Event_GameEventType_descriptor(), name, value);
}
enum Game_Event_Team {
  Game_Event_Team_TEAM_UNKNOWN = 0,
  Game_Event_Team_TEAM_YELLOW = 1,
  Game_Event_Team_TEAM_BLUE = 2
};
bool Game_Event_Team_IsValid(int value);
const Game_Event_Team Game_Event_Team_Team_MIN = Game_Event_Team_TEAM_UNKNOWN;
const Game_Event_Team Game_Event_Team_Team_MAX = Game_Event_Team_TEAM_BLUE;
const int Game_Event_Team_Team_ARRAYSIZE = Game_Event_Team_Team_MAX + 1;

const ::google::protobuf::EnumDescriptor* Game_Event_Team_descriptor();
inline const ::std::string& Game_Event_Team_Name(Game_Event_Team value) {
  return ::google::protobuf::internal::NameOfEnum(
    Game_Event_Team_descriptor(), value);
}
inline bool Game_Event_Team_Parse(
    const ::std::string& name, Game_Event_Team* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Game_Event_Team>(
    Game_Event_Team_descriptor(), name, value);
}
// ===================================================================

class Game_Event_Originator : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Game_Event.Originator) */ {
 public:
  Game_Event_Originator();
  virtual ~Game_Event_Originator();

  Game_Event_Originator(const Game_Event_Originator& from);

  inline Game_Event_Originator& operator=(const Game_Event_Originator& from) {
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
  static const Game_Event_Originator& default_instance();

  static inline const Game_Event_Originator* internal_default_instance() {
    return reinterpret_cast<const Game_Event_Originator*>(
               &_Game_Event_Originator_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(Game_Event_Originator* other);

  // implements Message ----------------------------------------------

  inline Game_Event_Originator* New() const PROTOBUF_FINAL { return New(NULL); }

  Game_Event_Originator* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Game_Event_Originator& from);
  void MergeFrom(const Game_Event_Originator& from);
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
  void InternalSwap(Game_Event_Originator* other);
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

  // required .Game_Event.Team team = 1;
  bool has_team() const;
  void clear_team();
  static const int kTeamFieldNumber = 1;
  ::Game_Event_Team team() const;
  void set_team(::Game_Event_Team value);

  // optional uint32 botId = 2;
  bool has_botid() const;
  void clear_botid();
  static const int kBotIdFieldNumber = 2;
  ::google::protobuf::uint32 botid() const;
  void set_botid(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:Game_Event.Originator)
 private:
  void set_has_team();
  void clear_has_team();
  void set_has_botid();
  void clear_has_botid();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  int team_;
  ::google::protobuf::uint32 botid_;
  friend struct protobuf_ssl_5fgame_5fevent_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Game_Event : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Game_Event) */ {
 public:
  Game_Event();
  virtual ~Game_Event();

  Game_Event(const Game_Event& from);

  inline Game_Event& operator=(const Game_Event& from) {
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
  static const Game_Event& default_instance();

  static inline const Game_Event* internal_default_instance() {
    return reinterpret_cast<const Game_Event*>(
               &_Game_Event_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(Game_Event* other);

  // implements Message ----------------------------------------------

  inline Game_Event* New() const PROTOBUF_FINAL { return New(NULL); }

  Game_Event* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Game_Event& from);
  void MergeFrom(const Game_Event& from);
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
  void InternalSwap(Game_Event* other);
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

  typedef Game_Event_Originator Originator;

  typedef Game_Event_GameEventType GameEventType;
  static const GameEventType UNKNOWN =
    Game_Event_GameEventType_UNKNOWN;
  static const GameEventType CUSTOM =
    Game_Event_GameEventType_CUSTOM;
  static const GameEventType NUMBER_OF_PLAYERS =
    Game_Event_GameEventType_NUMBER_OF_PLAYERS;
  static const GameEventType BALL_LEFT_FIELD =
    Game_Event_GameEventType_BALL_LEFT_FIELD;
  static const GameEventType GOAL =
    Game_Event_GameEventType_GOAL;
  static const GameEventType KICK_TIMEOUT =
    Game_Event_GameEventType_KICK_TIMEOUT;
  static const GameEventType NO_PROGRESS_IN_GAME =
    Game_Event_GameEventType_NO_PROGRESS_IN_GAME;
  static const GameEventType BOT_COLLISION =
    Game_Event_GameEventType_BOT_COLLISION;
  static const GameEventType MULTIPLE_DEFENDER =
    Game_Event_GameEventType_MULTIPLE_DEFENDER;
  static const GameEventType MULTIPLE_DEFENDER_PARTIALLY =
    Game_Event_GameEventType_MULTIPLE_DEFENDER_PARTIALLY;
  static const GameEventType ATTACKER_IN_DEFENSE_AREA =
    Game_Event_GameEventType_ATTACKER_IN_DEFENSE_AREA;
  static const GameEventType ICING =
    Game_Event_GameEventType_ICING;
  static const GameEventType BALL_SPEED =
    Game_Event_GameEventType_BALL_SPEED;
  static const GameEventType ROBOT_STOP_SPEED =
    Game_Event_GameEventType_ROBOT_STOP_SPEED;
  static const GameEventType BALL_DRIBBLING =
    Game_Event_GameEventType_BALL_DRIBBLING;
  static const GameEventType ATTACKER_TOUCH_KEEPER =
    Game_Event_GameEventType_ATTACKER_TOUCH_KEEPER;
  static const GameEventType DOUBLE_TOUCH =
    Game_Event_GameEventType_DOUBLE_TOUCH;
  static const GameEventType ATTACKER_TO_DEFENCE_AREA =
    Game_Event_GameEventType_ATTACKER_TO_DEFENCE_AREA;
  static const GameEventType DEFENDER_TO_KICK_POINT_DISTANCE =
    Game_Event_GameEventType_DEFENDER_TO_KICK_POINT_DISTANCE;
  static const GameEventType BALL_HOLDING =
    Game_Event_GameEventType_BALL_HOLDING;
  static const GameEventType INDIRECT_GOAL =
    Game_Event_GameEventType_INDIRECT_GOAL;
  static const GameEventType BALL_PLACEMENT_FAILED =
    Game_Event_GameEventType_BALL_PLACEMENT_FAILED;
  static const GameEventType CHIP_ON_GOAL =
    Game_Event_GameEventType_CHIP_ON_GOAL;
  static inline bool GameEventType_IsValid(int value) {
    return Game_Event_GameEventType_IsValid(value);
  }
  static const GameEventType GameEventType_MIN =
    Game_Event_GameEventType_GameEventType_MIN;
  static const GameEventType GameEventType_MAX =
    Game_Event_GameEventType_GameEventType_MAX;
  static const int GameEventType_ARRAYSIZE =
    Game_Event_GameEventType_GameEventType_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  GameEventType_descriptor() {
    return Game_Event_GameEventType_descriptor();
  }
  static inline const ::std::string& GameEventType_Name(GameEventType value) {
    return Game_Event_GameEventType_Name(value);
  }
  static inline bool GameEventType_Parse(const ::std::string& name,
      GameEventType* value) {
    return Game_Event_GameEventType_Parse(name, value);
  }

  typedef Game_Event_Team Team;
  static const Team TEAM_UNKNOWN =
    Game_Event_Team_TEAM_UNKNOWN;
  static const Team TEAM_YELLOW =
    Game_Event_Team_TEAM_YELLOW;
  static const Team TEAM_BLUE =
    Game_Event_Team_TEAM_BLUE;
  static inline bool Team_IsValid(int value) {
    return Game_Event_Team_IsValid(value);
  }
  static const Team Team_MIN =
    Game_Event_Team_Team_MIN;
  static const Team Team_MAX =
    Game_Event_Team_Team_MAX;
  static const int Team_ARRAYSIZE =
    Game_Event_Team_Team_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  Team_descriptor() {
    return Game_Event_Team_descriptor();
  }
  static inline const ::std::string& Team_Name(Team value) {
    return Game_Event_Team_Name(value);
  }
  static inline bool Team_Parse(const ::std::string& name,
      Team* value) {
    return Game_Event_Team_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // optional string message = 3;
  bool has_message() const;
  void clear_message();
  static const int kMessageFieldNumber = 3;
  const ::std::string& message() const;
  void set_message(const ::std::string& value);
  #if LANG_CXX11
  void set_message(::std::string&& value);
  #endif
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  ::std::string* mutable_message();
  ::std::string* release_message();
  void set_allocated_message(::std::string* message);

  // optional .Game_Event.Originator originator = 2;
  bool has_originator() const;
  void clear_originator();
  static const int kOriginatorFieldNumber = 2;
  const ::Game_Event_Originator& originator() const;
  ::Game_Event_Originator* mutable_originator();
  ::Game_Event_Originator* release_originator();
  void set_allocated_originator(::Game_Event_Originator* originator);

  // required .Game_Event.GameEventType gameEventType = 1;
  bool has_gameeventtype() const;
  void clear_gameeventtype();
  static const int kGameEventTypeFieldNumber = 1;
  ::Game_Event_GameEventType gameeventtype() const;
  void set_gameeventtype(::Game_Event_GameEventType value);

  // @@protoc_insertion_point(class_scope:Game_Event)
 private:
  void set_has_gameeventtype();
  void clear_has_gameeventtype();
  void set_has_originator();
  void clear_has_originator();
  void set_has_message();
  void clear_has_message();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr message_;
  ::Game_Event_Originator* originator_;
  int gameeventtype_;
  friend struct protobuf_ssl_5fgame_5fevent_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Game_Event_Originator

// required .Game_Event.Team team = 1;
inline bool Game_Event_Originator::has_team() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Game_Event_Originator::set_has_team() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Game_Event_Originator::clear_has_team() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Game_Event_Originator::clear_team() {
  team_ = 0;
  clear_has_team();
}
inline ::Game_Event_Team Game_Event_Originator::team() const {
  // @@protoc_insertion_point(field_get:Game_Event.Originator.team)
  return static_cast< ::Game_Event_Team >(team_);
}
inline void Game_Event_Originator::set_team(::Game_Event_Team value) {
  assert(::Game_Event_Team_IsValid(value));
  set_has_team();
  team_ = value;
  // @@protoc_insertion_point(field_set:Game_Event.Originator.team)
}

// optional uint32 botId = 2;
inline bool Game_Event_Originator::has_botid() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Game_Event_Originator::set_has_botid() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Game_Event_Originator::clear_has_botid() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Game_Event_Originator::clear_botid() {
  botid_ = 0u;
  clear_has_botid();
}
inline ::google::protobuf::uint32 Game_Event_Originator::botid() const {
  // @@protoc_insertion_point(field_get:Game_Event.Originator.botId)
  return botid_;
}
inline void Game_Event_Originator::set_botid(::google::protobuf::uint32 value) {
  set_has_botid();
  botid_ = value;
  // @@protoc_insertion_point(field_set:Game_Event.Originator.botId)
}

// -------------------------------------------------------------------

// Game_Event

// required .Game_Event.GameEventType gameEventType = 1;
inline bool Game_Event::has_gameeventtype() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void Game_Event::set_has_gameeventtype() {
  _has_bits_[0] |= 0x00000004u;
}
inline void Game_Event::clear_has_gameeventtype() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void Game_Event::clear_gameeventtype() {
  gameeventtype_ = 0;
  clear_has_gameeventtype();
}
inline ::Game_Event_GameEventType Game_Event::gameeventtype() const {
  // @@protoc_insertion_point(field_get:Game_Event.gameEventType)
  return static_cast< ::Game_Event_GameEventType >(gameeventtype_);
}
inline void Game_Event::set_gameeventtype(::Game_Event_GameEventType value) {
  assert(::Game_Event_GameEventType_IsValid(value));
  set_has_gameeventtype();
  gameeventtype_ = value;
  // @@protoc_insertion_point(field_set:Game_Event.gameEventType)
}

// optional .Game_Event.Originator originator = 2;
inline bool Game_Event::has_originator() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Game_Event::set_has_originator() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Game_Event::clear_has_originator() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Game_Event::clear_originator() {
  if (originator_ != NULL) originator_->::Game_Event_Originator::Clear();
  clear_has_originator();
}
inline const ::Game_Event_Originator& Game_Event::originator() const {
  // @@protoc_insertion_point(field_get:Game_Event.originator)
  return originator_ != NULL ? *originator_
                         : *::Game_Event_Originator::internal_default_instance();
}
inline ::Game_Event_Originator* Game_Event::mutable_originator() {
  set_has_originator();
  if (originator_ == NULL) {
    originator_ = new ::Game_Event_Originator;
  }
  // @@protoc_insertion_point(field_mutable:Game_Event.originator)
  return originator_;
}
inline ::Game_Event_Originator* Game_Event::release_originator() {
  // @@protoc_insertion_point(field_release:Game_Event.originator)
  clear_has_originator();
  ::Game_Event_Originator* temp = originator_;
  originator_ = NULL;
  return temp;
}
inline void Game_Event::set_allocated_originator(::Game_Event_Originator* originator) {
  delete originator_;
  originator_ = originator;
  if (originator) {
    set_has_originator();
  } else {
    clear_has_originator();
  }
  // @@protoc_insertion_point(field_set_allocated:Game_Event.originator)
}

// optional string message = 3;
inline bool Game_Event::has_message() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Game_Event::set_has_message() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Game_Event::clear_has_message() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Game_Event::clear_message() {
  message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_message();
}
inline const ::std::string& Game_Event::message() const {
  // @@protoc_insertion_point(field_get:Game_Event.message)
  return message_.GetNoArena();
}
inline void Game_Event::set_message(const ::std::string& value) {
  set_has_message();
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Game_Event.message)
}
#if LANG_CXX11
inline void Game_Event::set_message(::std::string&& value) {
  set_has_message();
  message_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Game_Event.message)
}
#endif
inline void Game_Event::set_message(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_message();
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Game_Event.message)
}
inline void Game_Event::set_message(const char* value, size_t size) {
  set_has_message();
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Game_Event.message)
}
inline ::std::string* Game_Event::mutable_message() {
  set_has_message();
  // @@protoc_insertion_point(field_mutable:Game_Event.message)
  return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Game_Event::release_message() {
  // @@protoc_insertion_point(field_release:Game_Event.message)
  clear_has_message();
  return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Game_Event::set_allocated_message(::std::string* message) {
  if (message != NULL) {
    set_has_message();
  } else {
    clear_has_message();
  }
  message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
  // @@protoc_insertion_point(field_set_allocated:Game_Event.message)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::Game_Event_GameEventType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Game_Event_GameEventType>() {
  return ::Game_Event_GameEventType_descriptor();
}
template <> struct is_proto_enum< ::Game_Event_Team> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Game_Event_Team>() {
  return ::Game_Event_Team_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_ssl_5fgame_5fevent_2eproto__INCLUDED
