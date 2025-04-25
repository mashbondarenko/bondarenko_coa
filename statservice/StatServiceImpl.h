#ifndef STAT_SERVICE_IMPL_H
#define STAT_SERVICE_IMPL_H

#include <string>
#include <grpcpp/grpcpp.h>
#include "stat.grpc.pb.h"

class StatServiceImpl final : public promo::StatService::Service {
public:
    explicit StatServiceImpl(const std::string& db_conn_str);

    grpc::Status GetAllComments(grpc::ServerContext* context,
                                 const promo::GetAllComments* request,
                                 promo::GetAllComments* response) override;

private:
    std::string conn_str_;
};

#endif // PROMO_SERVICE_IMPL_H