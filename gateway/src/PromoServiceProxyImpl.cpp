#include "PromoServiceProxyImpl.h"

namespace gateway {

PromoServiceProxyImpl::PromoServiceProxyImpl(std::shared_ptr<grpc::Channel> promoChannel,
                                             std::shared_ptr<grpc::Channel> authChannel)
    : promo_stub_(promo::PromoService::NewStub(promoChannel)),
      auth_stub_(auth::AuthService::NewStub(authChannel))
{}

grpc::Status PromoServiceProxyImpl::ValidateToken(const std::string& token) {
    auth::GetProfileRequest authReq;
    authReq.set_token(token);
    auth::User dummyUser;
    grpc::ClientContext authContext;
    grpc::Status authStatus = auth_stub_->GetProfile(&authContext, authReq, &dummyUser);
    if (!authStatus.ok()) {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Недействительный токен");
    }
    return grpc::Status::OK;
}

grpc::Status PromoServiceProxyImpl::CreatePromoCode(grpc::ServerContext* context,
                                                    const promo::CreatePromoCodeRequest* request,
                                                    promo::PromoCode* response) {
    grpc::Status tokenStatus = ValidateToken(request->token());
    if (!tokenStatus.ok()) {
        return tokenStatus;
    }
    grpc::ClientContext client_context;
    return promo_stub_->CreatePromoCode(&client_context, *request, response);
}

grpc::Status PromoServiceProxyImpl::UpdatePromoCode(grpc::ServerContext* context,
                                                    const promo::UpdatePromoCodeRequest* request,
                                                    promo::PromoCode* response) {
    grpc::Status tokenStatus = ValidateToken(request->token());
    if (!tokenStatus.ok()) {
        return tokenStatus;
    }
    grpc::ClientContext client_context;
    return promo_stub_->UpdatePromoCode(&client_context, *request, response);
}

grpc::Status PromoServiceProxyImpl::DeletePromoCode(grpc::ServerContext* context,
                                                    const promo::DeletePromoCodeRequest* request,
                                                    promo::PromoCode* response) {
    grpc::Status tokenStatus = ValidateToken(request->token());
    if (!tokenStatus.ok()) {
        return tokenStatus;
    }
    grpc::ClientContext client_context;
    return promo_stub_->DeletePromoCode(&client_context, *request, response);
}

grpc::Status PromoServiceProxyImpl::GetPromoCodeById(grpc::ServerContext* context,
                                                     const promo::GetPromoCodeRequest* request,
                                                     promo::PromoCode* response) {
    grpc::Status tokenStatus = ValidateToken(request->token());
    if (!tokenStatus.ok()) {
        return tokenStatus;
    }
    grpc::ClientContext client_context;
    return promo_stub_->GetPromoCodeById(&client_context, *request, response);
}

grpc::Status PromoServiceProxyImpl::ListPromoCodes(grpc::ServerContext* context,
                                                   const promo::ListPromoCodesRequest* request,
                                                   promo::ListPromoCodesResponse* response) {
    grpc::Status tokenStatus = ValidateToken(request->token());
    if (!tokenStatus.ok()) {
        return tokenStatus;
    }
    grpc::ClientContext client_context;
    return promo_stub_->ListPromoCodes(&client_context, *request, response);
}

} // namespace gateway