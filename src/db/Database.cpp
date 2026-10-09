#include "Database.h"
#include <fstream>
#include <sstream>
#include "../utils/Crypto.h"
namespace blessimart {
namespace db {

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

void Database::initialize(const std::string& dbPath, const std::string& schemaPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
    }
    
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

    std::ifstream schemaFile(schemaPath);
    if (!schemaFile.is_open()) {
        throw std::runtime_error("Failed to open schema file: " + schemaPath);
    }
    std::stringstream buffer;
    buffer << schemaFile.rdbuf();
    std::string schemaSql = buffer.str();

    char* errMsg = nullptr;
    if (sqlite3_exec(db_, schemaSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string errStr(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to execute schema: " + errStr);
    }
    // Seed default admin account if none exists
    auto adminRows = query(
        "SELECT id FROM users WHERE role = 'admin' LIMIT 1",
        {}
    );

    if (adminRows.empty()) {
        std::string adminUsername = "admin";
        std::string adminEmail = "admin@blessimart.com";
        std::string adminPassword = "Admin@123";
        std::string adminFullName = "BlessiMart Admin";

        std::string salt = utils::Crypto::generateSalt();
        std::string hash = utils::Crypto::hashPassword(adminPassword, salt);

        execute(
            "INSERT INTO users "
            "(username, email, password_hash, salt, full_name, role) "
            "VALUES (?, ?, ?, ?, ?, ?)",
            {
                adminUsername,
                adminEmail,
                hash,
                salt,
                adminFullName,
                "admin"
            }
        );
    }
}

bool Database::execute(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        if (sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

ResultSet Database::query(const std::string& sql, const std::vector<std::string>& params) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare query statement: " + std::string(sqlite3_errmsg(db_)));
    }

    for (size_t i = 0; i < params.size(); ++i) {
        if (sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to bind query param: " + std::string(sqlite3_errmsg(db_)));
        }
    }

    ResultSet results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Row row;
        int cols = sqlite3_column_count(stmt);
        for (int i = 0; i < cols; ++i) {
            const char* colName = sqlite3_column_name(stmt, i);
            const char* colText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[colName] = colText ? colText : "";
        }
        results.push_back(row);
    }
    sqlite3_finalize(stmt);
    return results;
}

int64_t Database::lastInsertRowId() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return sqlite3_last_insert_rowid(db_);
}

} // namespace db
} // namespace blessimart
