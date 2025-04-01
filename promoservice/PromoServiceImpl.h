#ifndef PROMO_SERVICE_IMPL_H
#define PROMO_SERVICE_IMPL_H

#include <string>
#include <grpcpp/grpcpp.h>
#include "promo.grpc.pb.h"

class PromoServiceImpl final : public promo::PromoService::Service {
public:
    explicit PromoServiceImpl(const std::string& db_conn_str);

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

private:
    std::string conn_str_;
};

#endif // PROMO_SERVICE_IMPL_H