/**
 * @file pdf_document_impl.cpp
 * @brief PDF文档内部实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/internal/pdf_document_impl.hpp"
#include "tinakit/pdf/core/writer.hpp"
#include "tinakit/core/logger.hpp"
#include <stdexcept>

namespace tinakit::pdf::internal {

// ========================================
// 构造函数和析构函数
// ========================================

pdf_document_impl::pdf_document_impl() 
    : writer_(std::make_unique<core::Writer>()) {
    // PDF文档实现初始化
}

// ========================================
// 文档设置
// ========================================

void pdf_document_impl::set_page_size(PageSize size, PageOrientation orientation) {
    auto [width, height] = page_size_to_points(size, orientation);
    page_width_ = width;
    page_height_ = height;
    
}

void pdf_document_impl::set_custom_page_size(double width, double height) {
    page_width_ = width;
    page_height_ = height;
}

void pdf_document_impl::set_margins(const PageMargins& margins) {
    margins_ = margins;
}

void pdf_document_impl::set_document_info(const DocumentInfo& info) {
    doc_info_ = info;
    writer_->setDocumentInfo(info.title, info.author, info.subject, info.creator);
}

// ========================================
// 页面管理
// ========================================

void pdf_document_impl::add_page() {
    writer_->createPage(page_width_, page_height_);
    current_page_index_ = static_cast<int>(writer_->getPageCount() - 1);
}

std::size_t pdf_document_impl::page_count() const {
    return writer_->getPageCount();
}

core::PdfPage* pdf_document_impl::current_page() {
    if (current_page_index_ >= 0 && 
        current_page_index_ < static_cast<int>(writer_->getPageCount())) {
        return writer_->getPage(current_page_index_);
    }
    return nullptr;
}

// ========================================
// 内容添加
// ========================================

void pdf_document_impl::add_text(const std::string& text, const Point& position, const Font& font) {
    auto page = current_page();
    if (!page) {
        return;
    }
    
    // 获取或注册字体
    std::string font_resource = get_font_resource_id(font);
    
    // 添加文本到页面
    page->beginText();
    page->setFont(font_resource, font.size);
    page->setTextPosition(position.x, position.y);
    page->setTextColor(font.color.red() / 255.0, font.color.green() / 255.0, font.color.blue() / 255.0);
    page->showText(text);
    page->endText();
}

void pdf_document_impl::add_text_block(const std::string& text, const Rect& bounds,
                                      const Font& font, TextAlignment alignment) {
    auto page = current_page();
    if (!page) {
        return;
    }
    
    // 获取或注册字体
    std::string font_resource = get_font_resource_id(font);
    
    // 简单实现：将文本放在边界框的左上角
    // TODO: 实现文本换行和对齐
    page->beginText();
    page->setFont(font_resource, font.size);
    page->setTextPosition(bounds.x, bounds.y + bounds.height - font.size);
    page->setTextColor(font.color.red() / 255.0, font.color.green() / 255.0, font.color.blue() / 255.0);
    page->showText(text);
    page->endText();
}

void pdf_document_impl::add_table(const Table& table, const Point& position) {
    // TODO: 实现表格渲染
}

// ========================================
// Excel集成
// ========================================

void pdf_document_impl::add_excel_table(const excel::Worksheet& sheet,
                                       const std::string& range_address,
                                       const Point& position,
                                       bool preserve_formatting) {
    // TODO: 实现Excel表格集成
}

void pdf_document_impl::add_excel_range(const excel::Range& range,
                                       const Point& position,
                                       bool preserve_formatting) {
    // TODO: 实现Excel范围集成
}

void pdf_document_impl::add_excel_sheet(const excel::Worksheet& sheet, bool preserve_formatting) {
    // TODO: 实现Excel工作表集成
}

// ========================================
// 文件操作
// ========================================

void pdf_document_impl::save(const std::filesystem::path& file_path) {
    if (writer_->getPageCount() == 0) {
        add_page();
    }

    writer_->saveToFile(file_path.string());
}

std::vector<std::uint8_t> pdf_document_impl::save_to_buffer() {
    if (writer_->getPageCount() == 0) {
        add_page();
    }

    return writer_->saveToBuffer();
}

// ========================================
// 内部辅助方法
// ========================================

std::string pdf_document_impl::get_font_resource_id(const Font& font) {
    // 检查字体是否已注册
    std::string resource_id = writer_->getFontResourceId(font.family);
    if (resource_id.empty()) {
        // 注册新字体
        resource_id = writer_->registerFont(font.family);
    }
    return resource_id;
}

std::string pdf_document_impl::color_to_pdf(const Color& color) {
    std::ostringstream oss;
    oss << (color.red() / 255.0) << " " << (color.green() / 255.0) << " " << (color.blue() / 255.0);
    return oss.str();
}

std::string pdf_document_impl::escape_pdf_text(const std::string& text) {
    std::string result;
    result.reserve(text.length() * 2);
    
    for (char c : text) {
        switch (c) {
            case '(':
                result += "\\(";
                break;
            case ')':
                result += "\\)";
                break;
            case '\\':
                result += "\\\\";
                break;
            default:
                result += c;
                break;
        }
    }
    
    return result;
}

std::string pdf_document_impl::text_to_hex(const std::string& text) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase;
    for (unsigned char c : text) {
        oss << std::setfill('0') << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

double pdf_document_impl::calculate_text_width(const std::string& text, const Font& font) {
    // 简单估算：每个字符约为字体大小的0.6倍
    return text.length() * font.size * 0.6;
}

std::pair<double, double> pdf_document_impl::page_size_to_points(PageSize size, PageOrientation orientation) {
    return tinakit::pdf::page_size_to_points(size, orientation);
}

// ========================================
// 辅助函数实现
// ========================================

Color excel_color_to_pdf_color(const Color& excel_color) {
    return excel_color;  // 假设颜色格式相同
}

Font excel_font_to_pdf_font(const std::string& font_name, double font_size, bool bold, bool italic) {
    Font font;
    font.family = font_name;
    font.size = font_size;
    font.bold = bold;
    font.italic = italic;
    return font;
}

TextAlignment excel_alignment_to_pdf_alignment(int alignment) {
    // TODO: 实现Excel对齐方式转换
    return TextAlignment::Left;
}

} // namespace tinakit::pdf::internal
