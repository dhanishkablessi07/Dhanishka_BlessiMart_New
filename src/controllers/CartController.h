#pragma once
#include <drogon/HttpController.h>

namespace blessimart::controllers {

class CartController : public drogon::HttpController<CartController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CartController::getCart,     "/api/cart",                drogon::Get);
    ADD_METHOD_TO(CartController::addItem,     "/api/cart/items",                drogon::Post);
    ADD_METHOD_TO(CartController::removeItem,  "/api/cart/{productId}",    drogon::Delete);
    ADD_METHOD_TO(CartController::clearCart,   "/api/cart/clear",          drogon::Delete);
    METHOD_LIST_END

    void getCart(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void addItem(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void removeItem(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& productId);
    void clearCart(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
};

} // namespace blessimart::controllers
