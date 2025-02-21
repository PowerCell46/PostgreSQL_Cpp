#pragma once
#include <vector>
#include <cstdint>


class SHA256 {
    static std::vector<uint8_t> padMessage(const std::string &input);

public:
    static std::string hash(const std::string &input);
};
