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

project (pulsar-cpp)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../cmake_modules")

option(LINK_STATIC "Link against static libraries" OFF)
if (VCPKG_TRIPLET)
    message(STATUS "Use vcpkg, triplet is ${VCPKG_TRIPLET}")
    set(CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TRIPLET}")
    message(STATUS "Use CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    set(VCPKG_DEBUG_ROOT "${PROJECT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TRIPLET}/debug")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OPENSSL_ROOT_DIR ${VCPKG_DEBUG_ROOT})
        set(CMAKE_PREFIX_PATH ${VCPKG_DEBUG_ROOT} ${CMAKE_PREFIX_PATH})
    else ()
        set(OPENSSL_ROOT_DIR "${PROJECT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TRIPLET}")
    endif ()
    if (VCPKG_TRIPLET MATCHES ".*-static")
        set(LINK_STATIC ON)
        set(OPENSSL_USE_STATIC_LIBS TRUE)
        set(OPENSSL_MSVC_STATIC_RT TRUE)
    else ()
        set(LINK_STATIC OFF)
    endif ()
endif()
MESSAGE(STATUS "LINK_STATIC:  " ${LINK_STATIC})

IF (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF ()

find_package(OpenSSL REQUIRED)
message("OPENSSL_INCLUDE_DIR: " ${OPENSSL_INCLUDE_DIR})
message("OPENSSL_LIBRARIES: " ${OPENSSL_LIBRARIES})
