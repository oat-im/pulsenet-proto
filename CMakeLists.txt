cmake_minimum_required(VERSION 3.15)
project(pulsenet_proto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  pulsenet_udp
  GIT_REPOSITORY https://git.pulsenet.dev/pulse/udp
  GIT_TAG        main
)

FetchContent_MakeAvailable(pulsenet_udp)

# Standalone mode toggle
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(PULSENET_PROTO_STANDALONE_BUILD ON)
else()
    set(PULSENET_PROTO_STANDALONE_BUILD OFF)
endif()

include(GNUInstallDirs)

# Source and headers
file(GLOB_RECURSE PULSENET_PROTO_HEADERS CONFIGURE_DEPENDS include/pulse/net/proto/*.h)
file(GLOB_RECURSE PULSENET_PROTO_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(pulsenet_proto STATIC ${PULSENET_PROTO_SOURCES} ${PULSENET_PROTO_HEADERS})
add_library(pulsenet::proto ALIAS pulsenet_proto)

target_include_directories(pulsenet_proto PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(pulsenet_proto PUBLIC pulsenet::udp)

# Install logic
include(CMakePackageConfigHelpers)

install(TARGETS pulsenet_proto
    EXPORT pulsenet_protoTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT pulsenet_protoTargets
    FILE pulsenet_protoTargets.cmake
    NAMESPACE pulsenet::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pulsenet_proto
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/pulsenet_protoConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/pulsenet_protoConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/pulsenet_protoConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pulsenet_proto
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/pulsenet_protoConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/pulsenet_protoConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pulsenet_proto
)

# Tests (standalone only)
if (PULSENET_PROTO_STANDALONE_BUILD)
    add_executable(pulsenet_proto_test tests/IntegrationTest.cpp)
    target_link_libraries(pulsenet_proto_test PRIVATE pulsenet::proto)
    install(TARGETS pulsenet_proto_test RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
