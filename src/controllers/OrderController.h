#pragma once
#include <drogon/HttpController.h>

namespace blessimart::controllers {

class OrderController : public drogon::HttpController<OrderController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(OrderController::checkout,        "/api/orders",                    drogon::Post);
    ADD_METHOD_TO(OrderController::listOrders,      "/api/orders",                    drogon::Get);
    ADD_METHOD_TO(OrderController::getOrder,        "/api/orders/{id}",               drogon::Get);
    ADD_METHOD_TO(OrderController::sellerOrders,    "/api/seller/orders",             drogon::Get);
    ADD_METHOD_TO(OrderController::updateStatus,    "/api/orders/{id}/status",        drogon::Put);
    METHOD_LIST_END

    void checkout(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void listOrders(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void getOrder(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
    void sellerOrders(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void updateStatus(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
};

} // namespace blessimart::controllers
