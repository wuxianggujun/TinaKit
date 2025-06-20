/**
 * @file workbook_impl.hpp
 * @brief 工作簿内部实现类定义
 * @author TinaKit Team
 * @date 2025-6-20
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/excel/types.hpp"
#include "tinakit/excel/shared_strings.hpp"
#include "tinakit/excel/style_manager.hpp"
#include <memory>
#include <string>
#include <map>
#include <variant>
#include <filesystem>

namespace tinakit::internal {

// 前向声明
class worksheet_impl;

/**
 * @brief 单元格数据结构
 */
struct cell_data {
    using CellValue = std::variant<std::string, double, int, bool>;
    
    CellValue value;
    std::optional<std::string> formula;
    std::uint32_t style_id = 0;
    
    cell_data() = default;
    cell_data(const CellValue& val) : value(val) {}
};

/**
 * @class workbook_impl
 * @brief 工作簿的内部实现类（数据中心）
 * 
 * 这是整个 TinaKit 架构的核心，所有数据都由这个类管理。
 * 所有的句柄类（workbook, worksheet, range, cell）都通过这个类来访问和修改数据。
 * 
 * 核心职责：
 * 1. 管理所有工作表的数据
 * 2. 提供惰性加载机制
 * 3. 管理共享字符串和样式
 * 4. 处理文件 I/O 操作
 * 5. 维护数据一致性
 */
class workbook_impl {
public:
    /**
     * @brief 构造函数（用于创建新工作簿）
     */
    workbook_impl();
    
    /**
     * @brief 构造函数（用于加载现有文件）
     * @param file_path 文件路径
     */
    explicit workbook_impl(const std::filesystem::path& file_path);
    
    /**
     * @brief 析构函数
     */
    ~workbook_impl();
    
    // 禁用拷贝和移动
    workbook_impl(const workbook_impl&) = delete;
    workbook_impl& operator=(const workbook_impl&) = delete;
    workbook_impl(workbook_impl&&) = delete;
    workbook_impl& operator=(workbook_impl&&) = delete;
    
    // ========================================
    // 工作表管理
    // ========================================
    
    /**
     * @brief 获取工作表数量
     */
    std::size_t worksheet_count() const;
    
    /**
     * @brief 获取工作表名称列表
     */
    std::vector<std::string> worksheet_names() const;
    
    /**
     * @brief 检查工作表是否存在
     */
    bool has_worksheet(const std::string& name) const;
    
    /**
     * @brief 创建新工作表
     */
    void create_worksheet(const std::string& name);
    
    /**
     * @brief 删除工作表
     */
    void remove_worksheet(const std::string& name);
    
    /**
     * @brief 重命名工作表
     */
    void rename_worksheet(const std::string& old_name, const std::string& new_name);
    
    // ========================================
    // 单元格数据访问（核心API）
    // ========================================
    
    /**
     * @brief 获取单元格值
     */
    cell_data get_cell_data(const std::string& sheet_name, const core::Position& pos);
    
    /**
     * @brief 设置单元格值
     */
    void set_cell_value(const std::string& sheet_name, const core::Position& pos, const cell_data::CellValue& value);
    
    /**
     * @brief 设置单元格公式
     */
    void set_cell_formula(const std::string& sheet_name, const core::Position& pos, const std::string& formula);
    
    /**
     * @brief 设置单元格样式
     */
    void set_cell_style(const std::string& sheet_name, const core::Position& pos, std::uint32_t style_id);
    
    /**
     * @brief 批量设置范围内的值
     */
    void set_range_values(const std::string& sheet_name, const core::range_address& range, 
                         const std::vector<std::vector<cell_data::CellValue>>& values);
    
    /**
     * @brief 批量设置范围内的样式
     */
    void set_range_style(const std::string& sheet_name, const core::range_address& range, std::uint32_t style_id);
    
    // ========================================
    // 样式管理
    // ========================================
    
    /**
     * @brief 获取样式管理器
     */
    excel::StyleManager& style_manager();
    const excel::StyleManager& style_manager() const;
    
    /**
     * @brief 获取共享字符串管理器
     */
    excel::SharedStrings& shared_strings();
    const excel::SharedStrings& shared_strings() const;

    /**
     * @brief 获取共享字符串管理器指针
     */
    std::shared_ptr<excel::SharedStrings> get_shared_strings() const { return shared_strings_; }
    
    // ========================================
    // 文件操作
    // ========================================
    
    /**
     * @brief 保存到文件
     */
    void save(const std::filesystem::path& file_path);
    
    /**
     * @brief 保存到当前文件
     */
    void save();
    
    /**
     * @brief 获取文件路径
     */
    const std::filesystem::path& file_path() const;

    /**
     * @brief 获取归档器（供 worksheet_impl 使用）
     */
    std::shared_ptr<core::OpenXmlArchiver> get_archiver() const { return archiver_; }

    // ========================================
    // 内部状态管理
    // ========================================
    
    /**
     * @brief 确保工作表已加载
     */
    void ensure_worksheet_loaded(const std::string& sheet_name);
    
    /**
     * @brief 标记工作表为已修改
     */
    void mark_worksheet_dirty(const std::string& sheet_name);
    
private:
    // 文件路径
    std::filesystem::path file_path_;
    
    // OpenXML 归档器
    std::shared_ptr<core::OpenXmlArchiver> archiver_;
    
    // 样式和共享字符串管理
    std::shared_ptr<excel::StyleManager> style_manager_;
    std::shared_ptr<excel::SharedStrings> shared_strings_;
    
    // 工作表实现映射
    std::map<std::string, std::unique_ptr<worksheet_impl>> worksheets_;
    
    // 工作表名称顺序
    std::vector<std::string> worksheet_order_;

    // 活动工作表名称
    std::string active_sheet_name_;

    // 修改标志
    bool is_dirty_ = false;
    
    // 内部方法
    void load_from_file();
    void create_default_structure();
    void parse_workbook_xml();
    void parse_workbook_rels();
    void load_styles_xml();
    void load_shared_strings_xml();
    void save_to_archiver();
    void generate_content_types();
    void generate_main_rels();
    void generate_workbook_xml();
    void generate_workbook_rels();
    void generate_styles_xml();
    void generate_shared_strings_xml();
    
    // 获取或创建工作表实现
    worksheet_impl& get_worksheet_impl(const std::string& sheet_name);
};

} // namespace tinakit::internal
