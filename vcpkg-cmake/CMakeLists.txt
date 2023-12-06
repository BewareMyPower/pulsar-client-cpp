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
