cmake_minimum_required(VERSION 3.1)

project(Li LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
#add_definitions(-DZSPLUGIN_DEBUG)
set(libs)
# define protobuf generater
find_package(Protobuf REQUIRED)
function(get_pb_file H CPP)
   list(APPEND PH ${ARGN})
   list(APPEND PC ${ARGN})
   list(TRANSFORM PH PREPEND ${CMAKE_BINARY_DIR}/proto/)
   list(TRANSFORM PH APPEND .pb.h)
   list(TRANSFORM PC PREPEND ${CMAKE_BINARY_DIR}/proto/)
   list(TRANSFORM PC APPEND .pb.cc)
   set(${H} ${PH} PARENT_SCOPE)
   set(${CPP} ${PC} PARENT_SCOPE)
endfunction()
SET(PROTO_META_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
file(MAKE_DIRECTORY ${PROTO_META_BASE_DIR})

LIST(APPEND PROTO_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/proto)
file(GLOB PROTO_FILES "${CMAKE_SOURCE_DIR}/proto/*.proto")
FOREACH(FIL ${PROTO_FILES})
   GET_FILENAME_COMPONENT(ABS_FIL ${FIL} ABSOLUTE)
   GET_FILENAME_COMPONENT(FIL_WE ${ABS_FIL} NAME_WE)
   EXECUTE_PROCESS(
       COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FLAGS} --cpp_out=${PROTO_META_BASE_DIR} ${FIL}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   )
ENDFOREACH()

find_package(Eigen3 REQUIRED)
find_package(ZLIB REQUIRED)

set(PROTO_LIST
   log_labeler_data
   log_labels
   messages_robocup_ssl_detection
   messages_robocup_ssl_geometry_legacy
   messages_robocup_ssl_geometry
   messages_robocup_ssl_refbox_log
   messages_robocup_ssl_wrapper_legacy
   messages_robocup_ssl_wrapper
   ssl_game_controller_auto_ref
   ssl_game_controller_common
   ssl_game_controller_team
   ssl_game_event_2019
   ssl_game_event
   ssl_referee
   vision_detection
   zss_cmd
   zss_debug
   zss_rec
)
get_pb_file(PROTO_H PROTO_CPP ${PROTO_LIST})

find_package(Qt5 COMPONENTS Core Quick Gui Network QuickControls2 REQUIRED)

set(PROJECT_INCLUDE_DIRS
    ${CMAKE_BINARY_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/vision_gd/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/udp_sender/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/udp_receiver/include
)

include_directories(${PROJECT_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIRS})
list(APPEND libs ${ZLIB_LIBRARIES})
list(APPEND libs ${Protobuf_LIBRARIES})
list(APPEND libs Qt5::Core Qt5::Quick Qt5::Gui Qt5::Network Qt5::QuickControls2)

file(GLOB CPPFILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/vision_gd/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/udp_sender/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/plugins/udp_receiver/src/*.cpp"
)
set(RESOURCES
    Li.qrc
)
set(SOURCE
    ${CPPFILES}
    ${RESOURCES}
    ${PROTO_CPP}
    ${PROTO_H}
)
add_executable(${PROJECT_NAME} ${SOURCE})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} PRIVATE ${libs})
