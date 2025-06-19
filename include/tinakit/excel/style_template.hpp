/**
 * @file style_template.hpp
 * @brief Excel 样式模板系统
 * @author TinaKit Team
 * @date 2025-01-19
 */

#pragma once

#include "tinakit/excel/types.hpp"
#include "tinakit/core/color.hpp"
#include <optional>
#include <memory>

namespace tinakit::excel {

// 前向声明
class StyleManager;

/**
 * @class StyleTemplate
 * @brief 用户友好的样式模板类
 * 
 * 提供链式调用接口来创建可复用的样式模板，支持：
 * - 字体设置（名称、大小、粗体、斜体等）
 * - 颜色设置（字体颜色、背景颜色）
 * - 对齐方式（水平、垂直对齐）
 * - 边框样式（类型、样式、颜色）
 * - 数字格式
 * - 文本格式（换行、缩进）
 * 
 * @example
 * ```cpp
 * // 创建标题样式
 * auto title_style = StyleTemplate()
 *     .font("微软雅黑", 16)
 *     .bold()
 *     .color(Color::White)
 *     .background_color(Color::Blue)
 *     .align(Alignment::Center);
 * 
 * // 应用到单元格
 * sheet["A1"].style(title_style);
 * 
 * // 应用到范围
 * sheet.range("A1:C1").style(title_style);
 * ```
 */
class StyleTemplate {
public:
    /**
     * @brief 默认构造函数
     */
    StyleTemplate();
    
    /**
     * @brief 拷贝构造函数
     */
    StyleTemplate(const StyleTemplate& other);
    
    /**
     * @brief 移动构造函数
     */
    StyleTemplate(StyleTemplate&& other) noexcept;
    
    /**
     * @brief 拷贝赋值运算符
     */
    StyleTemplate& operator=(const StyleTemplate& other);
    
    /**
     * @brief 移动赋值运算符
     */
    StyleTemplate& operator=(StyleTemplate&& other) noexcept;
    
    /**
     * @brief 析构函数
     */
    ~StyleTemplate();

    // ========================================
    // 字体设置
    // ========================================
    
    /**
     * @brief 设置字体名称和大小
     * @param font_name 字体名称
     * @param size 字体大小（磅）
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& font(const std::string& font_name, double size = 11.0);
    
    /**
     * @brief 设置粗体
     * @param bold 是否粗体
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& bold(bool bold = true);
    
    /**
     * @brief 设置斜体
     * @param italic 是否斜体
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& italic(bool italic = true);
    
    /**
     * @brief 设置下划线
     * @param underline 是否下划线
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& underline(bool underline = true);
    
    /**
     * @brief 设置删除线
     * @param strike 是否删除线
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& strike(bool strike = true);

    // ========================================
    // 颜色设置
    // ========================================
    
    /**
     * @brief 设置字体颜色
     * @param color 字体颜色
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& color(const Color& color);
    
    /**
     * @brief 设置背景颜色
     * @param color 背景颜色
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& background_color(const Color& color);

    // ========================================
    // 对齐设置
    // ========================================
    
    /**
     * @brief 设置对齐方式
     * @param alignment 对齐方式
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& align(const Alignment& alignment);
    
    /**
     * @brief 设置水平对齐
     * @param horizontal 水平对齐方式
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& align_horizontal(Alignment::Horizontal horizontal);
    
    /**
     * @brief 设置垂直对齐
     * @param vertical 垂直对齐方式
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& align_vertical(Alignment::Vertical vertical);

    // ========================================
    // 边框设置
    // ========================================
    
    /**
     * @brief 设置边框
     * @param type 边框类型
     * @param style 边框样式
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& border(BorderType type, BorderStyle style);
    
    /**
     * @brief 设置彩色边框
     * @param type 边框类型
     * @param style 边框样式
     * @param color 边框颜色
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& border(BorderType type, BorderStyle style, const Color& color);

    // ========================================
    // 数字格式设置
    // ========================================
    
    /**
     * @brief 设置数字格式
     * @param format_code 格式代码
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& number_format(const std::string& format_code);

    // ========================================
    // 文本格式设置
    // ========================================
    
    /**
     * @brief 设置文本换行
     * @param wrap 是否换行
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& wrap_text(bool wrap = true);
    
    /**
     * @brief 设置文本缩进
     * @param indent_level 缩进级别（0-15）
     * @return 自身引用，支持链式调用
     */
    StyleTemplate& indent(int indent_level);

    // ========================================
    // 内部接口（供Cell和Range使用）
    // ========================================
    
    /**
     * @brief 应用样式到StyleManager并获取样式ID
     * @param style_manager 样式管理器
     * @return 样式ID
     */
    std::uint32_t apply_to_style_manager(StyleManager& style_manager) const;
    
    /**
     * @brief 检查是否有任何样式设置
     * @return 如果有样式设置返回true
     */
    bool has_any_style() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// ========================================
// 预定义样式模板
// ========================================

/**
 * @namespace StyleTemplates
 * @brief 预定义的常用样式模板
 */
namespace StyleTemplates {

/**
 * @brief 标题样式
 * @param size 字体大小，默认16
 * @return 标题样式模板
 */
StyleTemplate title(double size = 16.0);

/**
 * @brief 副标题样式
 * @param size 字体大小，默认14
 * @return 副标题样式模板
 */
StyleTemplate subtitle(double size = 14.0);

/**
 * @brief 表头样式
 * @return 表头样式模板
 */
StyleTemplate header();

/**
 * @brief 数据样式
 * @return 数据样式模板
 */
StyleTemplate data();

/**
 * @brief 高亮样式
 * @param color 高亮颜色，默认黄色
 * @return 高亮样式模板
 */
StyleTemplate highlight(const Color& color = Color::Yellow);

/**
 * @brief 警告样式
 * @return 警告样式模板
 */
StyleTemplate warning();

/**
 * @brief 错误样式
 * @return 错误样式模板
 */
StyleTemplate error();

/**
 * @brief 成功样式
 * @return 成功样式模板
 */
StyleTemplate success();

} // namespace StyleTemplates

} // namespace tinakit::excel
