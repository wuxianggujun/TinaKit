/**
 * @file color.hpp
 * @brief 颜色类定义
 * @author TinaKit Team
 * @date 2025-6-16
 */

#pragma once

#include <string>
#include <cstdint>

namespace tinakit {

/**
 * @brief 颜色类
 */
class Color {
public:
    /**
     * @brief 从RGB值构造颜色
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     * @param a 透明度分量 (0-255)，默认为255 (不透明)
     */
    Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255);

    /**
     * @brief 从十六进制字符串构造颜色
     * @param hex 十六进制颜色字符串 (例如: "#FF0000" 或 "FF0000")
     */
    explicit Color(const std::string& hex);
    
    /**
     * @brief 从十六进制字符串创建颜色（静态方法）
     * @param hex 十六进制颜色字符串 (例如: "#FF0000" 或 "FF0000")
     * @return Color 对象
     */
    static Color from_hex(const std::string& hex);

    /**
     * @brief 获取红色分量
     */
    std::uint8_t red() const noexcept { return r_; }

    /**
     * @brief 获取绿色分量
     */
    std::uint8_t green() const noexcept { return g_; }

    /**
     * @brief 获取蓝色分量
     */
    std::uint8_t blue() const noexcept { return b_; }

    /**
     * @brief 获取透明度分量
     */
    std::uint8_t alpha() const noexcept { return a_; }

    /**
     * @brief 转换为十六进制字符串
     * @return 十六进制颜色字符串
     */
    std::string to_hex() const;

    /**
     * @brief 转换为Excel格式的颜色字符串（不带#前缀）
     * @return Excel格式的颜色字符串
     */
    std::string to_excel_rgb() const;

    /**
     * @brief 比较运算符
     */
    bool operator==(const Color& other) const noexcept;
    bool operator!=(const Color& other) const noexcept;

public:
    // 预定义颜色
    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color LightGray;
    static const Color DarkGray;
    static const Color LightBlue;
    static const Color LightGreen;
    static const Color LightRed;

private:
    std::uint8_t r_, g_, b_, a_;
};

} // namespace tinakit 