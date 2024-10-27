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
#include "MessageImpl.h"

#include "ObjectPool.h"

namespace pulsar {

static ObjectPool<MessageImpl, 100000> messagePool;

const Message::StringMap& MessageImpl::properties() {
    static Message::StringMap kEmptyMap;
    if (context_ == nullptr) {
        return kEmptyMap;
    }
    if (context_->properties.size() == 0) {
        for (int i = 0; i < metadata_.properties_size(); i++) {
            const std::string& key = metadata_.properties(i).key();
            const std::string& value = metadata_.properties(i).value();
            context_->properties.insert(std::make_pair(key, value));
        }
    }
    return context_->properties;
}

const std::string& MessageImpl::getPartitionKey() const { return metadata_.partition_key(); }

bool MessageImpl::hasPartitionKey() const { return metadata_.has_partition_key(); }

const std::string& MessageImpl::getOrderingKey() const { return metadata_.ordering_key(); }

bool MessageImpl::hasOrderingKey() const { return metadata_.has_ordering_key(); }

uint64_t MessageImpl::getPublishTimestamp() const {
    if (metadata_.has_publish_time()) {
        return metadata_.publish_time();
    } else {
        return 0ull;
    }
}

uint64_t MessageImpl::getEventTimestamp() const {
    if (metadata_.has_event_time()) {
        return metadata_.event_time();
    } else {
        return 0ull;
    }
}

void MessageImpl::setReplicationClusters(const std::vector<std::string>& clusters) {
    google::protobuf::RepeatedPtrField<std::string> r(clusters.begin(), clusters.end());
    r.Swap(metadata_.mutable_replicate_to());
}

void MessageImpl::disableReplication(bool flag) {
    google::protobuf::RepeatedPtrField<std::string> r;
    if (flag) {
        r.AddAllocated(new std::string("__local__"));
    }
    r.Swap(metadata_.mutable_replicate_to());
}

void MessageImpl::setProperty(const std::string& name, const std::string& value) {
    proto::KeyValue* keyValue = proto::KeyValue().New();
    keyValue->set_key(name);
    keyValue->set_value(value);
    metadata_.mutable_properties()->AddAllocated(keyValue);
}

void MessageImpl::setPartitionKey(const std::string& partitionKey) {
    metadata_.set_partition_key(partitionKey);
}

void MessageImpl::setOrderingKey(const std::string& orderingKey) { metadata_.set_ordering_key(orderingKey); }

void MessageImpl::setEventTimestamp(uint64_t eventTimestamp) { metadata_.set_event_time(eventTimestamp); }

void MessageImpl::setTopicName(const std::shared_ptr<std::string>& topicName) {
    if (context_) {
        context_->topic = topicName;
        context_->messageId.setTopicName(topicName);
    }
}

const std::string& MessageImpl::getTopicName() {
    static std::string kEmptyTopic = "";
    return context_ ? *context_->topic : kEmptyTopic;
}

int MessageImpl::getRedeliveryCount() { return context_ ? context_->redeliveryCount : 0; }

void MessageImpl::setRedeliveryCount(int count) {
    if (context_) {
        context_->redeliveryCount = count;
    }
}

bool MessageImpl::hasSchemaVersion() const { return metadata_.has_schema_version(); }

void MessageImpl::setSchemaVersion(const std::string& schemaVersion) {
    metadata_.set_schema_version(schemaVersion);
}

const std::string& MessageImpl::getSchemaVersion() const { return metadata_.schema_version(); }

void MessageImpl::convertKeyValueToPayload(const pulsar::SchemaInfo& schemaInfo) {
    if (schemaInfo.getSchemaType() != KEY_VALUE) {
        // ignore not key_value schema.
        return;
    }
    KeyValueEncodingType keyValueEncodingType = getKeyValueEncodingType(schemaInfo);
    payload_ = keyValuePtr_->getContent(keyValueEncodingType);
    if (keyValueEncodingType == KeyValueEncodingType::SEPARATED) {
        setPartitionKey(keyValuePtr_->getKey());
    }
}

void MessageImpl::convertPayloadToKeyValue(const pulsar::SchemaInfo& schemaInfo) {
    if (schemaInfo.getSchemaType() != KEY_VALUE) {
        // ignore not key_value schema.
        return;
    }
    keyValuePtr_ =
        std::make_shared<KeyValueImpl>(static_cast<const char*>(payload_.data()), payload_.readableBytes(),
                                       getKeyValueEncodingType(schemaInfo));
}

KeyValueEncodingType MessageImpl::getKeyValueEncodingType(SchemaInfo schemaInfo) {
    if (schemaInfo.getSchemaType() != KEY_VALUE) {
        throw std::invalid_argument("Schema not key value type.");
    }
    const StringMap& properties = schemaInfo.getProperties();
    auto data = properties.find("kv.encoding.type");
    if (data == properties.end()) {
        throw std::invalid_argument("Not found kv.encoding.type by properties");
    } else {
        return enumEncodingType(data->second);
    }
}

}  // namespace pulsar
