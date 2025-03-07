#include "AuthServiceProxyImpl.h"

namespace gateway {

AuthServiceProxyImpl::AuthServiceProxyImpl(std::shared_ptr<grpc::Channel> channel)
    : stub_(auth::AuthService::NewStub(channel)) {}

grpc::Status AuthServiceProxyImpl::RegisterUser(grpc::ServerContext* context,
    const auth::RegisterUserRequest* request, auth::User* response) {
    grpc::ClientContext client_context;
    return stub_->RegisterUser(&client_context, *request, response);
}

grpc::Status AuthServiceProxyImpl::Login(grpc::ServerContext* context,
    const auth::LoginRequest* request, auth::LoginResponse* response) {
    grpc::ClientContext client_context;
    return stub_->Login(&client_context, *request, response);
}

grpc::Status AuthServiceProxyImpl::UpdateProfile(grpc::ServerContext* context,
    const auth::UpdateProfileRequest* request, auth::User* response) {
    grpc::ClientContext client_context;
    return stub_->UpdateProfile(&client_context, *request, response);
}

grpc::Status AuthServiceProxyImpl::GetProfile(grpc::ServerContext* context,
    const auth::GetProfileRequest* request, auth::User* response) {
    grpc::ClientContext client_context;
    return stub_->GetProfile(&client_context, *request, response);
}

} // namespace gateway
