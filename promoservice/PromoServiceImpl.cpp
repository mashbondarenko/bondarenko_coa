#include "PromoServiceImpl.h"
#include "db_helper.cpp"
#include <iostream>

std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&now_c));
    return std::string(buf);
}

PromoServiceImpl::PromoServiceImpl(const std::string& db_conn_str)
    : conn_str_(db_conn_str)
{
    try {
        DBHelper db(conn_str_);
        db.execute([&](pqxx::work &W) -> int {
            W.exec(R"(
                CREATE TABLE IF NOT EXISTS promo_codes (
                    id SERIAL PRIMARY KEY,
                    title VARCHAR(255) NOT NULL,
                    description TEXT,
                    creator_id BIGINT NOT NULL,
                    discount DOUBLE PRECISION NOT NULL,
                    code VARCHAR(50) NOT NULL,
                    created_at TIMESTAMP NOT NULL,
                    updated_at TIMESTAMP NOT NULL
                );
            )");
            return 0;
        });
    } catch (const std::exception& e) {
        std::cerr << "DB init error in PromoService: " << e.what() << std::endl;
    }
}

grpc::Status PromoServiceImpl::CreatePromoCode(grpc::ServerContext* context,
                                               const promo::CreatePromoCodeRequest* request,
                                               promo::PromoCode* response)
{
    int64_t creator_id = 0;
    std::string now = current_timestamp();

    try {
        DBHelper db(conn_str_);
        int promo_id = db.execute([&](pqxx::work &W) -> int {
            std::string query = R"(
                INSERT INTO promo_codes (title, description, creator_id, discount, code, created_at, updated_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
                RETURNING id;
            )";
            pqxx::result r = W.exec_params(query,
                                           request->title(),
                                           request->description(),
                                           creator_id,
                                           request->discount(),
                                           request->code(),
                                           now,
                                           now);
            return r[0]["id"].as<int>();
        });

        response->set_id(promo_id);
        response->set_title(request->title());
        response->set_description(request->description());
        response->set_creator_id(creator_id);
        response->set_discount(request->discount());
        response->set_code(request->code());
        response->set_created_at(now);
        response->set_updated_at(now);
    } catch (const std::exception &e) {
        std::cerr << "CreatePromoCode error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при создании промокода");
    }

    return grpc::Status::OK;
}

grpc::Status PromoServiceImpl::UpdatePromoCode(grpc::ServerContext* context,
                                               const promo::UpdatePromoCodeRequest* request,
                                               promo::PromoCode* response)
{
    int64_t promo_id = request->promo_id();
    std::string now = current_timestamp();
    try {
        DBHelper db(conn_str_);
        pqxx::result result = db.execute([&](pqxx::work &W) -> pqxx::result {
            std::string query = R"(
                UPDATE promo_codes
                SET title = $1, description = $2, discount = $3, code = $4, updated_at = $5
                WHERE id = $6
                RETURNING id, title, description, creator_id, discount, code, created_at, updated_at;
            )";
            return W.exec_params(query,
                                 request->title(),
                                 request->description(),
                                 request->discount(),
                                 request->code(),
                                 now,
                                 promo_id);
        });
        if(result.empty()){
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Промокод не найден");
        }
        response->set_id(result[0]["id"].as<int64_t>());
        response->set_title(result[0]["title"].as<std::string>());
        response->set_description(result[0]["description"].as<std::string>());
        response->set_creator_id(result[0]["creator_id"].as<int64_t>());
        response->set_discount(result[0]["discount"].as<double>());
        response->set_code(result[0]["code"].as<std::string>());
        response->set_created_at(result[0]["created_at"].as<std::string>());
        response->set_updated_at(result[0]["updated_at"].as<std::string>());
    } catch (const std::exception &e) {
        std::cerr << "UpdatePromoCode error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при обновлении промокода");
    }

    return grpc::Status::OK;
}

grpc::Status PromoServiceImpl::DeletePromoCode(grpc::ServerContext* context,
                                               const promo::DeletePromoCodeRequest* request,
                                               promo::PromoCode* response)
{
    int64_t promo_id = request->promo_id();
    try {
        DBHelper db(conn_str_);
        pqxx::result select_result = db.execute([&](pqxx::work &W) -> pqxx::result {
            std::string select_query = R"(
                SELECT id, title, description, creator_id, discount, code, created_at, updated_at
                FROM promo_codes
                WHERE id = $1;
            )";
            return W.exec_params(select_query, promo_id);
        });
        if(select_result.empty()){
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Промокод не найден");
        }
        db.execute([&](pqxx::work &W) -> int {
            std::string delete_query = "DELETE FROM promo_codes WHERE id = $1;";
            W.exec_params(delete_query, promo_id);
            return 0;
        });
        response->set_id(select_result[0]["id"].as<int64_t>());
        response->set_title(select_result[0]["title"].as<std::string>());
        response->set_description(select_result[0]["description"].as<std::string>());
        response->set_creator_id(select_result[0]["creator_id"].as<int64_t>());
        response->set_discount(select_result[0]["discount"].as<double>());
        response->set_code(select_result[0]["code"].as<std::string>());
        response->set_created_at(select_result[0]["created_at"].as<std::string>());
        response->set_updated_at(select_result[0]["updated_at"].as<std::string>());
    } catch (const std::exception &e) {
        std::cerr << "DeletePromoCode error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при удалении промокода");
    }
    return grpc::Status::OK;
}grpc::Status PromoServiceImpl::GetPromoCodeById(grpc::ServerContext* context,
                                                const promo::GetPromoCodeRequest* request,
                                                promo::PromoCode* response)
{
    int64_t promo_id = request->promo_id();
    try {
        DBHelper db(conn_str_);
        pqxx::result r = db.execute([&](pqxx::work &W) -> pqxx::result {
            std::string query = R"(
                SELECT id, title, description, creator_id, discount, code, created_at, updated_at
                FROM promo_codes
                WHERE id = $1;
            )";
            return W.exec_params(query, promo_id);
        });
        if(r.empty()){
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Промокод не найден");
        }
        response->set_id(r[0]["id"].as<int64_t>());
        response->set_title(r[0]["title"].as<std::string>());
        response->set_description(r[0]["description"].as<std::string>());
        response->set_creator_id(r[0]["creator_id"].as<int64_t>());
        response->set_discount(r[0]["discount"].as<double>());
        response->set_code(r[0]["code"].as<std::string>());
        response->set_created_at(r[0]["created_at"].as<std::string>());
        response->set_updated_at(r[0]["updated_at"].as<std::string>());
    } catch (const std::exception &e) {
        std::cerr << "GetPromoCodeById error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при получении промокода");
    }
    return grpc::Status::OK;
}

grpc::Status PromoServiceImpl::ListPromoCodes(grpc::ServerContext* context,
                                              const promo::ListPromoCodesRequest* request,
                                              promo::ListPromoCodesResponse* response)
{
    int page = request->page();
    int page_size = request->page_size();
    if(page < 1) page = 1;
    if(page_size < 1) page_size = 10;
    int offset = (page - 1) * page_size;
    try {
        DBHelper db(conn_str_);
        int total_count = db.execute([&](pqxx::work &W) -> int {
            pqxx::result r = W.exec("SELECT COUNT(*) FROM promo_codes;");
            return r[0][0].as<int>();
        });
        pqxx::result r = db.execute([&](pqxx::work &W) -> pqxx::result {
            std::string query = R"(
                SELECT id, title, description, creator_id, discount, code, created_at, updated_at
                FROM promo_codes
                ORDER BY id DESC
                LIMIT $1 OFFSET $2;
            )";
            return W.exec_params(query, page_size, offset);
        });
        for (auto row : r) {
            promo::PromoCode* pc = response->add_promo_codes();
            pc->set_id(row["id"].as<int64_t>());
            pc->set_title(row["title"].as<std::string>());
            pc->set_description(row["description"].as<std::string>());
            pc->set_creator_id(row["creator_id"].as<int64_t>());
            pc->set_discount(row["discount"].as<double>());
            pc->set_code(row["code"].as<std::string>());
            pc->set_created_at(row["created_at"].as<std::string>());
            pc->set_updated_at(row["updated_at"].as<std::string>());
        }
        response->set_total_count(total_count);
        response->set_page(page);
        response->set_page_size(page_size);
    } catch (const std::exception &e) {
        std::cerr << "ListPromoCodes error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при получении списка промокодов");
    }
    return grpc::Status::OK;
}