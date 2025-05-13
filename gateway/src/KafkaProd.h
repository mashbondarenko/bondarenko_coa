#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <string>
#include <librdkafka/rdkafkacpp.h>
#include <iostream>

class KafkaProducer {
    public:
        KafkaProducer(const std::string& brokers,
                      const std::string& topic,
                      int poll_timeout_ms = 100);
        ~KafkaProducer();
    
        bool sendMessage(const std::string& key,
                         const std::string& value);
    
    private:
        class DeliveryReportCb : public RdKafka::DeliveryReportCb {
        public:
            void dr_cb(RdKafka::Message& msg) override {
                if (msg.err())
                    std::cerr << "% Delivery failed: " << msg.errstr() << std::endl;
                else
                    std::cout << "% Delivered to " << msg.topic_name()
                              << "[" << msg.partition() << "] @"
                              << msg.offset() << std::endl;
            }
        } dr_cb_;
    
        RdKafka::Producer* producer_;
        RdKafka::Topic*    topic_;
        int                poll_timeout_ms_;
    };
    

#endif // KAFKA_PRODUCER_H