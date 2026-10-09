#include "ChatController.h"
#include <drogon/drogon.h>
#include <algorithm>
#include <cctype>
#include <string>

namespace blessimart::controllers {

void ChatController::chat(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    if (!json || !json->isMember("message")) {
        Json::Value result;
        result["message"] = "Please enter a message.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string message = (*json)["message"].asString();

    // Input limit
    if (message.empty()) {
        Json::Value result;
        result["message"] = "Please enter a message.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    if (message.length() > 500) {
        Json::Value result;
        result["message"] = "Message is too long. Please keep it under 500 characters.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    // Convert message to lowercase for FAQ matching
    std::string text = message;

    std::transform(
        text.begin(),
        text.end(),
        text.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    std::string reply;

    // BlessiMart product-domain FAQ fallback
    if (text.find("product") != std::string::npos ||
        text.find("products") != std::string::npos) {

        reply =
            "You can browse BlessiMart products from the Products page. "
            "You can search by keyword and filter products by category.";
    }
    else if (text.find("cart") != std::string::npos) {

        reply =
            "You can add products to your cart, update quantities, "
            "and remove products from the Cart page.";
    }
    else if (text.find("order") != std::string::npos) {

        reply =
            "After checkout, you can view your orders from the Orders page "
            "and track the current order status.";
    }
    else if (text.find("review") != std::string::npos ||
             text.find("rating") != std::string::npos) {

        reply =
            "You can submit a 1 to 5 star review for a product after "
            "your completed order has been delivered.";
    }
    else if (text.find("seller") != std::string::npos) {

        reply =
            "Sellers can create, edit and delete product listings "
            "and manage incoming orders from the Seller dashboard.";
    }
    else if (text.find("login") != std::string::npos ||
             text.find("register") != std::string::npos) {

        reply =
            "Use the Login page to sign in. New buyers and sellers "
            "can register using the Register page.";
    }
    else if (text.find("price") != std::string::npos) {

        reply =
            "Product prices are displayed on the product cards "
            "and on the Product Details page.";
    }
    else {
        reply =
            "I can help with BlessiMart products, cart, orders, "
            "reviews, ratings, sellers and account-related questions.";
    }

    Json::Value result;
    result["message"] = reply;
    result["source"] = "blessimart-assistant";

    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    callback(resp);
}

}