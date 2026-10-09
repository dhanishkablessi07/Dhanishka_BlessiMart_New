#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <stdexcept>
#include <sqlite3.h>

namespace blessimart {
namespace db {

using Row = std::map<std::string, std::string>;
using ResultSet = std::vector<Row>;

class Database {
public:
    static Database& getInstance();

    // Disable copy
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Initialize DB (apply schema.sql if tables don't exist)
    void initialize(const std::string& dbPath, const std::string& schemaPath);

    // Execute SQL with bound parameters (INSERT, UPDATE, DELETE)
    // params is vector of string values bound positionally to ?
    bool execute(const std::string& sql, const std::vector<std::string>& params = {});

    // Query returning rows
    ResultSet query(const std::string& sql, const std::vector<std::string>& params = {});

    // Get last insert rowid
    int64_t lastInsertRowId();

private:
    Database() = default;
    ~Database();
    sqlite3* db_ = nullptr;
    std::recursive_mutex mutex_;
};

} // namespace db
} // namespace blessimart
