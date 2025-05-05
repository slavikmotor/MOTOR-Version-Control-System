#ifndef MOTOR_COMPRESSION_H
#define MOTOR_COMPRESSION_H

#include <string>

namespace motor {
namespace compression {

std::string compress(const std::string& data);
std::string decompress(const std::string& data);

} // namespace compression
} // namespace motor

#endif // MOTOR_COMPRESSION_H 