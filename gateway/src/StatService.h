#ifndef CLICK_AND_COMMENT_SERVICE_H
#define CLICK_AND_COMMENT_SERVICE_H

#include <string>
#include "KafkaProd.h"

class StatService {
public:
    static void SendClickEvent(const std::string& user_id, const std::string& promo_id);
    static void SendCommentEvent(const std::string& user_id, const std::string& promo_id, const std::string& comment);
};

#endif // CLICK_AND_COMMENT_SERVICE_H