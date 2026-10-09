#include "CategoryController.h"
#include "../db/Database.h"
#include "../utils/JsonHelper.h"
#include "../utils/SessionHelper.h"

namespace blessimart::controllers {

void CategoryController::getAll(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto& db = db::Database::getInstance();
    auto rows = db.query("SELECT * FROM categories ORDER BY name", {});
    
    Json::Value arr(Json::arrayValue);
    for (const auto& row : rows) {
        Json::Value item;
        for (const auto& [key, value] : row) {
            item[key] = value;
        }
        arr.append(item);
    }
    
    auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
    callback(resp);
}

void CategoryController::create(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !json->isMember("name")) {
        callback(utils::JsonHelper::error("Missing name field", 400));
        return;
    }

    std::string name = (*json)["name"].asString();
    std::string description = json->isMember("description") ? (*json)["description"].asString() : "";
    std::string icon = json->isMember("icon") ? (*json)["icon"].asString() : "";

    auto& db = db::Database::getInstance();
    if (db.execute("INSERT INTO categories (name, description, icon) VALUES (?, ?, ?)", {name, description, icon})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k201Created);
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to create category", 500));
    }
}

void CategoryController::update(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    if (!utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto json = req->getJsonObject();
    if (!json) {
        callback(utils::JsonHelper::error("Invalid JSON", 400));
        return;
    }

    auto& db = db::Database::getInstance();
    std::string query = "UPDATE categories SET ";
    std::vector<std::string> params;
    
    if (json->isMember("name")) {
        query += "name = ?, ";
        params.push_back((*json)["name"].asString());
    }
    if (json->isMember("description")) {
        query += "description = ?, ";
        params.push_back((*json)["description"].asString());
    }
    if (json->isMember("icon")) {
        query += "icon = ?, ";
        params.push_back((*json)["icon"].asString());
    }

    if (params.empty()) {
        callback(utils::JsonHelper::error("No fields to update", 400));
        return;
    }

    query.pop_back(); query.pop_back(); // remove trailing ", "
    query += " WHERE id = ?";
    params.push_back(id);

    if (db.execute(query, params)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to update category", 500));
    }
}

void CategoryController::remove(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    if (!utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto& db = db::Database::getInstance();
    if (db.execute("DELETE FROM categories WHERE id = ?", {id})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to delete category", 500));
    }
}

} // namespace blessimart::controllers
