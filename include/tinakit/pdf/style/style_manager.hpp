/**
 * @file style_manager.hpp
 * @brief PDF样式管理器 - 统一管理所有样式定义
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include "tinakit/pdf/style.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace tinakit::pdf::style {

/**
 * @class StyleManager
 * @brief PDF样式管理器
 * 
 * 提供样式的注册、获取、继承和组合功能，支持样式模板和主题
 */
class StyleManager {
public:
    // ========================================
    // 构造函数和析构函数
    // ========================================
    
    /**
     * @brief 构造函数
     */
    StyleManager();
    
    /**
     * @brief 析构函数
     */
    ~StyleManager() = default;
    
    /**
     * @brief 创建默认样式管理器
     * @return 包含预定义样式的管理器
     */
    static std::unique_ptr<StyleManager> createDefault();
    
    /**
     * @brief 创建空的样式管理器
     * @return 空的样式管理器
     */
    static std::unique_ptr<StyleManager> createEmpty();

    // ========================================
    // 样式注册
    // ========================================
    
    /**
     * @brief 注册文本样式
     * @param name 样式名称
     * @param style 文本样式
     */
    void registerTextStyle(const std::string& name, const FontStyle& style);
    
    /**
     * @brief 注册单元格样式
     * @param name 样式名称
     * @param style 单元格样式
     */
    void registerCellStyle(const std::string& name, const CellStyle& style);
    
    /**
     * @brief 注册表格样式
     * @param name 样式名称
     * @param style 表格样式
     */
    void registerTableStyle(const std::string& name, const TableStyle& style);
    
    /**
     * @brief 批量注册样式
     * @param styles 样式映射表
     */
    void registerTextStyles(const std::map<std::string, FontStyle>& styles);
    void registerCellStyles(const std::map<std::string, CellStyle>& styles);
    void registerTableStyles(const std::map<std::string, TableStyle>& styles);

    // ========================================
    // 样式获取
    // ========================================
    
    /**
     * @brief 获取文本样式
     * @param name 样式名称
     * @return 文本样式，如果不存在返回默认样式
     */
    FontStyle getTextStyle(const std::string& name) const;
    
    /**
     * @brief 获取单元格样式
     * @param name 样式名称
     * @return 单元格样式，如果不存在返回默认样式
     */
    CellStyle getCellStyle(const std::string& name) const;
    
    /**
     * @brief 获取表格样式
     * @param name 样式名称
     * @return 表格样式，如果不存在返回默认样式
     */
    TableStyle getTableStyle(const std::string& name) const;
    
    /**
     * @brief 检查样式是否存在
     * @param name 样式名称
     * @return 如果存在返回true
     */
    bool hasTextStyle(const std::string& name) const;
    bool hasCellStyle(const std::string& name) const;
    bool hasTableStyle(const std::string& name) const;

    // ========================================
    // 样式继承和组合
    // ========================================
    
    /**
     * @brief 创建继承样式
     * @param base_name 基础样式名称
     * @param overrides 覆盖的属性
     * @return 组合后的样式
     */
    FontStyle inheritTextStyle(const std::string& base_name, const FontStyle& overrides) const;
    CellStyle inheritCellStyle(const std::string& base_name, const CellStyle& overrides) const;
    TableStyle inheritTableStyle(const std::string& base_name, const TableStyle& overrides) const;
    
    /**
     * @brief 组合多个样式
     * @param style_names 样式名称列表（后面的覆盖前面的）
     * @return 组合后的样式
     */
    FontStyle combineTextStyles(const std::vector<std::string>& style_names) const;
    CellStyle combineCellStyles(const std::vector<std::string>& style_names) const;
    TableStyle combineTableStyles(const std::vector<std::string>& style_names) const;

    // ========================================
    // 主题支持
    // ========================================
    
    /**
     * @brief 样式主题
     */
    struct StyleTheme {
        std::string name;                           ///< 主题名称
        std::map<std::string, FontStyle> text_styles;    ///< 文本样式
        std::map<std::string, CellStyle> cell_styles;    ///< 单元格样式
        std::map<std::string, TableStyle> table_styles;  ///< 表格样式
    };
    
    /**
     * @brief 应用样式主题
     * @param theme 样式主题
     */
    void applyTheme(const StyleTheme& theme);
    
    /**
     * @brief 获取当前主题
     * @return 当前样式主题
     */
    StyleTheme getCurrentTheme() const;
    
    /**
     * @brief 创建预定义主题
     */
    static StyleTheme createDefaultTheme();
    static StyleTheme createProfessionalTheme();
    static StyleTheme createMinimalTheme();
    static StyleTheme createColorfulTheme();

    // ========================================
    // 样式查询和管理
    // ========================================
    
    /**
     * @brief 获取所有样式名称
     */
    std::vector<std::string> getTextStyleNames() const;
    std::vector<std::string> getCellStyleNames() const;
    std::vector<std::string> getTableStyleNames() const;
    
    /**
     * @brief 删除样式
     * @param name 样式名称
     */
    void removeTextStyle(const std::string& name);
    void removeCellStyle(const std::string& name);
    void removeTableStyle(const std::string& name);
    
    /**
     * @brief 清空所有样式
     */
    void clear();
    
    /**
     * @brief 重置为默认样式
     */
    void resetToDefault();

    // ========================================
    // 样式导入导出
    // ========================================
    
    /**
     * @brief 导出样式到JSON
     * @return JSON格式的样式定义
     */
    std::string exportToJson() const;
    
    /**
     * @brief 从JSON导入样式
     * @param json JSON格式的样式定义
     */
    void importFromJson(const std::string& json);
    
    /**
     * @brief 保存样式到文件
     * @param filename 文件名
     */
    void saveToFile(const std::string& filename) const;
    
    /**
     * @brief 从文件加载样式
     * @param filename 文件名
     */
    void loadFromFile(const std::string& filename);

private:
    // ========================================
    // 成员变量
    // ========================================
    
    std::map<std::string, FontStyle> text_styles_;      ///< 文本样式映射
    std::map<std::string, CellStyle> cell_styles_;      ///< 单元格样式映射
    std::map<std::string, TableStyle> table_styles_;    ///< 表格样式映射
    
    // 默认样式
    FontStyle default_text_style_;                       ///< 默认文本样式
    CellStyle default_cell_style_;                       ///< 默认单元格样式
    TableStyle default_table_style_;                     ///< 默认表格样式
    
    // ========================================
    // 内部方法
    // ========================================
    
    /**
     * @brief 初始化默认样式
     */
    void initializeDefaultStyles();
    
    /**
     * @brief 加载预定义样式
     */
    void loadPredefinedStyles();
    
    /**
     * @brief 验证样式名称
     * @param name 样式名称
     * @return 如果有效返回true
     */
    bool isValidStyleName(const std::string& name) const;
};

// ========================================
// 全局样式管理器
// ========================================

/**
 * @brief 获取全局样式管理器实例
 * @return 全局样式管理器
 */
StyleManager& getGlobalStyleManager();

/**
 * @brief 设置全局样式管理器
 * @param manager 样式管理器
 */
void setGlobalStyleManager(std::unique_ptr<StyleManager> manager);

// ========================================
// 便利函数
// ========================================

/**
 * @brief 快速获取文本样式
 * @param name 样式名称
 * @return 文本样式
 */
inline FontStyle getTextStyle(const std::string& name) {
    return getGlobalStyleManager().getTextStyle(name);
}

/**
 * @brief 快速获取单元格样式
 * @param name 样式名称
 * @return 单元格样式
 */
inline CellStyle getCellStyle(const std::string& name) {
    return getGlobalStyleManager().getCellStyle(name);
}

/**
 * @brief 快速获取表格样式
 * @param name 样式名称
 * @return 表格样式
 */
inline TableStyle getTableStyle(const std::string& name) {
    return getGlobalStyleManager().getTableStyle(name);
}

} // namespace tinakit::pdf::style
