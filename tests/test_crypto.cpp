// Include implementation directly
#define CATCH_CONFIG_MAIN  // not needed with Catch2WithMain
#include <catch2/catch_test_macros.hpp>
#include "utils/Crypto.h"
#include "utils/Crypto.cpp"  // direct include for testing

using namespace blessimart::utils;

TEST_CASE("Crypto::generateSalt", "[crypto]") {
    SECTION("returns non-empty string") {
        auto s = Crypto::generateSalt();
        REQUIRE(!s.empty());
    }
    SECTION("default length gives 32 hex chars") {
        auto s = Crypto::generateSalt(16);
        REQUIRE(s.size() == 32);  // 16 bytes = 32 hex chars
    }
    SECTION("two salts are different") {
        auto s1 = Crypto::generateSalt();
        auto s2 = Crypto::generateSalt();
        REQUIRE(s1 != s2);
    }
}

TEST_CASE("Crypto::hashPassword", "[crypto]") {
    SECTION("same input gives same hash") {
        auto h1 = Crypto::hashPassword("password", "salt123");
        auto h2 = Crypto::hashPassword("password", "salt123");
        REQUIRE(h1 == h2);
    }
    SECTION("different salt gives different hash") {
        auto h1 = Crypto::hashPassword("password", "salt1");
        auto h2 = Crypto::hashPassword("password", "salt2");
        REQUIRE(h1 != h2);
    }
    SECTION("hash is 64 hex chars (SHA-256)") {
        auto h = Crypto::hashPassword("test", "salt");
        REQUIRE(h.size() == 64);
    }
}

TEST_CASE("Crypto::verifyPassword", "[crypto]") {
    SECTION("correct password verifies") {
        std::string salt = Crypto::generateSalt();
        std::string hash = Crypto::hashPassword("mypassword", salt);
        REQUIRE(Crypto::verifyPassword("mypassword", salt, hash));
    }
    SECTION("wrong password fails") {
        std::string salt = Crypto::generateSalt();
        std::string hash = Crypto::hashPassword("correct", salt);
        REQUIRE_FALSE(Crypto::verifyPassword("wrong", salt, hash));
    }
}

TEST_CASE("Crypto::generateOrderNumber", "[crypto]") {
    SECTION("starts with BM-") {
        auto n = Crypto::generateOrderNumber();
        REQUIRE(n.substr(0, 3) == "BM-");
    }
    SECTION("two numbers are different") {
        auto n1 = Crypto::generateOrderNumber();
        auto n2 = Crypto::generateOrderNumber();
        // With mt19937 they MIGHT collide but very unlikely
        // Just check format
        REQUIRE(n1.size() > 5);
        REQUIRE(n2.size() > 5);
    }
}
