#include "KafkaProducer.h"
#include <librdkafka/rdkafkacpp.h>
#include <iostream>

KafkaProducer::KafkaProducer(const std::string& brokers, const std::string& topic)
    : brokers_(brokers), topic_(topic) {

    config_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    config_->set("metadata.broker.list", brokers_, errstr_);

    producer_ = RdKafka::Producer::create(config_, errstr_);
    if (!producer_) {
        std::cerr << "Failed to create producer: " << errstr_ << std::endl;
        exit(1);
    }
}

void KafkaProducer::SendMessage(const std::string& message) {
    RdKafka::Topic *topic = RdKafka::Topic::create(producer_, topic_, nullptr, errstr_);

    if (!topic) {
        std::cerr << "Failed to create topic: " << errstr_ << std::endl;
        return;
    }

    RdKafka::ErrorCode resp = producer_->produce(topic, RdKafka::Topic::PARTITION_UA,
                                                 RdKafka::Producer::RK_MSG_COPY,
                                                 const_cast<char*>(message.c_str()), message.size(),
                                                 nullptr, nullptr);

    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to send message: " << RdKafka::err2str(resp) << std::endl;
    }
    delete topic;
}

KafkaProducer::~KafkaProducer() {
    delete config_;
    delete producer_;
}