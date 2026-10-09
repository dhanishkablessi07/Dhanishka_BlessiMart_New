#pragma once

#include <drogon/HttpController.h>

namespace blessimart::controllers {

class ChatController : public drogon::HttpController<ChatController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ChatController::chat,
        "/api/chat",
        drogon::Post
    );

    METHOD_LIST_END

    void chat(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
};

}