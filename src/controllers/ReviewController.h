#pragma once
#include <drogon/HttpController.h>

namespace blessimart::controllers {

class ReviewController : public drogon::HttpController<ReviewController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ReviewController::create,     "/api/reviews",                   drogon::Post);
    ADD_METHOD_TO(ReviewController::listForProduct, "/api/products/{id}/reviews", drogon::Get);
    ADD_METHOD_TO(ReviewController::remove,     "/api/reviews/{id}",              drogon::Delete);
    METHOD_LIST_END

    void create(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void listForProduct(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
    void remove(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
};

} // namespace blessimart::controllers
