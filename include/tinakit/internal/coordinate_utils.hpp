/**
 * @file coordinate_utils.hpp
 * @brief 坐标转换工具集 - 集中化所有坐标格式转换逻辑
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include "tinakit/core/types.hpp"
#include <string>
#include <string_view>

namespace tinakit::internal::utils {

/**
 * @brief 坐标转换工具类
 * 
 * 这个类集中了所有坐标格式之间的转换逻辑，是整个项目中
 * 处理坐标字符串和数字转换的唯一入口点。
 * 
 * 设计原则：
 * - 单一职责：只负责坐标转换
 * - 无状态：所有方法都是静态的
 * - 高性能：避免不必要的字符串分配
 * - 异常安全：对无效输入抛出明确的异常
 */
class CoordinateUtils {
public:
    // ========================================
    // 单个坐标转换
    // ========================================
    
    /**
     * @brief 将"A1"风格的字符串转换为数字坐标
     * @param str 坐标字符串（如"A1", "Z99", "AA100"）
     * @return 数字坐标
     * @throws std::invalid_argument 如果字符串格式无效
     *
     * 支持的格式：
     * - "A1" -> {1, 1}
     * - "Z99" -> {99, 26}
     * - "AA100" -> {100, 27}
     */
    static core::Coordinate string_to_coordinate(std::string_view str);

    /**
     * @brief 将数字坐标转换为"A1"风格的字符串
     * @param coord 数字坐标
     * @return 坐标字符串
     * @throws std::invalid_argument 如果坐标无效（行列为0）
     *
     * 转换示例：
     * - {1, 1} -> "A1"
     * - {99, 26} -> "Z99"
     * - {100, 27} -> "AA100"
     */
    static std::string coordinate_to_string(const core::Coordinate& coord);
    
    // ========================================
    // 范围地址转换
    // ========================================
    
    /**
     * @brief 将"A1:B5"风格的字符串转换为范围地址
     * @param str 范围字符串（如"A1:B5", "A:C", "5:5"）
     * @return 范围地址
     * @throws std::invalid_argument 如果字符串格式无效
     * 
     * 支持的格式：
     * - "A1:B5" -> 标准矩形范围
     * - "A:C" -> 整列范围
     * - "5:5" -> 整行范围
     * - "A1" -> 单个单元格范围
     */
    static core::range_address string_to_range_address(std::string_view str);
    
    /**
     * @brief 将范围地址转换为"A1:B5"风格的字符串
     * @param addr 范围地址
     * @return 范围字符串
     * @throws std::invalid_argument 如果范围地址无效
     * 
     * 转换示例：
     * - {{1,1}, {5,2}} -> "A1:B5"
     * - {{1,1}, {1,1}} -> "A1"
     */
    static std::string range_address_to_string(const core::range_address& addr);
    
    // ========================================
    // 列号转换辅助函数
    // ========================================
    
    /**
     * @brief 将列字母转换为列号
     * @param column_letters 列字母（如"A", "Z", "AA"）
     * @return 列号（从1开始）
     * @throws std::invalid_argument 如果列字母无效
     * 
     * 转换示例：
     * - "A" -> 1
     * - "Z" -> 26
     * - "AA" -> 27
     */
    static std::size_t column_letters_to_number(std::string_view column_letters);
    
    /**
     * @brief 将列号转换为列字母
     * @param column_number 列号（从1开始）
     * @return 列字母
     * @throws std::invalid_argument 如果列号为0
     * 
     * 转换示例：
     * - 1 -> "A"
     * - 26 -> "Z"
     * - 27 -> "AA"
     */
    static std::string column_number_to_letters(std::size_t column_number);
    
    // ========================================
    // 验证函数
    // ========================================
    
    /**
     * @brief 验证坐标字符串格式是否正确
     * @param str 要验证的字符串
     * @return 如果格式正确返回true
     */
    static bool is_valid_coordinate_string(std::string_view str);
    
    /**
     * @brief 验证范围字符串格式是否正确
     * @param str 要验证的字符串
     * @return 如果格式正确返回true
     */
    static bool is_valid_range_string(std::string_view str);
    
    /**
     * @brief 验证列字母格式是否正确
     * @param letters 要验证的列字母
     * @return 如果格式正确返回true
     */
    static bool is_valid_column_letters(std::string_view letters);

private:
    // 私有辅助函数
    static std::pair<std::string_view, std::string_view> split_coordinate_string(std::string_view str);
    static std::pair<std::string_view, std::string_view> split_range_string(std::string_view str);
};

} // namespace tinakit::internal::utils
