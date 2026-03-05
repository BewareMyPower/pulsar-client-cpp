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
#include <gtest/gtest.h>
#include <pulsar/Client.h>
#include <pulsar/ServiceUrlProvider.h>

#include <chrono>
#include <future>
#include <mutex>
#include <optional>
#include <thread>

#include "tests/WaitUtils.h"

using namespace pulsar;

class ExampleServiceUrlProvider : public ServiceUrlProvider {
   public:
    ExampleServiceUrlProvider(const std::string& primary, const std::string& secondary)
        : url_(primary), primaryServiceUrl_(primary), secondaryServiceUrl_(secondary) {}

    ~ExampleServiceUrlProvider() override {
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

    std::string getServiceUrl() const override {
        std::lock_guard<std::mutex> lock{mutex_};
        return url_;
    }

    void initialize(Client& client) override {
        workerThread_ = std::thread([this, &client]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::promise<bool> promise;
            client.probe(primaryServiceUrl_, [&promise](bool success) { promise.set_value(success); });
            auto available = promise.get_future().get();
            if (!available) {
                std::lock_guard<std::mutex> lock{mutex_};
                client.updateConnectionInfo(secondaryServiceUrl_, std::nullopt, std::nullopt);
                url_ = secondaryServiceUrl_;
            }
        });
    }

   private:
    mutable std::mutex mutex_;
    std::string url_;

    const std::string primaryServiceUrl_;
    const std::string secondaryServiceUrl_;
    std::thread workerThread_;
};

TEST(ServiceUrlProviderTest, testUpdateUrl) {
    auto provider =
        std::make_shared<ExampleServiceUrlProvider>("pulsar://localhost:1111", "pulsar://localhost:6652");
    Client client{"pulsar://localhost:6650"};
    provider->initialize(client);

    ASSERT_EQ("pulsar://localhost:1111", provider->getServiceUrl());
    ASSERT_TRUE(waitUntil(std::chrono::seconds(5),
                          [provider]() { return provider->getServiceUrl() == "pulsar://localhost:6652"; }));
    ASSERT_EQ("pulsar://localhost:6652", provider->getServiceUrl());

    provider.reset();  // in case any failure, we should not reference the client after it's closed
    client.close();
}
