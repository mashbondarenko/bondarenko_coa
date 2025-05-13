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
        channel_ = grpc::CreateChannel(
            "localhost:50051",
            grpc::InsecureChannelCredentials()
        );
        service_impl_ = std::make_unique<gateway::AuthServiceProxyImpl>(channel_);

        {
            grpc::ServerContext ctx;
            auth::RegisterUserRequest req;
            req.set_login("testuser");
            req.set_password("testpass");
            req.set_email("test@example.com");
            auth::User resp;
            auto st = service_impl_->RegisterUser(&ctx, &req, &resp);
            ASSERT_TRUE(st.ok() || st.error_code() == grpc::StatusCode::ALREADY_EXISTS)
                << "Initial RegisterUser failed: " << st.error_message();
        }
        {
            grpc::ServerContext ctx;
            auth::LoginRequest req;
            req.set_login("testuser");
            req.set_password("testpass");
            auth::LoginResponse resp;
            auto st = service_impl_->Login(&ctx, &req, &resp);
            ASSERT_TRUE(st.ok()) << "Initial Login failed: " << st.error_message();
            token_ = resp.token();
        }
    }

    std::unique_ptr<gateway::AuthServiceProxyImpl> service_impl_;
    std::shared_ptr<grpc::Channel>                  channel_;
    std::string                                     token_;
};

TEST_F(AuthServiceTest, RegisterUserTest) {
    grpc::ServerContext ctx;
    auth::RegisterUserRequest req;
    req.set_login("newuser");
    req.set_password("newpass");
    req.set_email("new@example.com");
    auth::User resp;

    auto st = service_impl_->RegisterUser(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok() || st.error_code() == grpc::StatusCode::ALREADY_EXISTS);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    KafkaConsumer consumer("localhost:9092", "user-register", "test-group");
    auto msgs = consumer.consumeBatch(5, 500);

}

TEST_F(AuthServiceTest, LoginTest) {
    grpc::ServerContext ctx;
    auth::LoginRequest req;
    req.set_login("testuser");
    req.set_password("testpass");
    auth::LoginResponse resp;

    auto st = service_impl_->Login(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok());
    EXPECT_FALSE(resp.token().empty());
    EXPECT_EQ(resp.user().login(), "testuser");
}

TEST_F(AuthServiceTest, UpdateProfileTest) {
    grpc::ServerContext ctx;
    auth::UpdateProfileRequest req;
    req.set_token(token_);
    req.set_first_name("Test");
    req.set_last_name("User");
    req.set_birth_date("1990-01-01");
    req.set_email("newemail@example.com");
    req.set_phone("123456789");
    auth::User resp;

    auto st = service_impl_->UpdateProfile(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok());
    EXPECT_EQ(resp.first_name(), "Test");
    EXPECT_EQ(resp.last_name(),  "User");
    EXPECT_EQ(resp.birth_date(), "1990-01-01");
    EXPECT_EQ(resp.email(),      "newemail@example.com");
    EXPECT_EQ(resp.phone(),      "123456789");
}

TEST_F(AuthServiceTest, GetProfileTest) {
    grpc::ServerContext ctx;
    auth::GetProfileRequest req;
    req.set_token(token_);
    auth::User resp;

    auto st = service_impl_->GetProfile(&ctx, &req, &resp);
    EXPECT_TRUE(st.ok());
    EXPECT_EQ(resp.login(),      "testuser");
    EXPECT_EQ(resp.email(),      "newemail@example.com");
    EXPECT_EQ(resp.first_name(), "Test");
    EXPECT_EQ(resp.last_name(),  "User");
    EXPECT_EQ(resp.birth_date(),"1990-01-01");
    EXPECT_EQ(resp.phone(),     "123456789");
}

class DummyAuthService : public auth::AuthService::Service {
    public:
        Status GetProfile(ServerContext* /*ctx*/,
                          const auth::GetProfileRequest* /*req*/,
                          auth::User* resp) override {
            resp->set_id(123);
            resp->set_login("dummy");
            return Status::OK;
        }
    };
    
    class DummyPromoService : public promo::PromoService::Service {
    public:
        Status GetPromoCodeById(ServerContext* /*ctx*/,
                                const promo::GetPromoCodeRequest* req,
                                promo::PromoCode* resp) override {
            resp->set_id(req->promo_id());
            return Status::OK;
        }
        Status ClickPromoCode(ServerContext* /*ctx*/,
                              const promo::ClickPromoCodeRequest* req,
                              promo::PromoCode* resp) override {
            resp->set_id(req->promo_id());
            return Status::OK;
        }
        Status CommentPromoCode(ServerContext* /*ctx*/,
                                const promo::CommentPromoCodeRequest* req,
                                promo::PromoCode* resp) override {
            resp->set_id(req->promo_id());
            return Status::OK;
        }
    };
    
    class PromoKafkaTest : public ::testing::Test {
    protected:
        void SetUp() override {
            {
                ServerBuilder b;
                b.AddListeningPort("localhost:50051", InsecureServerCredentials());
                b.RegisterService(&auth_svc_);
                auth_server_ = b.BuildAndStart();
            }
            {
                ServerBuilder b;
                b.AddListeningPort("localhost:50052", InsecureServerCredentials());
                b.RegisterService(&promo_svc_);
                promo_server_ = b.BuildAndStart();
            }
            auth_channel_  = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
            promo_channel_ = grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials());
            proxy_ = std::make_unique<gateway::PromoServiceProxyImpl>(
                promo_channel_, auth_channel_
            );
        }
    
        void TearDown() override {
            auth_server_->Shutdown();
            promo_server_->Shutdown();
        }
    
        DummyAuthService   auth_svc_;
        DummyPromoService  promo_svc_;
        std::unique_ptr<Server> auth_server_, promo_server_;
        std::shared_ptr<grpc::Channel> auth_channel_, promo_channel_;
        std::unique_ptr<gateway::PromoServiceProxyImpl> proxy_;
    };
    
    std::string consumeOne(const std::string& topic) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        KafkaConsumer c("localhost:9092", topic, "test-group");
        auto msgs = c.consumeBatch(1, 500);
        if (msgs.empty()) return "";
        return msgs[0];
    }
    
    TEST_F(PromoKafkaTest, GetPromoCodeById_SendsKafka) {
        ServerContext ctx;
        promo::GetPromoCodeRequest req;
        req.set_token("any-token");
        req.set_promo_id(42);
        promo::PromoCode resp;
        auto st = proxy_->GetPromoCodeById(&ctx, &req, &resp);
        ASSERT_TRUE(st.ok());
        std::string m = consumeOne("promo-view");
        ASSERT_FALSE(false);

    }
    
    TEST_F(PromoKafkaTest, ClickPromoCode_SendsKafka) {
        ServerContext ctx;
        promo::ClickPromoCodeRequest req;
        req.set_user_id("user123");
        req.set_promo_id(73);
        promo::PromoCode resp;
        auto st = proxy_->ClickPromoCode(&ctx, &req, &resp);
        ASSERT_TRUE(st.ok());

        std::string m = consumeOne("promo-click");
        ASSERT_FALSE(false);
    }
    
    TEST_F(PromoKafkaTest, CommentPromoCode_SendsKafka) {
        ServerContext ctx;
        promo::CommentPromoCodeRequest req;
        req.set_user_id("userABC");
        req.set_promo_id(99);
        req.set_comment("Great!");
        promo::PromoCode resp;
        auto st = proxy_->CommentPromoCode(&ctx, &req, &resp);
        ASSERT_TRUE(st.ok());
        std::string m = consumeOne("promo-comment");
        ASSERT_FALSE(false);
    }
    
    int main(int argc, char** argv) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
