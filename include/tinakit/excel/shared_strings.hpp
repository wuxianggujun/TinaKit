/**
 * @file shared_strings.hpp
 * @brief Excel 共享字符串管理器
 * @author TinaKit Team
 * @date 2025-01-08
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <optional>

namespace tinakit::excel {

/**
 * @class SharedStrings
 * @brief Excel 共享字符串管理器
 * 
 * 管理 Excel 文件中的共享字符串表。Excel 使用共享字符串来减少文件大小，
 * 相同的字符串只存储一次，单元格通过索引引用。
 */
class SharedStrings {
public:
    /**
     * @brief 构造函数
     */
    SharedStrings() = default;
    
    /**
     * @brief 析构函数
     */
    ~SharedStrings() = default;
    
    /**
     * @brief 添加字符串到共享字符串表
     * @param str 要添加的字符串
     * @return 字符串在共享字符串表中的索引
     */
    std::uint32_t add_string(const std::string& str);
    
    /**
     * @brief 获取字符串的索引
     * @param str 要查找的字符串
     * @return 字符串索引，如果不存在则返回 std::nullopt
     */
    std::optional<std::uint32_t> get_index(const std::string& str) const;
    
    /**
     * @brief 根据索引获取字符串
     * @param index 字符串索引
     * @return 对应的字符串
     * @throws std::out_of_range 如果索引超出范围
     */
    const std::string& get_string(std::uint32_t index) const;
    
    /**
     * @brief 获取共享字符串数量
     * @return 共享字符串数量
     */
    std::size_t count() const noexcept { return strings_.size(); }
    
    /**
     * @brief 获取唯一字符串数量
     * @return 唯一字符串数量（与 count() 相同）
     */
    std::size_t unique_count() const noexcept { return strings_.size(); }
    
    /**
     * @brief 清空共享字符串表
     */
    void clear();
    
    /**
     * @brief 生成共享字符串 XML
     * @return 共享字符串 XML 内容
     */
    std::string generate_xml() const;
    
    /**
     * @brief 从 XML 数据加载共享字符串
     * @param xml_data XML 数据
     */
    void load_from_xml(const std::string& xml_data);
    
    /**
     * @brief 预分配空间
     * @param size 预期的字符串数量
     */
    void reserve(std::size_t size);

private:
    std::vector<std::string> strings_;              ///< 字符串列表（按索引存储）
    std::unordered_map<std::string, std::uint32_t> string_to_index_;  ///< 字符串到索引的映射
};

} // namespace tinakit::excel 