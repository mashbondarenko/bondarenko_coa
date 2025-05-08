#ifndef PROMO_SERVICE_PROXY_IMPL_H
#define PROMO_SERVICE_PROXY_IMPL_H

#include <memory>
#include <grpcpp/grpcpp.h>
#include "promo.grpc.pb.h"
#include "auth.grpc.pb.h"
#include "KafkaProducer.h"

namespace gateway {

class PromoServiceProxyImpl final : public promo::PromoService::Service {
public:
    PromoServiceProxyImpl(std::shared_ptr<grpc::Channel> promoChannel,
                          std::shared_ptr<grpc::Channel> authChannel);

    grpc::Status CreatePromoCode(grpc::ServerContext* context,
                                 const promo::CreatePromoCodeRequest* request,
                                 promo::PromoCode* response) override;

    grpc::Status UpdatePromoCode(grpc::ServerContext* context,
                                 const promo::UpdatePromoCodeRequest* request,
                                 promo::PromoCode* response) override;

    grpc::Status DeletePromoCode(grpc::ServerContext* context,
                                 const promo::DeletePromoCodeRequest* request,
                                 promo::PromoCode* response) override;

    grpc::Status GetPromoCodeById(grpc::ServerContext* context,
                                  const promo::GetPromoCodeRequest* request,
                                  promo::PromoCode* response) override;

    grpc::Status ListPromoCodes(grpc::ServerContext* context,
                                const promo::ListPromoCodesRequest* request,
                                promo::ListPromoCodesResponse* response) override;

    grpc::Status ClickPromoCode(grpc::ServerContext* context,
                                const promo::ClickPromoCodeRequest* request,
                                promo::PromoCode* response) override;

    grpc::Status CommentPromoCode(grpc::ServerContext* context,
                                  const promo::CommentPromoCodeRequest* request,
                                  promo::PromoCode* response) override;

private:
    std::unique_ptr<promo::PromoService::Stub> promo_stub_;
    std::unique_ptr<auth::AuthService::Stub> auth_stub_;

    grpc::Status ValidateToken(const std::string& token);
};

} // namespace gateway

#endif // PROMO_SERVICE_PROXY_IMPL_H
