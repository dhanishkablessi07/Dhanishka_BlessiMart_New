#pragma once

#include <drogon/HttpController.h>
#include <string>
#include <functional>

namespace blessimart::controllers {

class CategoryController : public drogon::HttpController<CategoryController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(CategoryController::getAll, "/api/categories", drogon::Get);
    ADD_METHOD_TO(CategoryController::create, "/api/categories", drogon::Post);
    ADD_METHOD_TO(CategoryController::update, "/api/categories/{id}", drogon::Put);
    ADD_METHOD_TO(CategoryController::remove, "/api/categories/{id}", drogon::Delete);

    METHOD_LIST_END

    void getAll(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void create(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void update(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        const std::string& id
    );

    void remove(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        const std::string& id
    );
};

}