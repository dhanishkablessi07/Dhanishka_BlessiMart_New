#pragma once
#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include <string>
#include <vector>

namespace blessimart {
namespace utils {

// ── Free-function helpers (take an existing resp ptr) ──────────────────────

inline void sendJson(const drogon::HttpResponsePtr& resp,
                     drogon::HttpStatusCode code,
                     const Json::Value& body) {
    resp->setStatusCode(code);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    Json::StreamWriterBuilder wb;
    resp->setBody(Json::writeString(wb, body));
}

inline void ok(const drogon::HttpResponsePtr& resp, const Json::Value& body) {
    sendJson(resp, drogon::k200OK, body);
}

inline void created(const drogon::HttpResponsePtr& resp, const Json::Value& body) {
    sendJson(resp, drogon::k201Created, body);
}

inline void err(const drogon::HttpResponsePtr& resp,
                drogon::HttpStatusCode code,
                const std::string& message) {
    Json::Value j;
    j["error"] = message;
    sendJson(resp, code, j);
}



// ── Static factory helpers (return a new HttpResponsePtr) ──────────────────
// Controllers call: callback(utils::JsonHelper::error("msg", 400));

struct JsonHelper {
    static std::vector<std::string> missingFields(const Json::Value& body,
                                                  const std::vector<std::string>& required) {
        std::vector<std::string> missing;
        for (const auto& f : required) {
            if (!body.isMember(f) || body[f].asString().empty())
                missing.push_back(f);
        }
        return missing;
    }

    static drogon::HttpResponsePtr error(const std::string& message, int statusCode) {
        Json::Value j;
        j["error"] = message;
        Json::StreamWriterBuilder wb;
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        resp->setBody(Json::writeString(wb, j));
        resp->setStatusCode(static_cast<drogon::HttpStatusCode>(statusCode));
        return resp;
    }

    static drogon::HttpResponsePtr success(const Json::Value& body,
                                            int statusCode = 200) {
        Json::StreamWriterBuilder wb;
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        resp->setBody(Json::writeString(wb, body));
        resp->setStatusCode(static_cast<drogon::HttpStatusCode>(statusCode));
        return resp;
    }
};

} // namespace utils
} // namespace blessimart
