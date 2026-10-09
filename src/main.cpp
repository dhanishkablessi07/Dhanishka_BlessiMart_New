#include <drogon/drogon.h>
#include "db/Database.h"
// include all controllers to ensure they register
#include "controllers/AuthController.h"
#include "controllers/CategoryController.h"
#include "controllers/ProductController.h"
#include "controllers/CartController.h"
#include "controllers/OrderController.h"
#include "controllers/ReviewController.h"
#include "controllers/AdminController.h"

#ifndef BLESSIMART_SCHEMA_PATH
#define BLESSIMART_SCHEMA_PATH "schema.sql"
#endif

int main() {
    // Initialize database
    blessimart::db::Database::getInstance().initialize(
        "blessimart.db",
        BLESSIMART_SCHEMA_PATH
    );

    // Load Drogon config
    drogon::app().loadConfigFile("config/config.json");

    // Run
    drogon::app().run();
    return 0;
}
