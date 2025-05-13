#include <librdkafka/rdkafkacpp.h>
#include <iostream>

class KafkaConsumer {
    public:
        KafkaConsumer(const std::string& brokers,
                      const std::string& topic,
                      const std::string& group_id = "test-group")
        {
            std::string err;
            RdKafka::Conf* gconf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
            gconf->set("bootstrap.servers", brokers, err);
            gconf->set("group.id",          group_id, err);
            gconf->set("auto.offset.reset","earliest", err);
    
            consumer_ = RdKafka::KafkaConsumer::create(gconf, err);
            delete gconf;
            if (!consumer_) throw std::runtime_error("KafkaConsumer create failed: " + err);
    
            std::vector<std::string> topics = { topic };
            auto ec = consumer_->subscribe(topics);
            if (ec != RdKafka::ERR_NO_ERROR)
                throw std::runtime_error("Subscribe failed: " + RdKafka::err2str(ec));
        }
    
        ~KafkaConsumer() {
            consumer_->close();
            delete consumer_;
        }
    
        std::vector<std::string> consumeBatch(int max_messages = 10,
                                              int timeout_ms   = 1000)
        {
            std::vector<std::string> out;
            for (int i = 0; i < max_messages; ++i) {
                RdKafka::Message* msg = consumer_->consume(timeout_ms);
                switch (msg->err()) {
                    case RdKafka::ERR__TIMED_OUT:
                        return out;
                    case RdKafka::ERR_NO_ERROR:
                        out.emplace_back(
                            std::string(reinterpret_cast<const char*>(msg->payload()),
                                        msg->len())
                        );
                        break;
                    default:
                        std::cerr << "Consume error: " << msg->errstr() << std::endl;
                        return out;
                }
            }
            return out;
        }
    
    private:
        RdKafka::KafkaConsumer* consumer_;
    };