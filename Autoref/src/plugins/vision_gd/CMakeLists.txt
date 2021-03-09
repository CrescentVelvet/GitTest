# Vision
set(libs)
# Qt5
find_package(Qt5 COMPONENTS Core REQUIRED)
list(APPEND libs Qt5::Core)

# Protobuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
list(APPEND libs ${Protobuf_LIBRARIES})

# Eigen
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

set(PROTO_LIST
#     game_event
#     grSim_Commands
#     grSimMessage
#     grSim_Packet
#     grSim_Replacement
    messages_robocup_ssl_detection
    messages_robocup_ssl_geometry_legacy
    messages_robocup_ssl_geometry
#     messages_robocup_ssl_refbox_log
    messages_robocup_ssl_wrapper_legacy
    messages_robocup_ssl_wrapper
#     referee
    vision_detection
#     zss_cmd
#     zss_debug
)
get_pb_file(PROTO_H PROTO_CPP ${PROTO_LIST})


set(PROJECT_INCLUDE_DIRS
    ${CMAKE_BINARY_DIR}/proto
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/utils
)
include_directories(${PROJECT_INCLUDE_DIRS})
set(EXPORT_HEADERS
    include/vision.h
)
set(EXPORT_SOURCES
    src/vision.cpp
)
file(GLOB HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
file(GLOB CPPFILES
    "${CMAKE_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
set(SRC
    ${HEADERS}
    ${SOURCES}
    ${PROTO_CPP}
    ${PROTO_H}
    ${CPPFILES}
)

set(target_lib zss_vision_gd)

add_library(${target_lib} SHARED ${SRC})

set_target_properties(${target_lib} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(${target_lib} PUBLIC ${PROJECT_INCLUDE_DIRS})

target_link_libraries(${target_lib} ${libs})

install(TARGETS ${target_lib}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
