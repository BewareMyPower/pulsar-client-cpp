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
#ifndef LIB_FUTURE_H_
#define LIB_FUTURE_H_

#include <atomic>
#include <condition_variable>
#include <forward_list>
#include <functional>
#include <memory>
#include <mutex>

#include "LogUtils.h"

namespace pulsar {

enum Status : uint8_t
{
    INITIAL,
    COMPLETING,
    COMPLETED
};

inline std::ostream &operator<<(std::ostream &os, Status status) {
    switch (status) {
        case INITIAL:
            os << "INITIAL";
            break;
        case COMPLETING:
            os << "COMPLETING";
            break;
        case COMPLETED:
            os << "COMPLETED";
            break;
        default:
            os << "???";
    }
    return os;
}

template <typename Result, typename Type>
class InternalState {
    DECLARE_LOG_OBJECT()
   public:
    using Listener = std::function<void(Result, const Type &)>;
    using Pair = std::pair<Result, Type>;
    using Lock = std::unique_lock<std::mutex>;

    // NOTE: Add the constructor explicitly just to be compatible with GCC 4.8
    InternalState() {}

    void addListener(Listener listener) {
        Lock lock{mutex_};
        if (completed()) {
            auto result = result_;
            auto value = value_;
            lock.unlock();
            LOG_INFO(this << " XYZ trigger listener directly " << result);
            listener(result, value);
        } else {
            LOG_INFO(this << " XYZ before inserting listener");
            tailListener_ = listeners_.emplace_after(tailListener_, std::move(listener));
            LOG_INFO(this << " XYZ after inserting listener");
        }
    }

    bool complete(Result result, const Type &value) {
        Status expected = Status::INITIAL;
        if (!status_.compare_exchange_strong(expected, Status::COMPLETING)) {
            LOG_INFO(this << " XYZ repeat complete, expected: " << expected << ", status_" << status_.load());
            return false;
        }
        LOG_INFO(this << " XYZ repeat complete, expected: " << expected << ", status_" << status_.load());

        // Ensure if another thread calls `addListener` at the same time, that thread can get the value by
        // `get` before the existing listeners are executed
        LOG_INFO(this << " XYZ before complete");
        Lock lock{mutex_};
        result_ = result;
        value_ = value;
        status_ = COMPLETED;
        cond_.notify_all();
        LOG_INFO(this << " XYZ after complete");

        if (!listeners_.empty()) {
            auto listeners = std::move(listeners_);
            lock.unlock();
            for (auto &&listener : listeners) {
                LOG_INFO(this << " XYZ trigger listener");
                listener(result, value);
            }
        } else {
            LOG_INFO(this << " XYZ no listener");
        }

        return true;
    }

    bool completed() const noexcept { return status_.load() == COMPLETED; }

    Result get(Type &value) const {
        Lock lock{mutex_};
        cond_.wait(lock, [this] { return completed(); });
        value = value_;
        LOG_INFO(this << " XYZ get " << result_);
        return result_;
    }

   private:
    mutable std::mutex mutex_;
    mutable std::condition_variable cond_;
    std::forward_list<Listener> listeners_;
    decltype(listeners_.before_begin()) tailListener_{listeners_.before_begin()};
    Result result_;
    Type value_;
    std::atomic<Status> status_;
};

template <typename Result, typename Type>
using InternalStatePtr = std::shared_ptr<InternalState<Result, Type>>;

template <typename Result, typename Type>
class Future {
   public:
    using Listener = typename InternalState<Result, Type>::Listener;

    Future &addListener(Listener listener) {
        state_->addListener(listener);
        return *this;
    }

    Result get(Type &result) { return state_->get(result); }

   private:
    InternalStatePtr<Result, Type> state_;

    Future(InternalStatePtr<Result, Type> state) : state_(state) {}

    template <typename U, typename V>
    friend class Promise;
};

template <typename Result, typename Type>
class Promise {
   public:
    Promise() : state_(std::make_shared<InternalState<Result, Type>>()) {}

    bool setValue(const Type &value) const { return state_->complete({}, value); }

    bool setFailed(Result result) const { return state_->complete(result, {}); }

    bool setSuccess() const { return setValue({}); }

    bool isComplete() const { return state_->completed(); }

    Future<Result, Type> getFuture() const { return Future<Result, Type>{state_}; }

   private:
    const InternalStatePtr<Result, Type> state_;
};

}  // namespace pulsar

#endif
