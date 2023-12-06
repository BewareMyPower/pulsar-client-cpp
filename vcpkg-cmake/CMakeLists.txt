#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

cmake_minimum_required(VERSION 3.13)

# TODO: https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration#vcpkg_manifest_features
set(ROOT_DIRECTORY "${CMAKE_SOURCE_DIR}/..")
set(VCPKG_MANIFEST_DIR ${ROOT_DIRECTORY})
set(CMAKE_TOOLCHAIN_FILE "${ROOT_DIRECTORY}/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(pulsar-cpp CXX)

if (NOT CMAKE_CXX_STANDARD)
    if (APPLE)
        set(CMAKE_CXX_STANDARD 17)
    else ()
        set(CMAKE_CXX_STANDARD 11)
    endif ()
endif ()

execute_process(COMMAND cat ${ROOT_DIRECTORY}/version.txt OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE PULSAR_CLIENT_VERSION)
message(STATUS "Pulsar Client version: ${PULSAR_CLIENT_VERSION}")

execute_process(COMMAND ${ROOT_DIRECTORY}/build-support/gen-pulsar-version-macro.py OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE PULSAR_CLIENT_VERSION_MACRO)
message(STATUS "Pulsar Client version macro: ${PULSAR_CLIENT_VERSION_MACRO}")

set(PVM_COMMENT "This is generated from Version.h.in by CMAKE. DO NOT EDIT DIRECTLY")
configure_file(${ROOT_DIRECTORY}/templates/Version.h.in include/pulsar/Version.h @ONLY)

option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()

option(BUILD_PERF_TOOLS "Build Pulsar CLI perf producer/consumer" OFF)
if (BUILD_PERF_TOOLS)
    list(APPEND VCPKG_MANIFEST_FEATURES "perf")
endif ()

find_package(protobuf CONFIG REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${ROOT_DIRECTORY}/proto/PulsarApi.proto)

# TODO: Use add_subdirectory() to use short file names
file(GLOB LIB_SOURCES ${ROOT_DIRECTORY}/lib/*.cc
    ${ROOT_DIRECTORY}/lib/c/*.cc
    ${ROOT_DIRECTORY}/lib/lz4/*.cc
    ${ROOT_DIRECTORY}/lib/checksum/*.cc
    ${ROOT_DIRECTORY}/lib/stats/*.cc
    ${ROOT_DIRECTORY}/lib/auth/*.cc
    ${ROOT_DIRECTORY}/lib/auth/athenz/*.cc
)

find_package(Boost REQUIRED)
# TODO: find program_options for perf
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Snappy CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)

add_library(pulsar ${LIB_SOURCES} ${PROTO_SRCS})
target_include_directories(pulsar PRIVATE
    ${ROOT_DIRECTORY}
    ${ROOT_DIRECTORY}/include
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include
    )
target_link_libraries(pulsar PRIVATE
    Boost::boost
    ZLIB::ZLIB
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    protobuf::libprotobuf-lite
    protobuf::libprotobuf
    Snappy::snappy
    zstd::libzstd_static
    )
