#include "AuthController.h"
#include "../db/Database.h"
#include "../utils/JsonHelper.h"
#include "../utils/SessionHelper.h"
#include "../utils/Crypto.h"

namespace blessimart {
namespace controllers {

void AuthController::registerUser(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json) {
        callback(utils::JsonHelper::error("Invalid JSON", 400));
        return;
    }

    std::string username = (*json)["username"].asString();
    std::string email = (*json)["email"].asString();
    std::string password = (*json)["password"].asString();
    std::string full_name = (*json)["full_name"].asString();
    std::string role = (*json)["role"].asString();

    if (username.empty() || email.empty() || password.empty() || full_name.empty() || role.empty()) {
        callback(utils::JsonHelper::error("Missing required fields", 400));
        return;
    }

    if (role != "buyer" && role != "seller") {
        callback(utils::JsonHelper::error("Role must be buyer or seller", 400));
        return;
    }

    auto& db = db::Database::getInstance();
    auto rows = db.query("SELECT id FROM users WHERE username = ? OR email = ?", {username, email});
    if (!rows.empty()) {
        callback(utils::JsonHelper::error("Username or email already exists", 409));
        return;
    }

    std::string salt = utils::Crypto::generateSalt();
    std::string hash = utils::Crypto::hashPassword(password, salt);

    try {
        db.execute(
            "INSERT INTO users (username, email, password_hash, salt, full_name, role) VALUES (?, ?, ?, ?, ?, ?)",
            {username, email, hash, salt, full_name, role}
        );
    } catch (const std::exception& e) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k500InternalServerError);
        Json::Value j; j["error"] = std::string(e.what());
        resp->setBody(Json::writeString(Json::StreamWriterBuilder{}, j));
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(resp);
        return;
    }

    auto newRows = db.query("SELECT id FROM users WHERE username = ?", {username});
    std::string userId = newRows.empty() ? "" : newRows[0]["id"];

    Json::Value respJson;
    respJson["message"] = "registered";
    respJson["userId"] = userId;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
    resp->setStatusCode(drogon::k201Created);
    callback(resp);
}

void AuthController::login(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json) {
        callback(utils::JsonHelper::error("Invalid JSON", 400));
        return;
    }

    std::string email = (*json)["email"].asString();
    std::string password = (*json)["password"].asString();

    if (email.empty() || password.empty()) {
        callback(utils::JsonHelper::error("Missing email or password", 400));
        return;
    }

    auto& db = db::Database::getInstance();
    auto rows = db.query("SELECT id, username, email, password_hash, salt, role, full_name FROM users WHERE email = ?", {email});
    if (rows.empty()) {
        callback(utils::JsonHelper::error("Invalid email or password", 401));
        return;
    }

    const auto& user = rows[0];
    if (!utils::Crypto::verifyPassword(password, user.at("salt"), user.at("password_hash"))) {
        callback(utils::JsonHelper::error("Invalid email or password", 401));
        return;
    }

    utils::SessionHelper::setSession(req, std::stoll(user.at("id")), user.at("role"));

    Json::Value respJson;
    respJson["message"] = "ok";
    Json::Value userJson;
    userJson["id"] = user.at("id");
    userJson["username"] = user.at("username");
    userJson["email"] = user.at("email");
    userJson["role"] = user.at("role");
    userJson["full_name"] = user.at("full_name");
    respJson["user"] = userJson;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
    callback(resp);
}

void AuthController::logout(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    utils::SessionHelper::clearSession(req);
    Json::Value respJson;
    respJson["message"] = "logged out";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
    callback(resp);
}

void AuthController::me(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    auto& db = db::Database::getInstance();
    auto rows = db.query("SELECT id, username, email, role, full_name FROM users WHERE id = ?", {userId});

    if (rows.empty()) {
        callback(utils::JsonHelper::error("User not found", 404));
        return;
    }

    Json::Value userJson;
    userJson["id"] = rows[0]["id"];
    userJson["username"] = rows[0]["username"];
    userJson["email"] = rows[0]["email"];
    userJson["role"] = rows[0]["role"];
    userJson["full_name"] = rows[0]["full_name"];

    auto resp = drogon::HttpResponse::newHttpJsonResponse(userJson);
    callback(resp);
}

} // namespace controllers
} // namespace blessimart
