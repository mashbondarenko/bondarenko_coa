#include "AuthServiceImpl.h"
#include "utils.h"
#include <iostream>
#include <pqxx/pqxx>

AuthServiceImpl::AuthServiceImpl(const std::string& conn_str) : conn_str_(conn_str) {
    try {
        pqxx::connection C(conn_str_);
        pqxx::work W(C);
        W.exec("CREATE TABLE IF NOT EXISTS users ("
               "id SERIAL PRIMARY KEY, "
               "login VARCHAR(50) UNIQUE NOT NULL, "
               "password VARCHAR(100) NOT NULL, "
               "email VARCHAR(100) NOT NULL, "
               "first_name VARCHAR(50), "
               "last_name VARCHAR(50), "
               "birth_date DATE, "
               "phone VARCHAR(20), "
               "created_at TIMESTAMP NOT NULL, "
               "updated_at TIMESTAMP NOT NULL"
               ");");
        W.commit();
    } catch (const std::exception &e) {
        std::cerr << "DB initialization error: " << e.what() << std::endl;
    }
}

grpc::Status AuthServiceImpl::RegisterUser(grpc::ServerContext* context, const auth::RegisterUserRequest* request,
                                             auth::User* response) {
    try {
        pqxx::connection C(conn_str_);
        pqxx::work W(C);

        std::string check_query = "SELECT COUNT(*) FROM users WHERE login = " + W.quote(request->login());
        pqxx::result R = W.exec(check_query);
        int count = R[0][0].as<int>();
        if(count > 0) {
            return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "Пользователь с таким логином уже существует");
        }
        std::string timestamp = current_timestamp();

        std::string insert_query = "INSERT INTO users (login, password, email, created_at, updated_at) VALUES ("
            + W.quote(request->login()) + ", "
            + W.quote(request->password()) + ", "
            + W.quote(request->email()) + ", "
            + W.quote(timestamp) + ", "
            + W.quote(timestamp) + ") RETURNING id;";
        pqxx::result R2 = W.exec(insert_query);
        W.commit();

        int user_id = R2[0][0].as<int>();

        response->set_id(user_id);
        response->set_login(request->login());
        response->set_email(request->email());
        response->set_created_at(timestamp);
        response->set_updated_at(timestamp);
    } catch (const std::exception &e) {
        std::cerr << "RegisterUser error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при регистрации пользователя");
    }
    return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::Login(grpc::ServerContext* context, const auth::LoginRequest* request,
                                      auth::LoginResponse* response) {
    try {
        pqxx::connection C(conn_str_);
        pqxx::work W(C);
        std::string query = "SELECT id, password, email, first_name, last_name, birth_date, phone, created_at, updated_at FROM users WHERE login = " + W.quote(request->login());
        pqxx::result R = W.exec(query);
        if(R.empty()){
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Пользователь не найден");
        }
        std::string stored_password = R[0]["password"].as<std::string>();
        if(stored_password != request->password()){
            return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Неверный пароль");
        }
        std::string token = request->login();
        response->set_token(token);

        auth::User* user = response->mutable_user();
        user->set_id(R[0]["id"].as<int>());
        user->set_login(request->login());
        user->set_email(R[0]["email"].as<std::string>());
        if(!R[0]["first_name"].is_null()) user->set_first_name(R[0]["first_name"].as<std::string>());
        if(!R[0]["last_name"].is_null()) user->set_last_name(R[0]["last_name"].as<std::string>());
        if(!R[0]["birth_date"].is_null()) user->set_birth_date(R[0]["birth_date"].as<std::string>());
        if(!R[0]["phone"].is_null()) user->set_phone(R[0]["phone"].as<std::string>());
        user->set_created_at(R[0]["created_at"].as<std::string>());
        user->set_updated_at(R[0]["updated_at"].as<std::string>());
    } catch (const std::exception &e) {
        std::cerr << "Login error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при входе");
    }
    return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::UpdateProfile(grpc::ServerContext* context, const auth::UpdateProfileRequest* request,
                                              auth::User* response) {
    std::string login = request->token();
    try {
        pqxx::connection C(conn_str_);
        pqxx::work W(C);
        std::string query = "SELECT id FROM users WHERE login = " + W.quote(login);
        pqxx::result R = W.exec(query);
        if(R.empty()){
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Пользователь не найден");
        }
        std::string timestamp = current_timestamp();

        std::string update_query = "UPDATE users SET "
            "first_name = " + W.quote(request->first_name()) + ", "
            "last_name = " + W.quote(request->last_name()) + ", "
            "birth_date = " + W.quote(request->birth_date()) + ", "
            "email = " + W.quote(request->email()) + ", "
            "phone = " + W.quote(request->phone()) + ", "
            "updated_at = " + W.quote(timestamp) +
            " WHERE login = " + W.quote(login) + ";";
        W.exec(update_query);
        W.commit();

        pqxx::connection C2(conn_str_);
        pqxx::work W2(C2);
        std::string select_query = "SELECT id, login, email, first_name, last_name, birth_date, phone, created_at, updated_at FROM users WHERE login = " + W2.quote(login);
        pqxx::result R2 = W2.exec(select_query);
        if(R2.empty()){
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Пользователь не найден после обновления");
        }
        response->set_id(R2[0]["id"].as<int>());
        response->set_login(R2[0]["login"].as<std::string>());
        response->set_email(R2[0]["email"].as<std::string>());
        if(!R2[0]["first_name"].is_null()) response->set_first_name(R2[0]["first_name"].as<std::string>());
        if(!R2[0]["last_name"].is_null()) response->set_last_name(R2[0]["last_name"].as<std::string>());
        if(!R2[0]["birth_date"].is_null()) response->set_birth_date(R2[0]["birth_date"].as<std::string>());
        if(!R2[0]["phone"].is_null()) response->set_phone(R2[0]["phone"].as<std::string>());
        response->set_created_at(R2[0]["created_at"].as<std::string>());
        response->set_updated_at(R2[0]["updated_at"].as<std::string>());
    } catch (const std::exception &e) {
        std::cerr << "UpdateProfile error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при обновлении профиля");
    }
    return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::GetProfile(grpc::ServerContext* context, const auth::GetProfileRequest* request,
                                           auth::User* response) {
    std::string login = request->token();
    try {
        pqxx::connection C(conn_str_);
        pqxx::work W(C);
        std::string query = "SELECT id, login, email, first_name, last_name, birth_date, phone, created_at, updated_at FROM users WHERE login = " + W.quote(login);
        pqxx::result R = W.exec(query);
        if(R.empty()){
            return grpc::Status(grpc::StatusCode::NOT_FOUND, "Пользователь не найден");
        }
        response->set_id(R[0]["id"].as<int>());
        response->set_login(R[0]["login"].as<std::string>());
        response->set_email(R[0]["email"].as<std::string>());
        if(!R[0]["first_name"].is_null()) response->set_first_name(R[0]["first_name"].as<std::string>());
        if(!R[0]["last_name"].is_null()) response->set_last_name(R[0]["last_name"].as<std::string>());
        if(!R[0]["birth_date"].is_null()) response->set_birth_date(R[0]["birth_date"].as<std::string>());
        if(!R[0]["phone"].is_null()) response->set_phone(R[0]["phone"].as<std::string>());
        response->set_created_at(R[0]["created_at"].as<std::string>());
        response->set_updated_at(R[0]["updated_at"].as<std::string>());
    } catch (const std::exception &e) {
        std::cerr << "GetProfile error: " << e.what() << std::endl;
        return grpc::Status(grpc::StatusCode::INTERNAL, "Ошибка при получении профиля");
    }
    return grpc::Status::OK;
}
