#ifndef AUTHSERVICEIMPL_H
#define AUTHSERVICEIMPL_H

#include <string>
#include "auth.grpc.pb.h"

class AuthServiceImpl final : public auth::AuthService::Service {
public:
    explicit AuthServiceImpl(const std::string& db_conn_str);

    grpc::Status RegisterUser(grpc::ServerContext* context, const auth::RegisterUserRequest* request,
                                auth::User* response) override;

    grpc::Status Login(grpc::ServerContext* context, const auth::LoginRequest* request,
                         auth::LoginResponse* response) override;

    grpc::Status UpdateProfile(grpc::ServerContext* context, const auth::UpdateProfileRequest* request,
                               auth::User* response) override;

    grpc::Status GetProfile(grpc::ServerContext* context, const auth::GetProfileRequest* request,
                            auth::User* response) override;

private:
    std::string conn_str_;
};

#endif // AUTHSERVICEIMPL_H
