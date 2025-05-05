#ifndef MOTOR_UTILS_H
#define MOTOR_UTILS_H

#include <string>
#include <vector>
#include <openssl/sha.h>

namespace motor {
namespace utils {

std::string hashSHA1(const std::string& data);

std::string compressData(const std::string& data);
std::string decompressData(const std::string& data);

std::string binaryToHex(const std::string& binary);
std::string hexToBinary(const std::string& hex);

std::vector<std::string> splitString(const std::string& str, char delimiter);
std::string trimString(const std::string& str);

} // namespace utils
} // namespace motor

#endif // MOTOR_UTILS_H 