#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "promo.grpc.pb.h"
#include "PromoServiceImpl.h"

class PromoServiceUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_conn_str = "host=localhost user=postgres password=postgres dbname=promodb_test";
        service_impl = new PromoServiceImpl(db_conn_str);
    }

    void TearDown() override {
        delete service_impl;
    }

    std::string db_conn_str;
    PromoServiceImpl* service_impl;
};

TEST_F(PromoServiceUnitTest, CreateAndGetPromoCodeDirectCall) {
    grpc::ServerContext context;

    promo::CreatePromoCodeRequest create_req;
    create_req.set_token("testuser");
    create_req.set_title("Test Promo");
    create_req.set_description("Test Description");
    create_req.set_discount(10.5);
    create_req.set_code("TEST123");

    promo::PromoCode create_resp;
    grpc::Status status = service_impl->CreatePromoCode(&context, &create_req, &create_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(create_resp.id(), 0);
    EXPECT_EQ(create_resp.title(), "Test Promo");

    promo::GetPromoCodeRequest get_req;
    get_req.set_token("testuser");
    get_req.set_promo_id(create_resp.id());

    promo::PromoCode get_resp;
    status = service_impl->GetPromoCodeById(&context, &get_req, &get_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(get_resp.id(), create_resp.id());
    EXPECT_EQ(get_resp.title(), "Test Promo");
}

TEST_F(PromoServiceUnitTest, ListPromoCodesDirectCall) {
    grpc::ServerContext context;

    const int promoCount = 2;
    for (int i = 0; i < promoCount; i++) {
        promo::CreatePromoCodeRequest req;
        req.set_token("testuser");
        req.set_title("Promo " + std::to_string(i));
        req.set_description("Desc " + std::to_string(i));
        req.set_discount(5.0 + i);
        req.set_code("CODE" + std::to_string(i));

        promo::PromoCode resp;
        grpc::Status create_status = service_impl->CreatePromoCode(&context, &req, &resp);
        EXPECT_TRUE(create_status.ok());
    }
    
    promo::ListPromoCodesRequest list_req;
    list_req.set_token("testuser");
    list_req.set_page(1);
    list_req.set_page_size(10);

    promo::ListPromoCodesResponse list_resp;
    grpc::Status status = service_impl->ListPromoCodes(&context, &list_req, &list_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_GE(list_resp.total_count(), promoCount);
    EXPECT_GE(list_resp.promo_codes_size(), promoCount);
}

TEST_F(PromoServiceUnitTest, UpdatePromoCodeDirectCall) {
    grpc::ServerContext context;
    
    promo::CreatePromoCodeRequest create_req;
    create_req.set_token("testuser");
    create_req.set_title("Original Title");
    create_req.set_description("Original Description");
    create_req.set_discount(10.0);
    create_req.set_code("ORIGCODE");
    promo::PromoCode create_resp;
    grpc::Status status = service_impl->CreatePromoCode(&context, &create_req, &create_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_GT(create_resp.id(), 0);

    promo::UpdatePromoCodeRequest update_req;
    update_req.set_token("testuser");
    update_req.set_promo_id(create_resp.id());
    update_req.set_title("Updated Title");
    update_req.set_description("Updated Description");
    update_req.set_discount(15.0);
    update_req.set_code("UPDATEDCODE");
    promo::PromoCode update_resp;
    status = service_impl->UpdatePromoCode(&context, &update_req, &update_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(update_resp.id(), create_resp.id());
    EXPECT_EQ(update_resp.title(), "Updated Title");
    EXPECT_EQ(update_resp.description(), "Updated Description");
    EXPECT_DOUBLE_EQ(update_resp.discount(), 15.0);
    EXPECT_EQ(update_resp.code(), "UPDATEDCODE");

    promo::GetPromoCodeRequest get_req;
    get_req.set_token("testuser");
    get_req.set_promo_id(create_resp.id());
    promo::PromoCode get_resp;
    status = service_impl->GetPromoCodeById(&context, &get_req, &get_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(get_resp.title(), "Updated Title");
}TEST_F(PromoServiceUnitTest, DeletePromoCodeDirectCall) {
    grpc::ServerContext context;
    
    promo::CreatePromoCodeRequest create_req;
    create_req.set_token("testuser");
    create_req.set_title("Promo To Delete");
    create_req.set_description("Description");
    create_req.set_discount(20.0);
    create_req.set_code("DELETE_ME");
    promo::PromoCode create_resp;
    grpc::Status status = service_impl->CreatePromoCode(&context, &create_req, &create_resp);
    EXPECT_TRUE(status.ok());
    int64_t promo_id = create_resp.id();
    EXPECT_GT(promo_id, 0);

    promo::DeletePromoCodeRequest delete_req;
    delete_req.set_token("testuser");
    delete_req.set_promo_id(promo_id);
    promo::PromoCode delete_resp;
    status = service_impl->DeletePromoCode(&context, &delete_req, &delete_resp);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(delete_resp.id(), promo_id);

    promo::GetPromoCodeRequest get_req;
    get_req.set_token("testuser");
    get_req.set_promo_id(promo_id);
    promo::PromoCode get_resp;
    status = service_impl->GetPromoCodeById(&context, &get_req, &get_resp);
    EXPECT_FALSE(status.ok());
}