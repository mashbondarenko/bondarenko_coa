#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "auth.grpc.pb.h"
#include "../src/AuthServiceProxyImpl.h"
#include "../src/PromoServiceProxyImpl.h"
#include "KafkaCons.cpp"
#include <thread>
#include <chrono>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::InsecureServerCredentials;

class AuthServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        chanel  = grpc::CreateChannel(
            "localhost:50051",
            grpc::InsecureChannelCredentials()
        );
        chanel2 = grpc::CreateChannel(
            "localhost:50053",
            grpc::InsecureChannelCredentials()
        );

        promoStub = promo::PromoService::NewStub(chanel2);
        serviceImpl = std::make_unique<gateway::AuthServiceProxyImpl>(chanel );
        serviceImplPromo = std::make_unique<gateway::PromoServiceProxyImpl>(chanel , chanel2);

        {
            grpc::ServerContext ctx;
            auth::RegisterUserRequest req;
            req.set_login("testuser");
            req.set_password("testpass");
            req.set_email("test@example.com");
            auth::User resp;
            auto st = serviceImpl->RegisterUser(&ctx, &req, &resp);
            ASSERT_TRUE(st.ok() || st.error_code() == grpc::StatusCode::ALREADY_EXISTS);
        }
        {
            grpc::ServerContext ctx;
            auth::LoginRequest req;
            req.set_login("testuser");
            req.set_password("testpass");
            auth::LoginResponse resp;
            auto st = serviceImpl->Login(&ctx, &req, &resp);
            ASSERT_TRUE(st.ok());
            token = resp.token();
        }
    }

    std::unique_ptr<gateway::AuthServiceProxyImpl> serviceImpl;
    std::unique_ptr<gateway::PromoServiceProxyImpl> serviceImplPromo;
    std::shared_ptr<grpc::Channel>                  chanel ;
    std::shared_ptr<grpc::Channel>                  chanel2;
    std::string                                     token;
    std::unique_ptr<promo::PromoService::Stub> promoStub;
};

/// Пользователи 1, e2e 1
TEST_F(AuthServiceTest, RegisterUser) {
    grpc::ServerContext ctx;
    auth::RegisterUserRequest req;
    req.set_login("newuser");
    req.set_password("newpass");
    req.set_email("new@example.com");
    auth::User resp;

    auto st = serviceImpl->RegisterUser(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok() || st.error_code() == grpc::StatusCode::ALREADY_EXISTS);

    grpc::ServerContext ctxLogin;
    auth::LoginRequest reqLogin;
    reqLogin.set_login("newuser");
    reqLogin.set_password("newpass");
    auth::LoginResponse respLogin;

    st = serviceImpl->Login(&ctxLogin, &reqLogin, &respLogin);
    EXPECT_TRUE(st.ok());
    EXPECT_FALSE(respLogin.token().empty());
    EXPECT_EQ(respLogin.user().login(), "newuser");
    EXPECT_EQ(respLogin.user().email(), "new@example.com");
}

/// 2e2 2
TEST_F(AuthServiceTest, Promo) {
    grpc::ServerContext ctx;
    auth::RegisterUserRequest req;
    req.set_login("testuser");
    req.set_password("testpass");
    req.set_email("test@example.com");
    auth::User resp;

    auto st = serviceImpl->RegisterUser(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok() || st.error_code() == grpc::StatusCode::ALREADY_EXISTS);

    grpc::ServerContext ctxLogin;
    auth::LoginRequest reqLogin;
    reqLogin.set_login("testuser");
    reqLogin.set_password("testpass");
    auth::LoginResponse respLogin;

    st = serviceImpl->Login(&ctxLogin, &reqLogin, &respLogin);
    EXPECT_TRUE(st.ok());
    EXPECT_FALSE(respLogin.token().empty());
    EXPECT_EQ(respLogin.user().login(), "testuser");
    EXPECT_EQ(respLogin.user().email(), "newemail@example.com");


    grpc::ClientContext context;
    promo::CreatePromoCodeRequest create_req;
    create_req.set_token(token);
    create_req.set_title("Promo");
    create_req.set_description("Description");
    create_req.set_discount(10.5);
    create_req.set_code("11");

    promo::PromoCode create_resp;
    grpc::Status status = promoStub->CreatePromoCode(&context, create_req, &create_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(create_resp.id(), 0);
    EXPECT_EQ(create_resp.title(), "Promo");

    promo::GetPromoCodeRequest get_req;
    get_req.set_token("testuser");
    get_req.set_promo_id(create_resp.id());
    grpc::ClientContext context2;

    promo::PromoCode get_resp;
    status = promoStub->GetPromoCodeById(&context2, get_req, &get_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(get_resp.id(), create_resp.id());
    EXPECT_EQ(get_resp.title(), "Promo");
}

/// 2e2 3
TEST_F(AuthServiceTest, PromoUpdate) {
    grpc::ServerContext ctx;
    auth::RegisterUserRequest req;
    req.set_login("testuser");
    req.set_password("testpass");
    req.set_email("test@example.com");
    auth::User resp;

    auto st = serviceImpl->RegisterUser(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok() || st.error_code() == grpc::StatusCode::ALREADY_EXISTS);

    grpc::ServerContext ctxLogin;
    auth::LoginRequest reqLogin;
    reqLogin.set_login("testuser");
    reqLogin.set_password("testpass");
    auth::LoginResponse respLogin;

    st = serviceImpl->Login(&ctxLogin, &reqLogin, &respLogin);
    EXPECT_TRUE(st.ok());
    EXPECT_FALSE(respLogin.token().empty());
    EXPECT_EQ(respLogin.user().login(), "testuser");
    EXPECT_EQ(respLogin.user().email(), "newemail@example.com");


    grpc::ClientContext context;
    promo::CreatePromoCodeRequest create_req;
    create_req.set_token(token);
    create_req.set_title("Promo");
    create_req.set_description("Description");
    create_req.set_discount(10.5);
    create_req.set_code("11");

    promo::PromoCode create_resp;
    grpc::Status status = promoStub->CreatePromoCode(&context, create_req, &create_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(create_resp.id(), 0);
    EXPECT_EQ(create_resp.title(), "Promo");

    grpc::ClientContext contextUp;
    promo::UpdatePromoCodeRequest update_req;

    update_req.set_token(token);
    update_req.set_promo_id(create_resp.id());
    update_req.set_title("New Promo");
    update_req.set_description("New Description");
    update_req.set_discount(10.5);
    update_req.set_code("11");

    promo::PromoCode update_resp;
    grpc::Status statusUp = promoStub->UpdatePromoCode(&contextUp,update_req, &update_resp);
    EXPECT_TRUE(statusUp.ok());
    EXPECT_GT(update_resp.id(), 0);
    EXPECT_EQ(update_resp.title(), "New Promo");

    promo::GetPromoCodeRequest get_req;
    get_req.set_token("testuser");
    get_req.set_promo_id(create_resp.id());
    grpc::ClientContext context2;

    promo::PromoCode get_resp;
    status = promoStub->GetPromoCodeById(&context2, get_req, &get_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(get_resp.id(), create_resp.id());
    EXPECT_EQ(get_resp.title(), "New Promo");
}

/// Пользователи 2 
TEST_F(AuthServiceTest, UpdateProfile) {
    grpc::ServerContext ctx;
    auth::UpdateProfileRequest req;
    req.set_token(token);
    req.set_first_name("Test");
    req.set_last_name("User");
    req.set_birth_date("2000-01-01");
    req.set_email("newemail@example.com");
    req.set_phone("123456789");
    auth::User resp;

    auto st = serviceImpl->UpdateProfile(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok());
    EXPECT_EQ(resp.first_name(), "Test");
    EXPECT_EQ(resp.last_name(),  "User");
    EXPECT_EQ(resp.birth_date(), "2000-01-01");
    EXPECT_EQ(resp.email(),      "newemail@example.com");
    EXPECT_EQ(resp.phone(),      "123456789");

    grpc::ServerContext ctxGet;
    auth::GetProfileRequest reqGet;
    reqGet.set_token(token);
    auth::User respGet;

     st = serviceImpl->GetProfile(&ctxGet, &reqGet, &respGet);
    EXPECT_TRUE(st.ok());
    EXPECT_EQ(respGet.email(),      "newemail@example.com");
    EXPECT_EQ(respGet.first_name(), "Test");
    EXPECT_EQ(respGet.last_name(),  "User");
    EXPECT_EQ(respGet.birth_date(),"2000-01-01");
    EXPECT_EQ(respGet.phone(),     "123456789");
}

/// Пользователи 3  
TEST_F(AuthServiceTest, GetProfile) {
    grpc::ServerContext ctx;
    auth::GetProfileRequest req;
    req.set_token(token);
    auth::User resp;

    auto st = serviceImpl->GetProfile(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok());
    EXPECT_EQ(resp.login(),      "testuser");
    EXPECT_EQ(resp.email(),      "newemail@example.com");
    EXPECT_EQ(resp.first_name(), "Test");
    EXPECT_EQ(resp.last_name(),  "User");
    EXPECT_EQ(resp.birth_date(),"2000-01-01");
    EXPECT_EQ(resp.phone(),     "123456789");
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
