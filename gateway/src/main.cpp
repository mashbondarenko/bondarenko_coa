#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "AuthServiceProxyImpl.h"
#include "PromoServiceProxyImpl.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::InsecureServerCredentials;

namespace gateway {

void RunServer(const std::string& server_address, 
               const std::string& authmanager_address,
               const std::string& promomanager_address) {
    AuthServiceProxyImpl authService(
        grpc::CreateChannel(authmanager_address, grpc::InsecureChannelCredentials())
    );
    PromoServiceProxyImpl promoService(
        grpc::CreateChannel(promomanager_address, grpc::InsecureChannelCredentials()),
        grpc::CreateChannel(authmanager_address, grpc::InsecureChannelCredentials())
    );

    ServerBuilder builder;
    builder.AddListeningPort(server_address, InsecureServerCredentials());
    builder.RegisterService(&authService);
    builder.RegisterService(&promoService);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "API Gateway listening on " << server_address << std::endl;
    server->Wait();
}

} // namespace gateway

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50052");
    std::string authmanager_address("localhost:50051");
    std::string promomanager_address("localhost:50053");

    gateway::RunServer(server_address, authmanager_address, promomanager_address);
    return 0;
}