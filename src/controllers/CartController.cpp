#include "CartController.h"
#include "../db/Database.h"
#include "../utils/JsonHelper.h"
#include "../utils/SessionHelper.h"
#include <string>

namespace blessimart::controllers {

void CartController::getCart(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    auto& db = db::Database::getInstance();
    
    std::string query = R"(
        SELECT c.product_id, c.quantity, p.name, p.price, p.image_url 
        FROM cart_items c 
        JOIN products p ON c.product_id = p.id 
        WHERE c.user_id = ?
    )";
    
    auto rows = db.query(query, {userId});
    
    Json::Value arr(Json::arrayValue);
    double grandTotal = 0.0;

    for (const auto& row : rows) {
        Json::Value item;
        item["product_id"] = row.at("product_id");
        item["quantity"] = std::stoi(row.at("quantity"));
        item["name"] = row.at("name");
        item["price"] = std::stod(row.at("price"));
        item["image_url"] = row.at("image_url");
        
        double lineTotal = std::stoi(row.at("quantity")) * std::stod(row.at("price"));
        item["line_total"] = lineTotal;
        grandTotal += lineTotal;
        
        arr.append(item);
    }

    Json::Value respJson;
    respJson["items"] = arr;
    respJson["grand_total"] = grandTotal;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
    callback(resp);
}

void CartController::addItem(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !json->isMember("product_id") || !json->isMember("quantity")) {
        callback(utils::JsonHelper::error("Missing product_id or quantity", 400));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    std::string productId = (*json)["product_id"].asString();
    std::string quantity = (*json)["quantity"].asString();

    auto& db = db::Database::getInstance();
    
    auto pRows = db.query("SELECT is_active, stock FROM products WHERE id = ?", {productId});
    if (pRows.empty() || pRows[0]["is_active"] == "0") {
        callback(utils::JsonHelper::error("Product not available", 400));
        return;
    }
    
    if (std::stoi(pRows[0]["stock"]) < std::stoi(quantity)) {
        callback(utils::JsonHelper::error("Not enough stock", 400));
        return;
    }

    if (db.execute("INSERT OR REPLACE INTO cart_items (user_id, product_id, quantity) VALUES (?, ?, ?)", {userId, productId, quantity})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to add to cart", 500));
    }
}

void CartController::removeItem(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& productId) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    auto& db = db::Database::getInstance();
    
    if (db.execute("DELETE FROM cart_items WHERE user_id = ? AND product_id = ?", {userId, productId})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to remove item", 500));
    }
}

void CartController::clearCart(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    auto& db = db::Database::getInstance();
    
    if (db.execute("DELETE FROM cart_items WHERE user_id = ?", {userId})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to clear cart", 500));
    }
}

} // namespace blessimart::controllers
