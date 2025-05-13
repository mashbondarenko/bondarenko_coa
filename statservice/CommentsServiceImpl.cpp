// CommentsServiceImpl.h
#include <grpcpp/grpcpp.h>
#include "stat.grpc.pb.h"
#include "KafkaConsumer.cpp"
#include <thread>
#include <mutex>
#include <vector>

class CommentsServiceImpl final : public stat::CommentService::Service {
public:
  CommentsServiceImpl(const std::string& brokers,
                      const std::string& topic)
    : brokers_(brokers), topic_(topic)
  {
    consumer_thread_ = std::thread([this](){
      KafkaConsumer c(brokers_, topic_, "test-group");
      while (running_) {
        auto msgs = c.consumeBatch(10, 500);
        if (!msgs.empty()) {
            for (const auto& m : msgs) {
                std::cout << "[Kafka] " << m << std::endl;
            }
            std::lock_guard<std::mutex> lk(mu_);
            comments_.insert(comments_.end(), msgs.begin(), msgs.end());
          }}
    });
  }

  ~CommentsServiceImpl() override {
    running_ = false;
    if (consumer_thread_.joinable()) consumer_thread_.join();
  }

  grpc::Status GetAllComments(grpc::ServerContext*,
                            const stat::GetAllCommentsRequest* req,
                            stat::GetAllCommentsResponse* resp) override
  {
    std::lock_guard<std::mutex> lk(mu_);
    int32_t sz = req->page_size();
    int32_t num = req->page();
    int start = (num-1)*sz;
    if (start < 0 || start >= (int)comments_.size()) {
      return grpc::Status::OK;
    }
    int end = std::min((int)comments_.size(), start + sz);
    for (int i = start; i < end; ++i) {
      resp->add_comments(comments_[i]);
    }
    return grpc::Status::OK;
  }

private:
  std::string brokers_, topic_;
  std::atomic<bool> running_{true};
  std::thread  consumer_thread_;
  std::mutex   mu_;
  std::vector<std::string> comments_;
};