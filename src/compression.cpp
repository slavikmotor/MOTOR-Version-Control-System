#include "motor/compression.h"
#include <vector>
#include <stdexcept>

namespace motor {
namespace compression {

std::string compress(const std::string& data) {
    if (data.empty()) {
        return data;
    }
    
    std::vector<char> output;
    output.reserve(data.size());
    
    size_t count = 1;
    char current = data[0];
    
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] == current && count < 255) {
            ++count;
        } else {
            output.push_back(static_cast<char>(count));
            output.push_back(current);
            current = data[i];
            count = 1;
        }
    }
    
    output.push_back(static_cast<char>(count));
    output.push_back(current);
    
    return std::string(output.begin(), output.end());
}

std::string decompress(const std::string& data) {
    if (data.empty()) {
        return data;
    }
    
    if (data.size() % 2 != 0) {
        throw std::runtime_error("Invalid compressed data format");
    }
    
    std::vector<char> output;
    
    for (size_t i = 0; i < data.size(); i += 2) {
        unsigned char count = static_cast<unsigned char>(data[i]);
        char value = data[i + 1];
        
        for (unsigned char j = 0; j < count; ++j) {
            output.push_back(value);
        }
    }
    
    return std::string(output.begin(), output.end());
}

} // namespace compression
} // namespace motor 