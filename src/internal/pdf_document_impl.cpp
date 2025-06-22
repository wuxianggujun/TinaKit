/**
 * @file pdf_document_impl.cpp
 * @brief PDF文档内部实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/internal/pdf_document_impl.hpp"
#include "tinakit/pdf/binary_writer.hpp"
#include "tinakit/core/exceptions.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace tinakit::pdf::internal {

// ========================================
// PDF页面对象实现
// ========================================

pdf_page::pdf_page(int id, double width, double height)
    : pdf_object(id), width_(width), height_(height) {
}

void pdf_page::add_content(const std::string& content) {
    content_streams_.push_back(content);
}

std::string pdf_page::to_pdf_string() const {
    std::ostringstream oss;
    oss << object_id() << " 0 obj\n";
    oss << "<<\n";
    oss << "/Type /Page\n";
    oss << "/MediaBox [0 0 " << width_ << " " << height_ << "]\n";
    oss << "/Contents [";
    
    // 添加内容流引用
    for (size_t i = 0; i < content_streams_.size(); ++i) {
        if (i > 0) oss << " ";
        oss << (object_id() + 1 + static_cast<int>(i)) << " 0 R";
    }
    
    oss << "]\n";
    oss << ">>\n";
    oss << "endobj\n";
    
    return oss.str();
}

// ========================================
// PDF字体对象实现
// ========================================

pdf_font::pdf_font(int id, const std::string& name)
    : pdf_object(id), font_name_(name) {
}

std::string pdf_font::to_pdf_string() const {
    std::ostringstream oss;
    oss << object_id() << " 0 obj\n";
    oss << "<<\n";
    oss << "/Type /Font\n";
    oss << "/Subtype /Type1\n";
    oss << "/BaseFont /" << font_name_ << "\n";
    oss << ">>\n";
    oss << "endobj\n";
    
    return oss.str();
}

// ========================================
// PDF文档实现类
// ========================================

pdf_document_impl::pdf_document_impl() {
    // 设置默认页面大小为A4
    page_width_ = 595.0;   // A4宽度（点）
    page_height_ = 842.0;  // A4高度（点）
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
}

// ========================================
// 页面管理
// ========================================

void pdf_document_impl::add_page() {
    auto page = std::make_unique<pdf_page>(next_object_id(), page_width_, page_height_);
    current_page_index_ = static_cast<int>(pages_.size());
    pages_.push_back(std::move(page));
}

std::size_t pdf_document_impl::page_count() const {
    return pages_.size();
}

pdf_page* pdf_document_impl::current_page() {
    if (current_page_index_ >= 0 && current_page_index_ < static_cast<int>(pages_.size())) {
        return pages_[current_page_index_].get();
    }
    return nullptr;
}

// ========================================
// 内容添加
// ========================================

void pdf_document_impl::add_text(const std::string& text, const core::Point& position, const FontStyle& font) {
    if (!current_page()) {
        add_page();
    }

    // 检查是否包含中文字符
    bool has_chinese = false;
    for (unsigned char c : text) {
        if (c > 127) {  // 非ASCII字符
            has_chinese = true;
            break;
        }
    }

    // 生成PDF文本命令
    std::ostringstream content;
    content << "BT\n";  // 开始文本

    if (has_chinese) {
        // 对于中文，使用十六进制编码
        content << "/F" << get_font_id(font) << " " << font.size << " Tf\n";
        content << color_to_pdf(font.color) << " rg\n";
        content << position.x << " " << position.y << " Td\n";
        content << "<" << text_to_hex(text) << "> Tj\n";  // 使用十六进制字符串
    } else {
        // 英文使用普通编码
        content << "/F" << get_font_id(font) << " " << font.size << " Tf\n";
        content << color_to_pdf(font.color) << " rg\n";
        content << position.x << " " << position.y << " Td\n";
        content << "(" << escape_pdf_text(text) << ") Tj\n";
    }

    content << "ET\n";  // 结束文本

    current_page()->add_content(content.str());
}

void pdf_document_impl::add_text_block(const std::string& text, const core::Rect& bounds, 
                                      const FontStyle& font, TextAlignment alignment) {
    // 简化实现：将文本块作为单行文本处理
    core::Point position(bounds.x, bounds.y + bounds.height - font.size);
    add_text(text, position, font);
}

void pdf_document_impl::add_table(const Table& table, const core::Point& position) {
    if (table.rows.empty()) return;
    
    double current_y = position.y;
    double row_height = table.style.row_height;
    
    for (size_t row_idx = 0; row_idx < table.rows.size(); ++row_idx) {
        const auto& row = table.rows[row_idx];
        double current_x = position.x;
        
        for (size_t col_idx = 0; col_idx < row.size() && col_idx < table.column_widths.size(); ++col_idx) {
            const auto& cell = row[col_idx];
            double col_width = table.column_widths[col_idx];
            
            // 绘制单元格背景
            if (cell.style.background_color != Color::White) {
                // TODO: 实现背景绘制
            }
            
            // 绘制文本
            core::Point text_pos(current_x + cell.style.padding, current_y - row_height + cell.style.padding);
            add_text(cell.text, text_pos, cell.style.font);
            
            current_x += col_width;
        }
        
        current_y -= row_height;
    }
}

// ========================================
// Excel集成
// ========================================

void pdf_document_impl::add_excel_table(const excel::Worksheet& sheet, 
                                       const std::string& range_address,
                                       const core::Point& position,
                                       bool preserve_formatting) {
    // 简化实现：创建一个基本表格
    Table table;
    table.style = StyleTemplates::professional_table();
    
    // 添加示例数据（实际实现中应该从Excel读取）
    TableRow header_row;
    header_row.push_back(TableCell("列1", table.style.header_style));
    header_row.push_back(TableCell("列2", table.style.header_style));
    table.rows.push_back(header_row);
    
    TableRow data_row;
    data_row.push_back(TableCell("数据1", table.style.data_style));
    data_row.push_back(TableCell("数据2", table.style.data_style));
    table.rows.push_back(data_row);
    
    table.column_widths = {100.0, 100.0};
    table.has_header = true;
    
    add_table(table, position);
}

void pdf_document_impl::add_excel_range(const excel::Range& range, 
                                       const core::Point& position,
                                       bool preserve_formatting) {
    // 简化实现
    add_text("Excel Range Data", position, FontStyle());
}

void pdf_document_impl::add_excel_sheet(const excel::Worksheet& sheet, bool preserve_formatting) {
    // 简化实现
    add_text("Excel Sheet Content", core::Point(50, 750), FontStyle());
}

// ========================================
// 文件操作
// ========================================

void pdf_document_impl::save(const std::filesystem::path& file_path) {
    auto buffer = save_to_buffer();
    
    std::ofstream file(file_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法创建PDF文件: " + file_path.string());
    }
    
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

std::vector<std::uint8_t> pdf_document_impl::save_to_buffer() {
    std::string pdf_content = generate_pdf_document();
    
    std::vector<std::uint8_t> buffer(pdf_content.begin(), pdf_content.end());
    return buffer;
}

// ========================================
// 内部辅助方法
// ========================================

int pdf_document_impl::next_object_id() {
    return next_object_id_++;
}

void pdf_document_impl::add_object(std::unique_ptr<pdf_object> obj) {
    objects_.push_back(std::move(obj));
}

int pdf_document_impl::get_font_id(const FontStyle& font) {
    std::string font_key = font.family;
    if (font.bold) font_key += "-Bold";
    if (font.italic) font_key += "-Italic";

    auto it = font_cache_.find(font_key);
    if (it != font_cache_.end()) {
        return it->second;
    }

    int font_id = next_object_id();
    font_cache_[font_key] = font_id;

    // 对于中文字体，使用系统字体
    std::string pdf_font_name = font.family;
    if (font.family == "Arial" || font.family == "微软雅黑") {
        pdf_font_name = "Arial-Unicode-MS";  // 支持中文的字体
    }

    auto pdf_font_obj = std::make_unique<pdf_font>(font_id, pdf_font_name);
    add_object(std::move(pdf_font_obj));

    return font_id;
}

std::string pdf_document_impl::color_to_pdf(const Color& color) {
    // 简化实现：返回黑色
    return "0 0 0";
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
            case '\r':
                result += "\\r";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
                break;
        }
    }

    return result;
}

std::string pdf_document_impl::text_to_hex(const std::string& text) {
    std::ostringstream hex_stream;
    hex_stream << std::hex << std::uppercase;

    for (unsigned char c : text) {
        hex_stream << std::setfill('0') << std::setw(2) << static_cast<int>(c);
    }

    return hex_stream.str();
}

std::string pdf_document_impl::generate_pdf_document() {
    std::ostringstream pdf;
    std::vector<size_t> object_offsets;

    // PDF头部
    pdf << "%PDF-1.4\n";

    // 1. 目录对象
    object_offsets.push_back(pdf.tellp());
    pdf << "1 0 obj\n";
    pdf << "<<\n";
    pdf << "/Type /Catalog\n";
    pdf << "/Pages 2 0 R\n";
    pdf << ">>\n";
    pdf << "endobj\n";

    // 2. 页面树对象
    object_offsets.push_back(pdf.tellp());
    pdf << "2 0 obj\n";
    pdf << "<<\n";
    pdf << "/Type /Pages\n";
    pdf << "/Kids [";

    // 添加所有页面引用
    for (size_t i = 0; i < pages_.size(); ++i) {
        if (i > 0) pdf << " ";
        pdf << (3 + i) << " 0 R";  // 页面对象从3开始
    }

    pdf << "]\n";
    pdf << "/Count " << pages_.size() << "\n";
    pdf << ">>\n";
    pdf << "endobj\n";

    // 3. 页面对象和内容流
    for (size_t page_idx = 0; page_idx < pages_.size(); ++page_idx) {
        auto& page = pages_[page_idx];

        // 页面对象
        object_offsets.push_back(pdf.tellp());
        pdf << (3 + page_idx) << " 0 obj\n";
        pdf << "<<\n";
        pdf << "/Type /Page\n";
        pdf << "/Parent 2 0 R\n";
        pdf << "/MediaBox [0 0 " << page->width() << " " << page->height() << "]\n";

        // 字体资源
        if (!font_cache_.empty()) {
            pdf << "/Resources <<\n";
            pdf << "/Font <<\n";
            for (const auto& [font_name, font_id] : font_cache_) {
                pdf << "/F" << font_id << " " << font_id << " 0 R\n";
            }
            pdf << ">>\n";
            pdf << ">>\n";
        }

        // 内容流引用
        int content_obj_id = 3 + static_cast<int>(pages_.size()) + static_cast<int>(page_idx);
        pdf << "/Contents " << content_obj_id << " 0 R\n";
        pdf << ">>\n";
        pdf << "endobj\n";
    }

    // 4. 内容流对象
    for (size_t page_idx = 0; page_idx < pages_.size(); ++page_idx) {
        auto& page = pages_[page_idx];

        // 收集页面内容
        std::ostringstream content_stream;
        for (const auto& content : page->get_content_streams()) {
            content_stream << content << "\n";
        }

        std::string content_str = content_stream.str();

        object_offsets.push_back(pdf.tellp());
        int content_obj_id = 3 + static_cast<int>(pages_.size()) + static_cast<int>(page_idx);
        pdf << content_obj_id << " 0 obj\n";
        pdf << "<<\n";
        pdf << "/Length " << content_str.length() << "\n";
        pdf << ">>\n";
        pdf << "stream\n";
        pdf << content_str;
        pdf << "endstream\n";
        pdf << "endobj\n";
    }

    // 5. 字体对象
    for (const auto& [font_name, font_id] : font_cache_) {
        object_offsets.push_back(pdf.tellp());
        pdf << font_id << " 0 obj\n";
        pdf << "<<\n";
        pdf << "/Type /Font\n";
        pdf << "/Subtype /Type1\n";
        pdf << "/BaseFont /" << font_name << "\n";
        pdf << ">>\n";
        pdf << "endobj\n";
    }

    // 6. 交叉引用表
    size_t xref_offset = pdf.tellp();
    pdf << "xref\n";
    pdf << "0 " << (object_offsets.size() + 1) << "\n";
    pdf << "0000000000 65535 f \n";

    for (size_t offset : object_offsets) {
        pdf << std::setfill('0') << std::setw(10) << offset << " 00000 n \n";
    }

    // 7. 尾部
    pdf << "trailer\n";
    pdf << "<<\n";
    pdf << "/Size " << (object_offsets.size() + 1) << "\n";
    pdf << "/Root 1 0 R\n";
    pdf << ">>\n";
    pdf << "startxref\n";
    pdf << xref_offset << "\n";
    pdf << "%%EOF\n";

    return pdf.str();
}

std::pair<double, double> pdf_document_impl::page_size_to_points(PageSize size, PageOrientation orientation) {
    double width, height;
    
    switch (size) {
        case PageSize::A4:
            width = 595.0; height = 842.0;
            break;
        case PageSize::A3:
            width = 842.0; height = 1191.0;
            break;
        case PageSize::Letter:
            width = 612.0; height = 792.0;
            break;
        case PageSize::Legal:
            width = 612.0; height = 1008.0;
            break;
        default:
            width = 595.0; height = 842.0;  // 默认A4
            break;
    }
    
    if (orientation == PageOrientation::Landscape) {
        std::swap(width, height);
    }
    
    return {width, height};
}

// ========================================
// 辅助函数实现
// ========================================

Color excel_color_to_pdf_color(const Color& excel_color) {
    return excel_color;  // 简化实现
}

FontStyle excel_font_to_pdf_font(const std::string& font_name, double font_size, bool bold, bool italic) {
    FontStyle font;
    font.family = font_name;
    font.size = font_size;
    font.bold = bold;
    font.italic = italic;
    return font;
}

TextAlignment excel_alignment_to_pdf_alignment(int alignment) {
    // 简化实现
    return TextAlignment::Left;
}

} // namespace tinakit::pdf::internal
