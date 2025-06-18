/**
 * @file color.cpp
 * @brief 颜色类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/core/color.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace tinakit {

Color::Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
    : r_(r), g_(g), b_(b), a_(a) {
}

Color::Color(const std::string& hex) {
    std::string hex_str = hex;
    
    // 移除 # 前缀
    if (hex_str.starts_with("#")) {
        hex_str = hex_str.substr(1);
    }
    
    // 检查长度
    if (hex_str.length() != 6 && hex_str.length() != 8) {
        throw std::invalid_argument("Invalid hex color format: " + hex);
    }
    
    try {
        r_ = static_cast<std::uint8_t>(std::stoi(hex_str.substr(0, 2), nullptr, 16));
        g_ = static_cast<std::uint8_t>(std::stoi(hex_str.substr(2, 2), nullptr, 16));
        b_ = static_cast<std::uint8_t>(std::stoi(hex_str.substr(4, 2), nullptr, 16));
        
        if (hex_str.length() == 8) {
            a_ = static_cast<std::uint8_t>(std::stoi(hex_str.substr(6, 2), nullptr, 16));
        } else {
            a_ = 255;
        }
    } catch (const std::exception&) {
        throw std::invalid_argument("Invalid hex color format: " + hex);
    }
}

std::string Color::to_hex() const {
    std::ostringstream oss;
    oss << "#" << std::hex << std::setfill('0') << std::uppercase
        << std::setw(2) << static_cast<int>(r_)
        << std::setw(2) << static_cast<int>(g_)
        << std::setw(2) << static_cast<int>(b_);
    
    if (a_ != 255) {
        oss << std::setw(2) << static_cast<int>(a_);
    }
    
    return oss.str();
}

bool Color::operator==(const Color& other) const noexcept {
    return r_ == other.r_ && g_ == other.g_ && b_ == other.b_ && a_ == other.a_;
}

bool Color::operator!=(const Color& other) const noexcept {
    return !(*this == other);
}

Color Color::from_hex(const std::string& hex) {
    return Color(hex);
}

// 预定义颜色
const Color Color::Black(0, 0, 0);
const Color Color::White(255, 255, 255);
const Color Color::Red(255, 0, 0);
const Color Color::Green(0, 128, 0);
const Color Color::Blue(0, 0, 255);
const Color Color::Yellow(255, 255, 0);
const Color Color::Cyan(0, 255, 255);
const Color Color::Magenta(255, 0, 255);
const Color Color::LightGray(211, 211, 211);
const Color Color::DarkGray(169, 169, 169);
const Color Color::LightBlue(173, 216, 230);
const Color Color::LightGreen(144, 238, 144);
const Color Color::LightRed(255, 182, 193);

} // namespace tinakit 