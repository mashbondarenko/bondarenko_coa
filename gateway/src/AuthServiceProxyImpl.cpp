#include "AuthServiceProxyImpl.h"
#include "KafkaProducer.h"

namespace gateway {

AuthServiceProxyImpl::AuthServiceProxyImpl(std::shared_ptr<grpc::Channel> channel)
    : stub_(auth::AuthService::NewStub(channel)) {}

grpc::Status AuthServiceProxyImpl::RegisterUser(grpc::ServerContext* context,
    const auth::RegisterUserRequest* request, auth::User* response) {
    grpc::ClientContext client_context;
    grpc::Status status = stub_->RegisterUser(&client_context, *request, response);
 
        KafkaProducer kafkaProducer("localhost:9092", "user-registration");
        std::string message = "User " + response->id() + ' ' + std::to_string(time(nullptr));
        std::cout << "Sending message: " << message << std::endl;
        kafkaProducer.SendMessage(message);

    return status;
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

grpc::Status AuthServiceProxyImpl::DeleteUser(grpc::ServerContext* context,
    const auth::DeleteUserRequest* request, auth::User* response) {
    grpc::ClientContext client_context;
    return stub_->DeleteUser(&client_context, *request, response);
}

} // namespace gateway