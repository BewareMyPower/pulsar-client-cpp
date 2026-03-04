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

#include <pulsar/Authentication.h>

#include <functional>
#include <memory>
#include <optional>

namespace pulsar {

class ClientImpl;

/**
 * Interface that allows dynamically providing the Pulsar service URL.
 */
class PULSAR_PUBLIC ServiceUrlProvider {
   public:
    /**
     * Initialize the provider with a reference to the client implementation. This method is called internally
     * by `Client` after creating the `ClientImpl` instance.
     */
    explicit ServiceUrlProvider(const std::shared_ptr<ClientImpl>& client) : client_(client) {}
    virtual ~ServiceUrlProvider() {}

    /**
     * Implementations are responsible to call `probe` periodically to check the validity of the service URL
     * and update the connection info accordingly via the methods whose name start with `update`.
     */
    virtual void start() = 0;

    /**
     * Close the provider and release resources.
     * This method should be implemented when the implementation starts a background thread to probe the
     * service URL, so that the background thread can be stopped and joined
     */
    virtual void close() {}

    // -----------------------------------------------------------------------------------------------
    // The following methods are implemented internally via `ClientImpl` and they are all thread-safe.

    /**
     * Probe the provided service URL to check if it's valid and reachable. The callback will be called with
     * `true` in `ClientImpl`'s internal I/O thread.
     */
    void probe(const std::string& serviceUrl, std::function<void(bool)>&& callback);

    /**
     * Get the current service URL.
     */
    std::string getServiceUrl() const;

    /**
     * Update the connection info.
     * When this method is called, all existing connections will be closed, then the client will start using
     * the new connection info for new operations and `getServiceUrl` will return the new URL.
     *
     * @param serviceUrl The new service URL to connect to.
     * @param authentication The optional new authentication information to use when connecting to the service
     * URL.
     * @param tlsTrustCertsFilePath The optional new TLS trust certificate file path to use when connecting to
     * the service URL.
     */
    void update(const std::string& serviceUrl, const std::optional<const AuthenticationPtr>& authentication,
                const std::optional<std::string>& tlsTrustCertsFilePath);

   private:
    const std::weak_ptr<ClientImpl> client_;
};

}  // namespace pulsar

#endif /* PULSAR_SERVICE_URL_PROVIDER_H_ */
