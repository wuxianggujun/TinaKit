/**
 * @file pdf_document_impl.hpp
 * @brief PDF文档内部实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <map>
#include "tinakit/pdf/document.hpp"

namespace tinakit::pdf::internal {

/**
 * @brief PDF对象基类
 */
class pdf_object {
public:
    pdf_object(int id) : object_id_(id) {}
    virtual ~pdf_object() = default;
    
    int object_id() const { return object_id_; }
    virtual std::string to_pdf_string() const = 0;

private:
    int object_id_;
};

/**
 * @brief PDF页面对象
 */
class pdf_page : public pdf_object {
public:
    pdf_page(int id, double width, double height);
    
    void add_content(const std::string& content);
    std::string to_pdf_string() const override;
    
    double width() const { return width_; }
    double height() const { return height_; }

    const std::vector<std::string>& get_content_streams() const { return content_streams_; }

private:
    double width_;
    double height_;
    std::vector<std::string> content_streams_;
};

/**
 * @brief PDF字体对象
 */
class pdf_font : public pdf_object {
public:
    pdf_font(int id, const std::string& name);
    std::string to_pdf_string() const override;
    
    const std::string& name() const { return font_name_; }

private:
    std::string font_name_;
};

/**
 * @brief PDF文档实现类
 */
class pdf_document_impl {
public:
    // ========================================
    // 构造函数和析构函数
    // ========================================
    
    pdf_document_impl();
    ~pdf_document_impl() = default;

    // ========================================
    // 文档设置
    // ========================================
    
    void set_page_size(PageSize size, PageOrientation orientation);
    void set_custom_page_size(double width, double height);
    void set_margins(const PageMargins& margins);
    void set_document_info(const DocumentInfo& info);

    // ========================================
    // 页面管理
    // ========================================
    
    void add_page();
    std::size_t page_count() const;
    pdf_page* current_page();

    // ========================================
    // 内容添加
    // ========================================
    
    void add_text(const std::string& text, const core::Point& position, const Font& font);
    void add_text_block(const std::string& text, const core::Rect& bounds,
                       const Font& font, TextAlignment alignment);
    void add_table(const Table& table, const core::Point& position);

    // ========================================
    // Excel集成
    // ========================================
    
    void add_excel_table(const excel::Worksheet& sheet,
                         const std::string& range_address,
                         const core::Point& position,
                         bool preserve_formatting);

    void add_excel_range(const excel::Range& range,
                         const core::Point& position,
                         bool preserve_formatting);
    
    void add_excel_sheet(const excel::Worksheet& sheet, bool preserve_formatting);

    // ========================================
    // 文件操作
    // ========================================
    
    void save(const std::filesystem::path& file_path);
    std::vector<std::uint8_t> save_to_buffer();

private:
    // ========================================
    // 内部辅助方法
    // ========================================
    
    /**
     * @brief 获取下一个对象ID
     */
    int next_object_id();
    
    /**
     * @brief 添加PDF对象
     */
    void add_object(std::unique_ptr<pdf_object> obj);
    
    /**
     * @brief 获取或创建字体对象
     */
    int get_font_id(const Font& font);
    
    /**
     * @brief 转换颜色到PDF格式
     */
    std::string color_to_pdf(const Color& color);
    
    /**
     * @brief 转换文本到PDF格式（处理特殊字符）
     */
    std::string escape_pdf_text(const std::string& text);

    /**
     * @brief 将文本转换为十六进制编码（用于中文字符）
     */
    std::string text_to_hex(const std::string& text);
    
    /**
     * @brief 生成PDF文档结构
     */
    std::string generate_pdf_document();
    
    /**
     * @brief 生成PDF头部
     */
    std::string generate_pdf_header();
    
    /**
     * @brief 生成PDF目录
     */
    std::string generate_pdf_catalog();
    
    /**
     * @brief 生成PDF页面树
     */
    std::string generate_pdf_pages();
    
    /**
     * @brief 生成PDF交叉引用表
     */
    std::string generate_xref_table(const std::vector<size_t>& offsets);
    
    /**
     * @brief 生成PDF尾部
     */
    std::string generate_pdf_trailer(size_t xref_offset, const std::vector<size_t>& offsets);
    
    /**
     * @brief 从Excel单元格创建表格单元格
     */
    TableCell excel_cell_to_pdf_cell(const excel::Cell& cell, bool preserve_formatting);
    
    /**
     * @brief 计算文本宽度
     */
    double calculate_text_width(const std::string& text, const Font& font);
    
    /**
     * @brief 页面大小转换为点数
     */
    std::pair<double, double> page_size_to_points(PageSize size, PageOrientation orientation);

    // ========================================
    // 成员变量
    // ========================================
    
    // 文档设置
    double page_width_ = 595.0;     ///< 页面宽度（点）A4默认
    double page_height_ = 842.0;    ///< 页面高度（点）A4默认
    PageMargins margins_;           ///< 页面边距
    DocumentInfo doc_info_;         ///< 文档信息
    
    // 对象管理
    int next_object_id_ = 1;        ///< 下一个对象ID
    std::vector<std::unique_ptr<pdf_object>> objects_;  ///< PDF对象列表
    std::vector<std::unique_ptr<pdf_page>> pages_;      ///< 页面列表
    
    // 字体管理
    std::map<std::string, int> font_cache_;  ///< 字体缓存（字体名->对象ID）
    
    // 当前状态
    int current_page_index_ = -1;   ///< 当前页面索引
};

// ========================================
// 辅助函数
// ========================================

/**
 * @brief 将Excel颜色转换为PDF颜色
 */
Color excel_color_to_pdf_color(const Color& excel_color);

/**
 * @brief 将Excel字体信息转换为PDF字体
 */
Font excel_font_to_pdf_font(const std::string& font_name, double font_size, bool bold, bool italic);

/**
 * @brief 将Excel对齐方式转换为PDF对齐方式
 */
TextAlignment excel_alignment_to_pdf_alignment(int alignment);

} // namespace tinakit::pdf::internal
