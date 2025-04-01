#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "PromoServiceImpl.h"

using grpc::Server;
using grpc::ServerBuilder;

void RunServer(const std::string& server_address, const std::string& db_conn_str) {
    PromoServiceImpl service(db_conn_str);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Promo Service listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50053");
    std::string db_conn_str("host=localhost user=postgres password=postgres dbname=promodb");
    RunServer(server_address, db_conn_str);
    return 0;
}