#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <string>
#include <librdkafka/rdkafkacpp.h>

class KafkaProducer {
public:
    KafkaProducer(const std::string& brokers, const std::string& topic);
    void SendMessage(const std::string& message);
    ~KafkaProducer();

private:
    std::string brokers_;
    std::string topic_;
    std::string errstr_;
    RdKafka::Producer* producer_;
    RdKafka::Conf* config_;
};

#endif // KAFKA_PRODUCER_H