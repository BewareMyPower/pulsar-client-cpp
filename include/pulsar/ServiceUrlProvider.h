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
#ifndef PULSAR_SERVICE_URL_PROVIDER_H_
#define PULSAR_SERVICE_URL_PROVIDER_H_

#include <pulsar/defines.h>
#include <string>

namespace pulsar {

class Client;

/**
 * Interface that allows dynamically providing the Pulsar service URL.
 */
class PULSAR_PUBLIC ServiceUrlProvider {
   public:
    /**
     * Initialize the provider with a reference to the client implementation. This method is called internally
     * by `Client` after creating the `ClientImpl` instance.
     */
    virtual ~ServiceUrlProvider() {}

    /**
     * Implementations are responsible to detect some URLs and call `Client#updateConnectionInfo` to notify
     * the client that the service URL has changed.
     */
    virtual void initialize(Client& client) = 0;

    /**
     * Get the current service URL.
     * The implementation should return the latest service URL that has been passed to
     * `Client#updateConnectionInfo`. The client will call this method to get the initial service URL when
     * starting the connection.
     */
    virtual std::string getServiceUrl() const = 0;
};

}  // namespace pulsar

#endif /* PULSAR_SERVICE_URL_PROVIDER_H_ */
