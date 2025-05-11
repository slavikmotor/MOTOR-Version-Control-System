#include "motor/compression.h"
#include <vector>
#include <stdexcept>

namespace motor {
namespace compression {

std::string compress(const std::string& data) {
    if (data.empty()) {
        return data;
    }
    
    std::vector<unsigned char> output;
    output.reserve(data.size());
    
    size_t count = 1;
    unsigned char current = static_cast<unsigned char>(data[0]);
    
    for (size_t i = 1; i < data.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(data[i]);
        if (c == current && count < 255) {
            ++count;
        } else {
            output.push_back(static_cast<unsigned char>(count));
            output.push_back(current);
            current = c;
            count = 1;
        }
    }
    
    output.push_back(static_cast<unsigned char>(count));
    output.push_back(current);
    
    return std::string(reinterpret_cast<char*>(output.data()), output.size());
}

std::string decompress(const std::string& data) {
    if (data.empty()) {
        return data;
    }
    
    if (data.size() % 2 != 0) {
        throw std::runtime_error("Invalid compressed data format");
    }
    
    std::vector<unsigned char> output;
    output.reserve(data.size() * 2);
    
    for (size_t i = 0; i < data.size(); i += 2) {
        unsigned char count = static_cast<unsigned char>(data[i]);
        unsigned char value = static_cast<unsigned char>(data[i + 1]);
        
        for (unsigned char j = 0; j < count; ++j) {
            output.push_back(value);
        }
    }
    
    return std::string(reinterpret_cast<char*>(output.data()), output.size());
}

} // namespace compression
} // namespace motor 