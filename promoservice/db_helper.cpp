#ifndef DB_HELPER_H
#define DB_HELPER_H

#include <pqxx/pqxx>
#include <string>
#include <functional>

class DBHelper {
public:
    explicit DBHelper(const std::string& conn_str) : conn_str_(conn_str) {}

    template<typename Func>
    auto execute(Func f) -> decltype(f(std::declval<pqxx::work&>())) {
        try {
            pqxx::connection C(conn_str_);
            pqxx::work W(C);
            auto result = f(W);
            W.commit();
            return result;
        } catch (const std::exception &e) {
            throw std::runtime_error(std::string("DBHelper error: ") + e.what());
        }
    }

private:
    std::string conn_str_;
};

#endif // DB_HELPER_H