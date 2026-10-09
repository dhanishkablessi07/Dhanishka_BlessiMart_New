#include "ReviewController.h"
#include "../db/Database.h"
#include "../utils/JsonHelper.h"
#include "../utils/SessionHelper.h"
#include <string>

namespace blessimart::controllers {

void ReviewController::create(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback
) {
    // Only logged-in buyers can create reviews
    if (!utils::SessionHelper::isLoggedIn(req) ||
        utils::SessionHelper::getRole(req) != "buyer") {
        callback(utils::JsonHelper::error(
            "Only buyers can leave reviews", 403));
        return;
    }

    auto json = req->getJsonObject();

    if (!json ||
        !json->isMember("product_id") ||
        !json->isMember("rating")) {
        callback(utils::JsonHelper::error(
            "Missing product_id or rating", 400));
        return;
    }

    std::string userId =
        utils::SessionHelper::getUserId(req);

    std::string productId =
        (*json)["product_id"].asString();

    int rating =
        (*json)["rating"].asInt();

    std::string comment =
        json->isMember("comment")
            ? (*json)["comment"].asString()
            : "";

    // Rating must be between 1 and 5
    if (rating < 1 || rating > 5) {
        callback(utils::JsonHelper::error(
            "Rating must be between 1 and 5", 400));
        return;
    }

    auto& db = db::Database::getInstance();

    // Buyer must have a delivered order containing this product
    auto purchased = db.query(
        "SELECT oi.id "
        "FROM order_items oi "
        "JOIN orders o ON oi.order_id = o.id "
        "WHERE o.user_id = ? "
        "AND oi.product_id = ? "
        "AND o.order_status = 'Delivered' "
        "LIMIT 1",
        {userId, productId}
    );

    if (purchased.empty()) {
        callback(utils::JsonHelper::error(
            "You can review this product only after delivery",
            403));
        return;
    }

    // Add review
    if (db.execute(
        "INSERT OR IGNORE INTO reviews "
        "(product_id, user_id, rating, comment) "
        "VALUES (?, ?, ?, ?)",
        {
            productId,
            userId,
            std::to_string(rating),
            comment
        }
    )) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k201Created);
        callback(resp);
    } else {
        callback(utils::JsonHelper::error(
            "Failed to add review", 500));
    }
}

void ReviewController::listForProduct(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    const std::string& id
) {
    auto& db = db::Database::getInstance();

    std::string query = R"(
        SELECT r.*, u.username
        FROM reviews r
        JOIN users u ON r.user_id = u.id
        WHERE r.product_id = ?
        ORDER BY r.created_at DESC
    )";

    auto rows = db.query(query, {id});

    Json::Value arr(Json::arrayValue);

    for (const auto& row : rows) {
        Json::Value item;

        for (const auto& [k, v] : row) {
            item[k] = v;
        }

        arr.append(item);
    }

    auto resp =
        drogon::HttpResponse::newHttpJsonResponse(arr);

    callback(resp);
}

void ReviewController::remove(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    const std::string& id
) {
    // User must be logged in
    if (!utils::SessionHelper::isLoggedIn(req)) {
        callback(utils::JsonHelper::error(
            "Unauthorized", 401));
        return;
    }

    auto& db = db::Database::getInstance();

    auto rows = db.query(
        "SELECT user_id FROM reviews WHERE id = ?",
        {id}
    );

    if (rows.empty()) {
        callback(utils::JsonHelper::error(
            "Review not found", 404));
        return;
    }

    std::string userId =
        utils::SessionHelper::getUserId(req);

    // Only review owner or admin can delete
    if (rows[0]["user_id"] != userId &&
        !utils::SessionHelper::isAdmin(req)) {
        callback(utils::JsonHelper::error(
            "Forbidden", 403));
        return;
    }

    if (db.execute(
        "DELETE FROM reviews WHERE id = ?",
        {id}
    )) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        callback(resp);
    } else {
        callback(utils::JsonHelper::error(
            "Failed to delete review", 500));
    }
}

} // namespace blessimart::controllers