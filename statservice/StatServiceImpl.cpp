#include "StatServiceImpl.h"
#include <iostream>

std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&now_c));
    return std::string(buf);
}

