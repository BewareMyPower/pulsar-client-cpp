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
#include <pulsar/ServiceUrlProvider.h>

#include "ClientImpl.h"
#include "LookupService.h"

namespace pulsar {

void ServiceUrlProvider::probe(const std::string& serviceUrl, std::function<void(bool)>&& callback) {
    // TODO:
}

std::string ServiceUrlProvider::getServiceUrl() const {
    if (auto client = client_.lock()) {
        return client->getLookup("")->getServiceNameResolver().serviceUri().uriString();
    } else {
        return {};
    }
}

void ServiceUrlProvider::update(const std::string& serviceUrl,
                                const std::optional<const AuthenticationPtr>& authentication,
                                const std::optional<std::string>& tlsTrustCertsFilePath) {
    if (auto client = client_.lock()) {
        client->updateConnectionInfo(serviceUrl, authentication, tlsTrustCertsFilePath);
    }
}

// TODO: implement some methods
}  // namespace pulsar
