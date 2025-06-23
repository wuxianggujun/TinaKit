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
    page->showText(text);
    page->endText();

    PDF_DEBUG("Text added successfully with font: " + font.family);
}

void pdf_document_impl::add_text_block(const std::string& text, const Rect& bounds,
                                      const Font& font, TextAlignment alignment) {
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

void pdf_document_impl::add_table(const Table& table, const Point& position) {
    // TODO: 实现表格渲染
}

// ========================================
// 图像功能
// ========================================

void pdf_document_impl::add_image(const std::string& image_path, const Point& position,
                                 double width, double height) {
    PDF_DEBUG("add_image called: path='" + image_path + "', pos=(" +
              std::to_string(position.x) + "," + std::to_string(position.y) + ")");

    auto page = current_page();
    if (!page) {
        PDF_ERROR("No current page!");
        return;
    }

    // 注册图像并获取资源ID
    std::string image_resource = writer_->registerImage(image_path);
    if (image_resource.empty()) {
        PDF_ERROR("Failed to register image: " + image_path);
        return;
    }

    // 如果未指定尺寸，使用原始尺寸
    // TODO: 从图像数据获取原始尺寸
    if (width <= 0) width = 100;  // 默认宽度
    if (height <= 0) height = 100; // 默认高度

    // 添加图像到页面
    page->addImage(image_resource, position.x, position.y, width, height);

    PDF_DEBUG("Image added successfully: " + image_resource);
}

void pdf_document_impl::add_image(const tinakit::core::Image& image, const Point& position,
                                 double width, double height) {
    PDF_DEBUG("add_image called with core::Image, pos=(" +
              std::to_string(position.x) + "," + std::to_string(position.y) + ")");

    auto page = current_page();
    if (!page) {
        PDF_ERROR("No current page!");
        return;
    }

    if (!image.isLoaded()) {
        PDF_ERROR("Image is not loaded!");
        return;
    }

    // 获取图像数据
    auto image_data = image.getDataCopy();
    int img_width = image.getWidth();
    int img_height = image.getHeight();
    int channels = image.getChannels();

    // 确定图像格式
    std::string format = "PNG"; // 默认格式，实际应该从图像数据推断

    // 注册图像并获取资源ID
    std::string image_resource = writer_->registerImage(image_data, img_width, img_height, format);
    if (image_resource.empty()) {
        PDF_ERROR("Failed to register image from core::Image");
        return;
    }

    // 如果未指定尺寸，使用原始尺寸或按比例缩放
    if (width <= 0 && height <= 0) {
        width = img_width;
        height = img_height;
    } else if (width <= 0) {
        width = height * img_width / img_height;
    } else if (height <= 0) {
        height = width * img_height / img_width;
    }

    // 添加图像到页面
    page->addImage(image_resource, position.x, position.y, width, height);

    PDF_DEBUG("Image added successfully from core::Image: " + image_resource);
}

void pdf_document_impl::add_image(const std::vector<std::uint8_t>& image_data,
                                 int width, int height, int channels,
                                 const Point& position,
                                 double display_width, double display_height) {
    PDF_DEBUG("add_image called with raw data: " + std::to_string(width) + "x" +
              std::to_string(height) + ", " + std::to_string(channels) + " channels");

    auto page = current_page();
    if (!page) {
        PDF_ERROR("No current page!");
        return;
    }

    if (image_data.empty() || width <= 0 || height <= 0 || channels <= 0) {
        PDF_ERROR("Invalid image data parameters");
        return;
    }

    // 确定图像格式
    std::string format = "PNG"; // 默认格式

    // 注册图像并获取资源ID
    std::string image_resource = writer_->registerImage(image_data, width, height, format);
    if (image_resource.empty()) {
        PDF_ERROR("Failed to register image from raw data");
        return;
    }

    // 如果未指定显示尺寸，使用原始尺寸
    if (display_width <= 0 && display_height <= 0) {
        display_width = width;
        display_height = height;
    } else if (display_width <= 0) {
        display_width = display_height * width / height;
    } else if (display_height <= 0) {
        display_height = display_width * height / width;
    }

    // 添加图像到页面
    page->addImage(image_resource, position.x, position.y, display_width, display_height);

    PDF_DEBUG("Image added successfully from raw data: " + image_resource);
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
