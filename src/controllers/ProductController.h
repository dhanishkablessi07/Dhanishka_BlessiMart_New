#pragma once
#include <drogon/HttpController.h>

namespace blessimart::controllers {

class ProductController : public drogon::HttpController<ProductController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ProductController::list,   "/api/products",     drogon::Get);
    ADD_METHOD_TO(ProductController::get,    "/api/products/{id}", drogon::Get);
    ADD_METHOD_TO(ProductController::create, "/api/products",     drogon::Post);
    ADD_METHOD_TO(ProductController::update, "/api/products/{id}", drogon::Put);
    ADD_METHOD_TO(ProductController::remove, "/api/products/{id}", drogon::Delete);
   ADD_METHOD_TO(ProductController::myProducts, "/api/seller/products", drogon::Get);   
    
    METHOD_LIST_END

    void list(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void get(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
    void create(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&);
    void update(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
    void remove(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&&, const std::string& id);
    void myProducts(
    const drogon::HttpRequestPtr&,
    std::function<void(const drogon::HttpResponsePtr&)>&&
);
};

} // namespace blessimart::controllers
