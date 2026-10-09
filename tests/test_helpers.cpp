#include <catch2/catch_test_macros.hpp>
#include "utils/JsonHelper.h"
#include <json/json.h>

using namespace blessimart::utils;

TEST_CASE("JsonHelper::missingFields", "[json_helper]") {
    SECTION("identifies missing fields") {
        Json::Value json;
        json["name"] = "Alice";
        
        auto missing = JsonHelper::missingFields(json, {"name", "email", "password"});
        REQUIRE(missing.size() == 2);
        REQUIRE(std::find(missing.begin(), missing.end(), "email") != missing.end());
        REQUIRE(std::find(missing.begin(), missing.end(), "password") != missing.end());
    }

    SECTION("identifies empty strings as missing") {
        Json::Value json;
        json["name"] = "Alice";
        json["email"] = "";
        
        auto missing = JsonHelper::missingFields(json, {"name", "email"});
        REQUIRE(missing.size() == 1);
        REQUIRE(missing[0] == "email");
    }

    SECTION("returns empty when all fields present") {
        Json::Value json;
        json["name"] = "Alice";
        json["email"] = "alice@example.com";
        
        auto missing = JsonHelper::missingFields(json, {"name", "email"});
        REQUIRE(missing.empty());
    }
}

// SessionHelper tests are skipped as HttpRequest mock is complex for unit testing here.
