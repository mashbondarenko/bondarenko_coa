#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "AuthServiceProxyImpl.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::InsecureServerCredentials;

namespace gateway {


void RunServer(const std::string& server_address, const std::string& authmanager_address) {
    AuthServiceProxyImpl service(
        grpc::CreateChannel(authmanager_address, grpc::InsecureChannelCredentials())
    );

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << server_address << std::endl;
    server->Wait();
}


} // namespace gateway

int main(int argc, char** argv) {
    std::string serverAddress("0.0.0.0:50052");
    std::string authmanagerAddress("localhost:50051");

    gateway::RunServer(serverAddress, authmanagerAddress);
    return 0;
}
