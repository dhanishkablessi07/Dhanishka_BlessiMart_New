#pragma once

#include <string>

namespace blessimart {
namespace utils {

class Crypto {
public:
    // Generate a random hex salt string
    static std::string generateSalt(size_t length = 16);

    // Hash password with salt using SHA-256
    static std::string hashPassword(const std::string& password, const std::string& salt);

    // Verify password against stored hash and salt
    static bool verifyPassword(const std::string& password, const std::string& salt, const std::string& expectedHash);

    // Generate random order number e.g. "BM-2026-XXXX"
    static std::string generateOrderNumber();
};

} // namespace utils
} // namespace blessimart
