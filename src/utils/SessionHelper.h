#pragma once
#include <drogon/HttpRequest.h>
#include <string>
#include <cstdint>

namespace blessimart {
namespace utils {

class SessionHelper {
public:
    static int64_t getSessionUserId(const drogon::HttpRequestPtr& req) {
        auto session = req->session();
        if (!session) return -1;
        auto val = session->getOptional<int64_t>("user_id");
        return val ? *val : -1;
    }

    // String form for SQL params
    static std::string getUserId(const drogon::HttpRequestPtr& req) {
        return std::to_string(getSessionUserId(req));
    }

    static std::string getRole(const drogon::HttpRequestPtr& req) {
        auto session = req->session();
        if (!session) return "";
        auto val = session->getOptional<std::string>("role");
        return val ? *val : "";
    }

    static void setSession(const drogon::HttpRequestPtr& req,
                           int64_t userId,
                           const std::string& role) {
        auto session = req->session();
        if (session) {
            session->insert("user_id", userId);
            session->insert("role", role);
        }
    }

    static void clearSession(const drogon::HttpRequestPtr& req) {
        auto session = req->session();
        if (session) session->clear();
    }

    static bool isLoggedIn(const drogon::HttpRequestPtr& req) {
        return getSessionUserId(req) > 0;
    }

    static bool hasRole(const drogon::HttpRequestPtr& req, const std::string& role) {
        return getRole(req) == role;
    }

    static bool isAdmin(const drogon::HttpRequestPtr& req) {
        return hasRole(req, "admin");
    }
};

} // namespace utils
} // namespace blessimart
