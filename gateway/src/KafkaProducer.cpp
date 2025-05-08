#include "KafkaProducer.h"
#include <iostream>
#include <stdexcept>
#include <chrono>


KafkaProducer::KafkaProducer(const std::string& brokers,
    const std::string& topic,
    int poll_timeout_ms)
: producer_(nullptr),
topic_(nullptr),
poll_timeout_ms_(poll_timeout_ms)
{
std::string err;
RdKafka::Conf* gconf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
if (gconf->set("bootstrap.servers", brokers, err) != RdKafka::Conf::CONF_OK ||
gconf->set("dr_cb", &dr_cb_,       err) != RdKafka::Conf::CONF_OK) {
delete gconf;
throw std::runtime_error("KafkaProducer config error: " + err);
}

producer_ = RdKafka::Producer::create(gconf, err);
delete gconf;
if (!producer_) throw std::runtime_error("KafkaProducer create failed: " + err);

RdKafka::Conf* tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
topic_ = RdKafka::Topic::create(producer_, topic, tconf, err);
delete tconf;
if (!topic_) throw std::runtime_error("Topic create failed: " + err);
}

KafkaProducer::~KafkaProducer() {
producer_->flush(10 * 1000);
delete topic_;
delete producer_;
}

bool KafkaProducer::sendMessage(const std::string& key,
    const std::string& value)
{
RdKafka::ErrorCode rc = producer_->produce(
topic_, RdKafka::Topic::PARTITION_UA,
RdKafka::Producer::RK_MSG_COPY,
const_cast<char*>(value.data()), value.size(),
key.empty()? nullptr : key.data(),
key.empty()? 0 : key.size(),
nullptr
);
std::cerr << "[KafkaProducer] about to produce message: " << value << std::endl;

if (rc != RdKafka::ERR_NO_ERROR) {
std::cerr << "[KafkaProducer] produce() failed: "
<< RdKafka::err2str(rc) << std::endl;
return false;
}

producer_->poll(poll_timeout_ms_);

RdKafka::ErrorCode f = producer_->flush(5000);
std::cerr << "[KafkaProducer] flush returned: " << RdKafka::err2str(f) << std::endl;

if (f != RdKafka::ERR_NO_ERROR) {
std::cerr << "[KafkaProducer] flush() failed: "
<< RdKafka::err2str(f) << std::endl;
return false;
}

std::cout << "[KafkaProducer] message flushed to broker\n";
return true;
}