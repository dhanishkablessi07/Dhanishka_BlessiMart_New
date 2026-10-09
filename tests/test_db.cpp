#include <catch2/catch_test_macros.hpp>
#include "db/Database.h"
#include "db/Database.cpp"

using namespace blessimart::db;

TEST_CASE("Database operations", "[db]") {
    // We will initialize the DB with :memory:
    Database& db = Database::getInstance();
    
    // Attempt to initialize in-memory DB for tests
    try {
        db.initialize(":memory:", ""); // No schema script path needed for basic tests, or mock one
    } catch (...) {
        // May already be initialized, ignore if so, or handle test isolation
    }

    SECTION("Execute and Query") {
        // Create table
        db.execute("CREATE TABLE IF NOT EXISTS test_users (id INTEGER PRIMARY KEY, name TEXT);");
        
        // Insert
        db.execute("INSERT INTO test_users (name) VALUES (?);", {"Alice"});
        long long id = db.lastInsertRowId();
        REQUIRE(id > 0);
        
        // Query
        auto results = db.query("SELECT id, name FROM test_users WHERE name = ?;", {"Alice"});
        REQUIRE(results.size() == 1);
        REQUIRE(results[0]["name"] == "Alice");
        REQUIRE(results[0]["id"] == std::to_string(id));
    }

    SECTION("Foreign Keys Enforcement") {
        db.execute("CREATE TABLE IF NOT EXISTS parent (id INTEGER PRIMARY KEY);");
        db.execute("CREATE TABLE IF NOT EXISTS child (id INTEGER PRIMARY KEY, p_id INTEGER, FOREIGN KEY(p_id) REFERENCES parent(id));");
        
        // Ensure pragma is on
        db.execute("PRAGMA foreign_keys = ON;");
        
        // Should return false when inserting invalid foreign key
        REQUIRE_FALSE(db.execute("INSERT INTO child (p_id) VALUES (999);"));
    }
}
