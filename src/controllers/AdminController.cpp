#include "AdminController.h"
#include "../db/Database.h"
#include "../utils/JsonHelper.h"
#include "../utils/SessionHelper.h"
#include <string>

namespace blessimart::controllers {

void AdminController::listUsers(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto& db = db::Database::getInstance();
    auto rows = db.query("SELECT id, username, email, role, full_name, status, created_at FROM users", {});
    
    Json::Value arr(Json::arrayValue);
    for (const auto& row : rows) {
        Json::Value item;
        for (const auto& [k, v] : row) item[k] = v;
        arr.append(item);
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
    callback(resp);
}

void AdminController::setStatus(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    if (!utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !json->isMember("status")) {
        callback(utils::JsonHelper::error("Missing status", 400));
        return;
    }

    std::string status = (*json)["status"].asString();
    if (status != "active" && status != "suspended") {
        callback(utils::JsonHelper::error("Invalid status", 400));
        return;
    }

    auto& db = db::Database::getInstance();
    if (db.execute("UPDATE users SET status = ? WHERE id = ?", {status, id})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to update status", 500));
    }
}

void AdminController::stats(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto& db = db::Database::getInstance();
    
    int total_users = 0;
    auto uRows = db.query("SELECT COUNT(*) as c FROM users", {});
    if (!uRows.empty()) total_users = std::stoi(uRows[0]["c"]);

    int total_products = 0;
    auto pRows = db.query("SELECT COUNT(*) as c FROM products", {});
    if (!pRows.empty()) total_products = std::stoi(pRows[0]["c"]);

    int total_orders = 0;
    auto oRows = db.query("SELECT COUNT(*) as c FROM orders", {});
    if (!oRows.empty()) total_orders = std::stoi(oRows[0]["c"]);

    double total_revenue = 0.0;
    auto rRows = db.query("SELECT SUM(total_amount) as s FROM orders", {});
    if (!rRows.empty() && rRows[0]["s"] != "") total_revenue = std::stod(rRows[0]["s"]);

    Json::Value statsJson;
    statsJson["total_users"] = total_users;
    statsJson["total_products"] = total_products;
    statsJson["total_orders"] = total_orders;
    statsJson["total_revenue"] = total_revenue;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(statsJson);
    callback(resp);
}
void AdminController::listOrders(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback
) {
    if (!utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto& db = db::Database::getInstance();

    auto rows = db.query(
        "SELECT id, order_number, user_id, total_amount, order_status, "
        "payment_method, payment_status, shipping_name, shipping_phone, "
        "shipping_address, created_at "
        "FROM orders ORDER BY id DESC",
        {}
    );

    Json::Value arr(Json::arrayValue);

    for (const auto& row : rows) {
        Json::Value item;

        for (const auto& [k, v] : row) {
            item[k] = v;
        }

        arr.append(item);
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
    callback(resp);
}

} // namespace blessimart::controllers
