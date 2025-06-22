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
#include <iomanip>
#include <filesystem>
#include "tinakit/pdf/document.hpp"
#include "tinakit/pdf/types.hpp"
#include "tinakit/pdf/core/writer.hpp"
#include "tinakit/pdf/core/page.hpp"

namespace tinakit::core {
    class Image;
}

namespace tinakit::pdf::internal {

// 前向声明
class pdf_document_impl;

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

    // 禁用拷贝，支持移动
    pdf_document_impl(const pdf_document_impl&) = delete;
    pdf_document_impl& operator=(const pdf_document_impl&) = delete;
    pdf_document_impl(pdf_document_impl&&) = default;
    pdf_document_impl& operator=(pdf_document_impl&&) = default;

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
    core::PdfPage* current_page();

    // ========================================
    // 内容添加
    // ========================================
    
    void add_text(const std::string& text, const Point& position, const Font& font);
    void add_text_block(const std::string& text, const Rect& bounds,
                       const Font& font, TextAlignment alignment);
    void add_table(const Table& table, const Point& position);

    // ========================================
    // 图像功能
    // ========================================

    void add_image(const std::string& image_path, const Point& position,
                   double width, double height);
    void add_image(const tinakit::core::Image& image, const Point& position,
                   double width, double height);
    void add_image(const std::vector<std::uint8_t>& image_data,
                   int width, int height, int channels,
                   const Point& position,
                   double display_width, double display_height);

    // ========================================
    // Excel集成
    // ========================================
    
    void add_excel_table(const excel::Worksheet& sheet,
                         const std::string& range_address,
                         const Point& position,
                         bool preserve_formatting);

    void add_excel_range(const excel::Range& range,
                         const Point& position,
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
     * @brief 获取或创建字体对象
     */
    std::string get_font_resource_id(const Font& font);

    /**
     * @brief 根据字体名称获取或创建字体资源ID
     */
    std::string get_font_resource_id_for_font_name(const std::string& font_name);
    
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
    
    // PDF生成功能现在由core::Writer类统一处理
    
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
    
    // 核心写入器 - 统一管理PDF对象和结构
    std::unique_ptr<core::Writer> writer_;  ///< PDF核心写入器

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
