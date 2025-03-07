#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "AuthServiceImpl.h"

using grpc::Server;
using grpc::ServerBuilder;

void RunServer(const std::string& server_address, const std::string& db_conn_str) {
    AuthServiceImpl service(db_conn_str);
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50051");
    std::string db_conn_str("host=localhost user=postgres password=postgres dbname=authdb");
    RunServer(server_address, db_conn_str);
    return 0;
}
