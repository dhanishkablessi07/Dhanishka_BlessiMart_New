#include "Crypto.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include <random>
#include <chrono>

namespace blessimart {
namespace utils {

std::string Crypto::generateSalt(size_t length) {
    // Use RAND_bytes for cryptographic randomness
    std::vector<unsigned char> buf(length);
    if (RAND_bytes(buf.data(), static_cast<int>(length)) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
    std::ostringstream oss;
    for (auto b : buf) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

std::string Crypto::hashPassword(const std::string& password, const std::string& salt) {
    std::string salted = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted.c_str()), salted.size(), hash);
    std::ostringstream oss;
    for (auto b : hash) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

bool Crypto::verifyPassword(const std::string& password, const std::string& salt, const std::string& expectedHash) {
    std::string computed = hashPassword(password, salt);
    // Constant-time compare
    if (computed.size() != expectedHash.size()) return false;
    volatile int diff = 0;
    for (size_t i = 0; i < computed.size(); ++i)
        diff |= computed[i] ^ expectedHash[i];
    return diff == 0;
}

std::string Crypto::generateOrderNumber() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    int year = tm_buf.tm_year + 1900;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(100000, 999999);
    std::ostringstream oss;
    oss << "BM-" << year << "-" << dist(rng);
    return oss.str();
}

} // namespace utils
} // namespace blessimart
