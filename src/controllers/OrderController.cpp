#include "OrderController.h"
#include "../db/Database.h"
#include "../utils/JsonHelper.h"
#include "../utils/SessionHelper.h"
#include <string>
#include <chrono>
#include <random>

namespace blessimart::controllers {

static std::string generateOrderNumber() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return "ORD-" + std::to_string(ms);
}

void OrderController::checkout(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isLoggedIn(req) || utils::SessionHelper::getRole(req) != "buyer") {
        callback(utils::JsonHelper::error("Only buyers can checkout", 403));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !json->isMember("shipping_name") || !json->isMember("shipping_phone") || !json->isMember("shipping_address")) {
        callback(utils::JsonHelper::error("Missing shipping info", 400));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    std::string s_name = (*json)["shipping_name"].asString();
    std::string s_phone = (*json)["shipping_phone"].asString();
    std::string shippingAddr = (*json)["shipping_address"].asString();

    auto& db = db::Database::getInstance();

    // Get cart items with seller_id for order_items
    auto cartItems = db.query(
        "SELECT c.product_id, c.quantity, p.price, p.stock, p.seller_id "
        "FROM cart_items c JOIN products p ON c.product_id = p.id WHERE c.user_id = ?",
        {userId}
    );
    if (cartItems.empty()) {
        callback(utils::JsonHelper::error("Cart is empty", 400));
        return;
    }

    double totalAmount = 0.0;
    for (const auto& item : cartItems) {
        int qty = std::stoi(item.at("quantity"));
        int stock = std::stoi(item.at("stock"));
        if (qty > stock) {
            callback(utils::JsonHelper::error("Insufficient stock for product " + item.at("product_id"), 400));
            return;
        }
        totalAmount += qty * std::stod(item.at("price"));
    }

    try {
        db.execute("BEGIN");
        std::string orderNum = generateOrderNumber();
        db.execute(
            "INSERT INTO orders (user_id, order_number, total_amount, order_status, shipping_address, shipping_name, shipping_phone) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)",
            {userId, orderNum, std::to_string(totalAmount), "Processing", shippingAddr, s_name, s_phone}
        );
        auto orderId = std::to_string(db.lastInsertRowId());

        for (const auto& item : cartItems) {
            double unitPrice = std::stod(item.at("price"));
            int qty = std::stoi(item.at("quantity"));
            double subtotal = unitPrice * qty;
            db.execute(
                "INSERT INTO order_items (order_id, product_id, seller_id, quantity, unit_price, subtotal) VALUES (?, ?, ?, ?, ?, ?)",
                {orderId, item.at("product_id"), item.at("seller_id"), item.at("quantity"),
                 std::to_string(unitPrice), std::to_string(subtotal)}
            );
            db.execute("UPDATE products SET stock = stock - ? WHERE id = ?",
                       {item.at("quantity"), item.at("product_id")});
        }

        db.execute("DELETE FROM cart_items WHERE user_id = ?", {userId});
        db.execute("COMMIT");

        Json::Value respJson;
        respJson["order_number"] = orderNum;
        respJson["message"] = "Checkout successful";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k201Created);
        callback(resp);
    } catch (const std::exception& e) {
        try { db.execute("ROLLBACK"); } catch (...) {}
        callback(utils::JsonHelper::error(std::string("Checkout failed: ") + e.what(), 500));
    }
}

void OrderController::listOrders(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    auto& db = db::Database::getInstance();
    std::string role = utils::SessionHelper::getRole(req);
    std::string userId = utils::SessionHelper::getUserId(req);

    std::vector<std::map<std::string, std::string>> rows;
    if (role == "admin") {
        rows = db.query("SELECT * FROM orders ORDER BY created_at DESC", {});
    } else {
        rows = db.query("SELECT * FROM orders WHERE user_id = ? ORDER BY created_at DESC", {userId});
    }

    Json::Value arr(Json::arrayValue);
    for (const auto& row : rows) {
        Json::Value item;
        for (const auto& [k, v] : row) item[k] = v;
        arr.append(item);
    }
    auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
    callback(resp);
}

void OrderController::getOrder(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    auto& db = db::Database::getInstance();
    auto orderRows = db.query("SELECT * FROM orders WHERE id = ?", {id});
    if (orderRows.empty()) {
        callback(utils::JsonHelper::error("Order not found", 404));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    std::string role = utils::SessionHelper::getRole(req);

    if (role == "buyer" && orderRows[0]["user_id"] != userId) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    Json::Value result;
    for (const auto& [k, v] : orderRows[0]) result[k] = v;

    auto itemsRows = db.query("SELECT oi.*, p.name FROM order_items oi JOIN products p ON oi.product_id = p.id WHERE oi.order_id = ?", {id});
    Json::Value itemsArr(Json::arrayValue);
    for (const auto& row : itemsRows) {
        Json::Value it;
        for (const auto& [k, v] : row) it[k] = v;
        itemsArr.append(it);
    }
    result["items"] = itemsArr;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    callback(resp);
}

void OrderController::sellerOrders(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!utils::SessionHelper::isLoggedIn(req) || utils::SessionHelper::getRole(req) != "seller") {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    std::string sellerId = utils::SessionHelper::getUserId(req);
    auto& db = db::Database::getInstance();

    std::string query = R"(
        SELECT DISTINCT o.* FROM orders o
        JOIN order_items oi ON o.id = oi.order_id
        JOIN products p ON oi.product_id = p.id
        WHERE p.seller_id = ?
        ORDER BY o.created_at DESC
    )";

    auto rows = db.query(query, {sellerId});
    Json::Value arr(Json::arrayValue);
    for (const auto& row : rows) {
        Json::Value item;
        for (const auto& [k, v] : row) item[k] = v;
        arr.append(item);
    }
    auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
    callback(resp);
}

void OrderController::updateStatus(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    std::string role = utils::SessionHelper::getRole(req);
    if (role != "seller" && role != "admin") {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !json->isMember("status")) {
        callback(utils::JsonHelper::error("Missing status", 400));
        return;
    }

    std::string status = (*json)["status"].asString();
    if (status != "Processing" && status != "Shipped" && status != "Delivered" && status != "Cancelled") {
        callback(utils::JsonHelper::error("Invalid status", 400));
        return;
    }

    auto& db = db::Database::getInstance();
    try {
        db.execute("UPDATE orders SET order_status = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?", {status, id});
        Json::Value respJson;
        respJson["message"] = "Status updated";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
        callback(resp);
    } catch (const std::exception& e) {
        callback(utils::JsonHelper::error(std::string("Failed to update status: ") + e.what(), 500));
    }
}

} // namespace blessimart::controllers
