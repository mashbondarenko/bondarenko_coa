#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "StatServiceProxyImpl.cpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::InsecureServerCredentials;

int main(int argc, char** argv) {
    const std::string kafka_brokers = "localhost:9092";
  const std::string kafka_topic   = "user-registration";

  CommentsServiceImpl service(kafka_brokers, kafka_topic);

  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:6000", grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "CommentsService listening on port 6000" << std::endl;

  server->Wait();
  return 0;
}