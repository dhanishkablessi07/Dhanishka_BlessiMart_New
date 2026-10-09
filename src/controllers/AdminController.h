#pragma once
#include <drogon/HttpController.h>

namespace blessimart::controllers {

class AdminController : public drogon::HttpController<AdminController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AdminController::listUsers,   "/api/admin/users",           drogon::Get);
    ADD_METHOD_TO(AdminController::setStatus,   "/api/admin/users/{id}/status", drogon::Put);
    ADD_METHOD_TO(AdminController::stats,       "/api/admin/stats",           drogon::Get);
    ADD_METHOD_TO(AdminController::listOrders,  "/api/admin/orders",          drogon::Get);
    METHOD_LIST_END

    void listUsers(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void setStatus(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
    void stats(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void listOrders(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
};

} // namespace blessimart::controllers
