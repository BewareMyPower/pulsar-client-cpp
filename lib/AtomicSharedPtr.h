/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include <atomic>
#include <memory>

namespace pulsar {

// C++17 does not support std::atomic<std::shared_ptr<T>>, so we need to implement our own atomic shared
// pointer.
template <typename T>
class AtomicSharedPtr {
   public:
    using SharedPtr = std::shared_ptr<T>;

    auto get() const noexcept { return std::atomic_load(&ptr_); }

    auto operator->() const noexcept { return get(); }

    auto swap(SharedPtr& newPtr) noexcept { return std::atomic_exchange(&ptr_, newPtr); }

   private:
    SharedPtr ptr_;
};

}  // namespace pulsar
