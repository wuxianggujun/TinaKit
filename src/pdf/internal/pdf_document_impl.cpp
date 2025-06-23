/**
 * @file pdf_document_impl.cpp
 * @brief PDF文档内部实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/internal/pdf_document_impl.hpp"
#include "tinakit/pdf/core/writer.hpp"
#include "tinakit/core/logger.hpp"
#include "tinakit/core/image.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>

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
    PDF_DEBUG("add_text called: text='" + text + "', pos=(" + std::to_string(position.x) + "," + std::to_string(position.y) + "), font=" + font.family);

    auto page = current_page();
    if (!page) {
        PDF_ERROR("No current page!");
        return;
    }
    PDF_DEBUG("Current page found");

    // 直接使用指定字体，不进行字体回退
    std::string font_resource = get_font_resource_id_for_font_name(font.family);
    std::string font_subtype = writer_->getFontSubtype(font.family);
    PDF_DEBUG("Font resource ID: " + font_resource);
    PDF_DEBUG("Font subtype: " + font_subtype);

    // 转换坐标系：从左上角坐标系转换为PDF的左下角坐标系
    double pdf_y = page_height_ - position.y;

    PDF_DEBUG("Coordinate conversion: input(" + std::to_string(position.x) + "," + std::to_string(position.y) +
              ") -> PDF(" + std::to_string(position.x) + "," + std::to_string(pdf_y) +
              "), page_height=" + std::to_string(page_height_));

    // 添加文本到页面
    page->beginText();
    page->setFont(font_resource, font.size, font_subtype);
    page->setTextPosition(position.x, pdf_y);
    page->setTextColor(font.color.red() / 255.0, font.color.green() / 255.0, font.color.blue() / 255.0);

    // 尝试使用GID编码，如果失败则回退到UTF-16BE
    auto* font_manager = writer_->getFontManager();
    if (font_manager && font_manager->isFontLoaded(font.family)) {
        page->showTextWithGID(text, font.family, font_manager);
        PDF_DEBUG("Used GID encoding for text: " + text);
    } else {
        page->showText(text);
        PDF_DEBUG("Used UTF-16BE encoding for text: " + text);
    }

    page->endText();

    PDF_DEBUG("Text added successfully with font: " + font.family);
}

void pdf_document_impl::add_text_block(const std::string& text, const Rect& bounds,
                                      const Font& font, TextAlignment alignment) {
    // 避免未使用参数警告
    (void)alignment;

    auto page = current_page();
    if (!page) {
        return;
    }

    // 获取或注册字体
    std::string font_resource = get_font_resource_id(font);
    std::string font_subtype = writer_->getFontSubtype(font.family);

    // 简单实现：将文本放在边界框的左上角
    // TODO: 实现文本换行和对齐
    page->beginText();
    page->setFont(font_resource, font.size, font_subtype);
    page->setTextPosition(bounds.x, bounds.y + bounds.height - font.size);
    page->setTextColor(font.color.red() / 255.0, font.color.green() / 255.0, font.color.blue() / 255.0);
    page->showText(text);
    page->endText();
}







// ========================================
// 字体管理
// ========================================

std::string pdf_document_impl::register_font(const std::string& font_name,
                                            const std::vector<std::uint8_t>& font_data,
                                            bool embed_font) {
    PDF_DEBUG("Registering font with data: " + font_name + " (" + std::to_string(font_data.size()) + " bytes)");

    // 启用新的字体子集化架构
    bool enable_subsetting = (font_data.size() > 1024 * 1024);  // 大于1MB的字体启用子集化

    if (enable_subsetting) {
        PDF_DEBUG("Enabling font subsetting with new architecture for: " + font_name);
        return writer_->registerFontWithSubsetting(font_name, font_data, true, embed_font);
    } else {
        PDF_DEBUG("Using standard font registration: " + font_name);
        return writer_->registerFont(font_name, font_data, embed_font);
    }
}

// ========================================
// 文件操作
// ========================================

void pdf_document_impl::save(const std::filesystem::path& file_path) {
    PDF_DEBUG("Saving PDF to: " + file_path.string());
    PDF_DEBUG("Page count: " + std::to_string(writer_->getPageCount()));

    if (writer_->getPageCount() == 0) {
        PDF_WARN("No pages, adding default page");
        add_page();
    }

    // 预先注册常用字体，确保资源字典不为空
    ensureCommonFontsRegistered();

    writer_->saveToFile(file_path.string());
    PDF_INFO("PDF saved successfully to: " + file_path.string());
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
    return get_font_resource_id_for_font_name(font.family);
}

std::string pdf_document_impl::get_font_resource_id_for_font_name(const std::string& font_name) {
    // 检查字体是否已注册
    std::string resource_id = writer_->getFontResourceId(font_name);
    if (resource_id.empty()) {
        // 注册新字体
        resource_id = writer_->registerFont(font_name);
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

void pdf_document_impl::ensureCommonFontsRegistered() {
    // 预先注册常用字体，确保资源字典不为空
    std::vector<std::string> common_fonts = {
        "SimSun",      // 中文字体
        "NSimSun",     // 中文字体
        "Helvetica",   // 西文字体
        "Arial",       // 西文字体
        "Times-Roman"  // 西文字体
    };

    for (const auto& font_name : common_fonts) {
        std::string resource_id = writer_->getFontResourceId(font_name);
        if (resource_id.empty()) {
            // 只注册实际使用的字体
            // 这里我们不预先注册，而是确保在使用时能正确注册
            PDF_DEBUG("Font " + font_name + " not yet registered");
        } else {
            PDF_DEBUG("Font " + font_name + " already registered as " + resource_id);
        }
    }
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
