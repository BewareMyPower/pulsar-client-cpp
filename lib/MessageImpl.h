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
#ifndef LIB_MESSAGEIMPL_H_
#define LIB_MESSAGEIMPL_H_

#include <pulsar/Message.h>
#include <pulsar/MessageId.h>

#include <memory>

#include "KeyValueImpl.h"
#include "PulsarApi.pb.h"
#include "SharedBuffer.h"

using namespace pulsar;
namespace pulsar {

class PulsarWrapper;
class ClientConnection;
class ConsumerImpl;
class BatchMessageContainer;

class MessageImpl {
   public:
    MessageImpl() = default;

    MessageImpl(proto::MessageMetadata&& metadata, SharedBuffer&& payload, int64_t index,
                const MessageId& messageId, const std::shared_ptr<std::string>& topic, ClientConnection* cnx)
        : metadata_(std::move(metadata)), payload_(std::move(payload)), context_(new ConsumerContext) {
        context_->index = index;
        context_->messageId = messageId;
        context_->topic = topic;
        context_->cnx = cnx;
    }

    proto::MessageMetadata& metadata() noexcept { return metadata_; }
    SharedBuffer& payload() noexcept { return payload_; }
    std::shared_ptr<KeyValueImpl> keyValuePtr() noexcept { return keyValuePtr_; }

    int64_t index() const noexcept { return context_ ? context_->index : -1; }

    void updateIndex(int32_t batchIndex, int32_t batchSize) {
        if (context_) {
            context_->index -= (batchSize - batchIndex - 1);
        }
    }

    const MessageId& messageId() const noexcept {
        static MessageId kDefaultMessageId = MessageId::earliest();
        return context_ ? context_->messageId : kDefaultMessageId;
    }

    void setMessageId(const MessageId& messageId) {
        if (context_) {
            context_->messageId = messageId;
        }
    }

    std::shared_ptr<std::string> topicName() const { return context_ ? context_->topic : nullptr; }
    ClientConnection* cnx() const noexcept { return context_->cnx; }

    void setConsumer(std::shared_ptr<ConsumerImpl> consumer) {
        if (context_) {
            context_->consumer = consumer;
        }
    }
    std::shared_ptr<ConsumerImpl> consumer() { return context_ ? context_->consumer.lock() : nullptr; }

    const Message::StringMap& properties();

    const std::string& getPartitionKey() const;
    bool hasPartitionKey() const;

    const std::string& getOrderingKey() const;
    bool hasOrderingKey() const;

    uint64_t getPublishTimestamp() const;
    uint64_t getEventTimestamp() const;

    /**
     * Get the topic Name from which this message originated from
     */
    const std::string& getTopicName();

    /**
     * Set a valid topicName
     */
    void setTopicName(const std::shared_ptr<std::string>& topicName);

    int getRedeliveryCount();
    void setRedeliveryCount(int count);

    bool hasSchemaVersion() const;
    const std::string& getSchemaVersion() const;
    void setSchemaVersion(const std::string& value);
    void convertKeyValueToPayload(const SchemaInfo& schemaInfo);
    void convertPayloadToKeyValue(const SchemaInfo& schemaInfo);
    KeyValueEncodingType getKeyValueEncodingType(SchemaInfo schemaInfo);

    friend class PulsarWrapper;
    friend class MessageBuilder;

   private:
    proto::MessageMetadata metadata_;
    SharedBuffer payload_;
    std::shared_ptr<KeyValueImpl> keyValuePtr_;

    // If the Message is constructed by a MessageBuilder, the context should be null
    struct ConsumerContext {
        int64_t index;
        std::shared_ptr<std::string> topic;
        MessageId messageId;
        ClientConnection* cnx;
        int redeliveryCount;
        bool hasSchemaVersion;
        std::weak_ptr<ConsumerImpl> consumer;
        Message::StringMap properties;
    };
    std::unique_ptr<ConsumerContext> context_;

    void setReplicationClusters(const std::vector<std::string>& clusters);
    void setProperty(const std::string& name, const std::string& value);
    void disableReplication(bool flag);
    void setPartitionKey(const std::string& partitionKey);
    void setOrderingKey(const std::string& orderingKey);
    void setEventTimestamp(uint64_t eventTimestamp);
};
}  // namespace pulsar

#endif /* LIB_MESSAGEIMPL_H_ */
