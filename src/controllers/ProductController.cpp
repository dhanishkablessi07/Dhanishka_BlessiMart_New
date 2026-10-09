#include "ProductController.h"
#include "../db/Database.h"
#include "../utils/JsonHelper.h"
#include "../utils/SessionHelper.h"
#include <string>

namespace blessimart::controllers {

void ProductController::list(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    int page = 1;
    int limit = 12;
    std::string category_id = req->getParameter("category_id");
    std::string search = req->getParameter("search");
    
    auto pageStr = req->getParameter("page");
    auto limitStr = req->getParameter("limit");
    
    if (!pageStr.empty()) page = std::stoi(pageStr);
    if (!limitStr.empty()) limit = std::stoi(limitStr);
    if (page < 1) page = 1;
    if (limit < 1) limit = 12;

    int offset = (page - 1) * limit;

    std::string whereClause = "WHERE is_active = 1 ";
    std::vector<std::string> params;
    std::vector<std::string> countParams;

    if (!category_id.empty()) {
        whereClause += "AND category_id = ? ";
        params.push_back(category_id);
        countParams.push_back(category_id);
    }
    if (!search.empty()) {
        whereClause += "AND name LIKE ? ";
        std::string likeSearch = "%" + search + "%";
        params.push_back(likeSearch);
        countParams.push_back(likeSearch);
    }

    auto& db = db::Database::getInstance();
    std::string countQuery = "SELECT COUNT(*) as total FROM products " + whereClause;
    auto countRows = db.query(countQuery, countParams);
    int total = countRows.empty() ? 0 : std::stoi(countRows[0]["total"]);

    std::string query = "SELECT * FROM products " + whereClause + " LIMIT ? OFFSET ?";
    params.push_back(std::to_string(limit));
    params.push_back(std::to_string(offset));

    auto rows = db.query(query, params);
    
    Json::Value arr(Json::arrayValue);
    for (const auto& row : rows) {
        Json::Value item;
        for (const auto& [key, value] : row) {
            item[key] = value;
        }
        arr.append(item);
    }

    Json::Value respJson;
    respJson["products"] = arr;
    respJson["total"] = total;
    respJson["page"] = page;
    respJson["limit"] = limit;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
    callback(resp);
}

void ProductController::get(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    auto& db = db::Database::getInstance();
    std::string query = R"(
        SELECT p.*, c.name as category_name, u.username as seller_name, 
               (SELECT AVG(rating) FROM reviews WHERE product_id = p.id) as avg_rating 
        FROM products p 
        LEFT JOIN categories c ON p.category_id = c.id 
        LEFT JOIN users u ON p.seller_id = u.id 
        WHERE p.id = ?
    )";

    auto rows = db.query(query, {id});
    if (rows.empty()) {
        callback(utils::JsonHelper::error("Product not found", 404));
        return;
    }

    Json::Value item;
    for (const auto& [key, value] : rows[0]) {
        item[key] = value;
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(item);
    callback(resp);
}

void ProductController::create(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
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
    if (!json || !json->isMember("name") || !json->isMember("price") || !json->isMember("stock") || !json->isMember("category_id")) {
        callback(utils::JsonHelper::error("Missing required fields", 400));
        return;
    }

    std::string name = (*json)["name"].asString();
    std::string description = json->isMember("description") ? (*json)["description"].asString() : "";
    std::string price = (*json)["price"].asString();
    std::string stock = (*json)["stock"].asString();
    std::string category_id = (*json)["category_id"].asString();
    std::string seller_id = utils::SessionHelper::getUserId(req);
    std::string image_url = json->isMember("image_url") ? (*json)["image_url"].asString() : "";

    auto& db = db::Database::getInstance();
    if (db.execute("INSERT INTO products (seller_id, category_id, name, description, price, stock, image_url) VALUES (?, ?, ?, ?, ?, ?, ?)",
                   {seller_id, category_id, name, description, price, stock, image_url})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k201Created);
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to create product", 500));
    }
}

void ProductController::update(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    auto& db = db::Database::getInstance();
    auto rows = db.query("SELECT seller_id FROM products WHERE id = ?", {id});
    if (rows.empty()) {
        callback(utils::JsonHelper::error("Product not found", 404));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    if (rows[0]["seller_id"] != userId && !utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    auto json = req->getJsonObject();
    if (!json) {
        callback(utils::JsonHelper::error("Invalid JSON", 400));
        return;
    }

    std::string query = "UPDATE products SET ";
    std::vector<std::string> params;

    if (json->isMember("name")) { query += "name = ?, "; params.push_back((*json)["name"].asString()); }
    if (json->isMember("description")) { query += "description = ?, "; params.push_back((*json)["description"].asString()); }
    if (json->isMember("price")) { query += "price = ?, "; params.push_back((*json)["price"].asString()); }
    if (json->isMember("stock")) { query += "stock = ?, "; params.push_back((*json)["stock"].asString()); }
    if (json->isMember("category_id")) { query += "category_id = ?, "; params.push_back((*json)["category_id"].asString()); }
    if (json->isMember("image_url")) { query += "image_url = ?, "; params.push_back((*json)["image_url"].asString()); }
    if (json->isMember("is_active")) { query += "is_active = ?, "; params.push_back((*json)["is_active"].asString()); }

    if (params.empty()) {
        callback(utils::JsonHelper::error("No fields to update", 400));
        return;
    }

    query.pop_back(); query.pop_back();
    query += " WHERE id = ?";
    params.push_back(id);

    if (db.execute(query, params)) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to update product", 500));
    }
}

void ProductController::remove(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    auto& db = db::Database::getInstance();
    auto rows = db.query("SELECT seller_id FROM products WHERE id = ?", {id});
    if (rows.empty()) {
        callback(utils::JsonHelper::error("Product not found", 404));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);
    if (rows[0]["seller_id"] != userId && !utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    if (db.execute("UPDATE products SET is_active = 0 WHERE id = ?", {id})) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error("Failed to delete product", 500));
    }
}
void ProductController::myProducts(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback
) {
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error("Unauthorized", 401));
        return;
    }

    std::string role = utils::SessionHelper::getRole(req);

    if (role != "seller" && role != "admin") {
        callback(utils::JsonHelper::error("Forbidden", 403));
        return;
    }

    std::string userId = utils::SessionHelper::getUserId(req);

    auto& db = db::Database::getInstance();

    std::string query = R"(
        SELECT p.*, c.name AS category_name
        FROM products p
        LEFT JOIN categories c ON p.category_id = c.id
        WHERE p.seller_id = ? AND p.is_active = 1
        ORDER BY p.created_at DESC
    )";

    auto rows = db.query(query, {userId});

    Json::Value arr(Json::arrayValue);

    for (const auto& row : rows) {
        Json::Value item;

        for (const auto& [key, value] : row) {
            item[key] = value;
        }

        arr.append(item);
    }

    Json::Value response;
    response["products"] = arr;

    auto resp =
        drogon::HttpResponse::newHttpJsonResponse(response);

    callback(resp);
}
} // namespace blessimart::controllers
