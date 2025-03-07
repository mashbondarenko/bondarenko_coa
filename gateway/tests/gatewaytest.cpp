#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "auth.grpc.pb.h"

class AuthServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
        stub = auth::AuthService::NewStub(channel);
    }

    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<auth::AuthService::Stub> stub;
};

TEST_F(AuthServiceTest, RegisterUserTest) {
    auth::RegisterUserRequest request;
    request.set_login("testuser");
    request.set_password("testpass");
    request.set_email("test@example.com");

    auth::User response;
    grpc::ClientContext context;
    grpc::Status status = stub->RegisterUser(&context, request, &response);

    EXPECT_TRUE(status.ok()) << "Registration failed: " << status.error_message();
    EXPECT_EQ(response.login(), "testuser");
    EXPECT_EQ(response.email(), "test@example.com");
    EXPECT_FALSE(response.created_at().empty());
    EXPECT_FALSE(response.updated_at().empty());
    EXPECT_GT(response.id(), 0);
}

TEST_F(AuthServiceTest, LoginTest) {
    auth::LoginRequest request;
    request.set_login("testuser");
    request.set_password("testpass");

    auth::LoginResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub->Login(&context, request, &response);

    EXPECT_TRUE(status.ok()) << "Login failed: " << status.error_message();
    EXPECT_FALSE(response.token().empty());
    EXPECT_EQ(response.user().login(), "testuser");
    EXPECT_EQ(response.user().email(), "test@example.com");
}

TEST_F(AuthServiceTest, UpdateProfileTest) {
    auth::UpdateProfileRequest request;
    request.set_token("testuser");
    request.set_first_name("Test");
    request.set_last_name("User");
    request.set_birth_date("1990-01-01");
    request.set_email("newemail@example.com");
    request.set_phone("123456789");

    auth::User response;
    grpc::ClientContext context;
    grpc::Status status = stub->UpdateProfile(&context, request, &response);

    EXPECT_TRUE(status.ok()) << "UpdateProfile failed: " << status.error_message();
    EXPECT_EQ(response.first_name(), "Test");
    EXPECT_EQ(response.last_name(), "User");
    EXPECT_EQ(response.birth_date(), "1990-01-01");
    EXPECT_EQ(response.email(), "newemail@example.com");
    EXPECT_EQ(response.phone(), "123456789");
}

TEST_F(AuthServiceTest, GetProfileTest) {
    auth::GetProfileRequest request;
    request.set_token("testuser");

    auth::User response;
    grpc::ClientContext context;
    grpc::Status status = stub->GetProfile(&context, request, &response);

    EXPECT_TRUE(status.ok()) << "GetProfile failed: " << status.error_message();
    EXPECT_EQ(response.login(), "testuser");
    EXPECT_EQ(response.email(), "newemail@example.com");
    EXPECT_EQ(response.first_name(), "Test");
    EXPECT_EQ(response.last_name(), "User");
    EXPECT_EQ(response.birth_date(), "1990-01-01");
    EXPECT_EQ(response.phone(), "123456789");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
