/**
 * @file page.cpp
 * @brief PDF页面管理实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/core/page.hpp"
#include "tinakit/pdf/core/object.hpp"
#include "tinakit/pdf/core/writer.hpp"
#include "tinakit/core/logger.hpp"
#include "tinakit/core/unicode.hpp"
#include <sstream>
#include <iomanip>
#include <set>
#include <map>
#include <cmath>

namespace tinakit::pdf::core {

// ========================================
// 构造函数和析构函数
// ========================================

PdfPage::PdfPage(int id, double width, double height)
    : id_(id), width_(width), height_(height) {
    setMediaBox({0, 0, width, height});
}

// ========================================
// 页面属性
// ========================================

void PdfPage::setSize(double width, double height) {
    width_ = width;
    height_ = height;
    setMediaBox({0, 0, width, height});
}

std::vector<double> PdfPage::getMediaBox() const {
    return media_box_;
}

void PdfPage::setMediaBox(const std::vector<double>& media_box) {
    if (media_box.size() == 4) {
        media_box_ = media_box;
    }
}

// ========================================
// 内容管理
// ========================================

void PdfPage::addContent(const std::string& content) {
    content_stream_.push_back(content);
}

void PdfPage::addContentLine(const std::string& content) {
    content_stream_.push_back(content + "\n");
}

void PdfPage::clearContent() {
    content_stream_.clear();
    in_text_object_ = false;
    graphics_state_level_ = 0;
}

std::string PdfPage::getContentStream() const {
    std::ostringstream oss;
    for (const auto& line : content_stream_) {
        oss << line;
    }
    return oss.str();
}

std::size_t PdfPage::getContentSize() const {
    std::size_t total = 0;
    for (const auto& line : content_stream_) {
        total += line.length();
    }
    return total;
}

// ========================================
// 图形状态
// ========================================

void PdfPage::saveGraphicsState() {
    addContent("q\n");
    graphics_state_level_++;
}

void PdfPage::restoreGraphicsState() {
    if (graphics_state_level_ > 0) {
        addContent("Q\n");
        graphics_state_level_--;
    }
}

void PdfPage::setTransform(double a, double b, double c, double d, double e, double f) {
    std::ostringstream oss;
    oss << formatFloat(a) << " " << formatFloat(b) << " " 
        << formatFloat(c) << " " << formatFloat(d) << " "
        << formatFloat(e) << " " << formatFloat(f) << " cm\n";
    addContent(oss.str());
}

void PdfPage::translate(double dx, double dy) {
    setTransform(1, 0, 0, 1, dx, dy);
}

void PdfPage::scale(double sx, double sy) {
    setTransform(sx, 0, 0, sy, 0, 0);
}

void PdfPage::rotate(double angle) {
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    setTransform(cos_a, sin_a, -sin_a, cos_a, 0, 0);
}

// ========================================
// 文本操作
// ========================================

void PdfPage::beginText() {
    if (in_text_object_) {
        return;
    }
    addContent("BT\n");
    in_text_object_ = true;
}

void PdfPage::endText() {
    if (!in_text_object_) {
        return;
    }
    addContent("ET\n");
    in_text_object_ = false;
}

void PdfPage::setFont(const std::string& font_resource, double size, const std::string& subtype) {
    current_font_subtype_ = subtype;  // 记录当前字体类型
    std::ostringstream oss;
    oss << "/" << font_resource << " " << formatFloat(size) << " Tf\n";
    addContent(oss.str());
    PDF_DEBUG("Font set: " + font_resource + " (" + subtype + "), size=" + std::to_string(size));
}

void PdfPage::setTextPosition(double x, double y) {
    // 使用Tm命令进行绝对定位，而不是Td的相对移动
    std::ostringstream oss;
    oss << "1 0 0 1 " << formatFloat(x) << " " << formatFloat(y) << " Tm\n";
    addContent(oss.str());
    PDF_DEBUG("Text position set to absolute: (" + std::to_string(x) + "," + std::to_string(y) + ")");
}

void PdfPage::moveTextPosition(double dx, double dy) {
    std::ostringstream oss;
    oss << formatFloat(dx) << " " << formatFloat(dy) << " Td\n";
    addContent(oss.str());
}

void PdfPage::showText(const std::string& text) {
    PDF_DEBUG("PdfPage::showText: '" + text + "'");

    // 统一使用UTF-16BE编码，不再分段处理
    std::string hex_string = tinakit::core::unicode::utf8_to_utf16be_hex(text);
    addContent(hex_string + " Tj\n");

    PDF_DEBUG("Text rendered using UTF-16BE encoding: '" + text + "' -> " + hex_string);
}

void PdfPage::showTextWithGID(const std::string& text, const std::string& font_name, FontManager* font_manager) {
    PDF_DEBUG("PdfPage::showTextWithGID: '" + text + "' using font: " + font_name);

    // 记录文本使用情况（用于字体子集化）
    text_usage_.push_back({text, font_name});

    if (!font_manager || !font_manager->isFontLoaded(font_name)) {
        PDF_WARN("FontManager not available or font not loaded, falling back to UTF-16BE");
        showText(text);
        return;
    }

    // 使用FontManager将文本转换为GID十六进制字符串
    std::string gid_hex_string = font_manager->textToGIDHex(font_name, text);

    if (gid_hex_string.empty()) {
        PDF_WARN("Failed to convert text to GID, falling back to UTF-16BE");
        showText(text);
        return;
    }

    // 直接使用GID十六进制字符串
    addContent(gid_hex_string + " Tj\n");

    PDF_DEBUG("Text rendered using GID encoding");
}

void PdfPage::showTextLine(const std::string& text) {
    std::ostringstream oss;
    oss << "(" << tinakit::core::unicode::escape_string(text) << ") '\n";
    addContent(oss.str());
}

void PdfPage::setTextColor(double r, double g, double b) {
    std::ostringstream oss;
    oss << formatFloat(r) << " " << formatFloat(g) << " " << formatFloat(b) << " rg\n";
    addContent(oss.str());
}

// ========================================
// 图形操作
// ========================================

void PdfPage::setLineWidth(double width) {
    std::ostringstream oss;
    oss << formatFloat(width) << " w\n";
    addContent(oss.str());
}

void PdfPage::setStrokeColor(double r, double g, double b) {
    std::ostringstream oss;
    oss << formatFloat(r) << " " << formatFloat(g) << " " << formatFloat(b) << " RG\n";
    addContent(oss.str());
}

void PdfPage::setFillColor(double r, double g, double b) {
    std::ostringstream oss;
    oss << formatFloat(r) << " " << formatFloat(g) << " " << formatFloat(b) << " rg\n";
    addContent(oss.str());
}

void PdfPage::moveTo(double x, double y) {
    std::ostringstream oss;
    oss << formatFloat(x) << " " << formatFloat(y) << " m\n";
    addContent(oss.str());
}

void PdfPage::lineTo(double x, double y) {
    std::ostringstream oss;
    oss << formatFloat(x) << " " << formatFloat(y) << " l\n";
    addContent(oss.str());
}

void PdfPage::rectangle(double x, double y, double width, double height) {
    std::ostringstream oss;
    oss << formatFloat(x) << " " << formatFloat(y) << " " 
        << formatFloat(width) << " " << formatFloat(height) << " re\n";
    addContent(oss.str());
}

void PdfPage::stroke() {
    addContent("S\n");
}

void PdfPage::fill() {
    addContent("f\n");
}

void PdfPage::fillAndStroke() {
    addContent("B\n");
}

void PdfPage::closePath() {
    addContent("h\n");
}

// ========================================
// 图像操作
// ========================================

void PdfPage::addImage(const std::string& image_resource, double x, double y, double width, double height) {
    PDF_DEBUG("Adding image: " + image_resource + " at (" + std::to_string(x) + "," + std::to_string(y) +
              ") size " + std::to_string(width) + "x" + std::to_string(height));

    // 保存当前图形状态
    saveGraphicsState();

    // 设置变换矩阵：缩放到指定尺寸并移动到指定位置
    // PDF图像默认是1x1单位，需要缩放到实际尺寸
    std::ostringstream oss;
    oss << formatFloat(width) << " 0 0 " << formatFloat(height) << " "
        << formatFloat(x) << " " << formatFloat(y) << " cm\n";
    addContent(oss.str());

    // 绘制图像
    oss.str("");
    oss << "/" << image_resource << " Do\n";
    addContent(oss.str());

    // 恢复图形状态
    restoreGraphicsState();

    PDF_DEBUG("Image added successfully");
}

// ========================================
// 高级操作
// ========================================

void PdfPage::addComment(const std::string& comment) {
    addContent("% " + comment + "\n");
}

std::unique_ptr<DictionaryObject> PdfPage::createPageObject(int parent_id, int content_id,
                                                           const std::string& resources) const {
    return createPageObjectWithId(id_, parent_id, content_id, resources);
}

std::unique_ptr<DictionaryObject> PdfPage::createPageObjectWithId(int page_id, int parent_id, int content_id,
                                                                 const std::string& resources) const {
    auto page_obj = std::make_unique<DictionaryObject>(page_id);

    page_obj->set("Type", "/Page");
    page_obj->setReference("Parent", parent_id);
    page_obj->setReference("Contents", content_id);

    // 设置MediaBox
    std::vector<std::string> media_box_values;
    for (double value : media_box_) {
        media_box_values.push_back(formatFloat(value));
    }
    page_obj->setArray("MediaBox", media_box_values);

    // 设置资源字典
    if (!resources.empty()) {
        page_obj->set("Resources", resources);
        PDF_DEBUG("Page object resources set to: " + resources);
    } else {
        page_obj->set("Resources", "<<>>");
        PDF_DEBUG("Page object resources set to empty dict");
    }

    PDF_DEBUG("Page object created with ID=" + std::to_string(page_id) + ", Parent=" + std::to_string(parent_id) + ", Contents=" + std::to_string(content_id));

    return page_obj;
}

std::unique_ptr<StreamObject> PdfPage::createContentObject(int content_id) const {
    std::string content = getContentStream();
    PDF_DEBUG("Page content stream (" + std::to_string(content.length()) + " bytes):");
    PDF_DEBUG("--- Content Start ---");
    PDF_DEBUG(content);
    PDF_DEBUG("--- Content End ---");

    auto content_obj = std::make_unique<StreamObject>(content_id);
    content_obj->setStreamData(content);

    PDF_DEBUG("StreamObject created with ID=" + std::to_string(content_id) + ", data size=" + std::to_string(content.length()));

    return content_obj;
}

// ========================================
// 内部方法
// ========================================

// escapeText函数已移除，统一使用tinakit::core::unicode::escape_pdf_string()

std::string PdfPage::formatFloat(double value, int precision) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    std::string result = oss.str();

    // 移除尾随的零和小数点
    if (result.find('.') != std::string::npos) {
        result.erase(result.find_last_not_of('0') + 1);
        if (result.back() == '.') {
            result.pop_back();
        }
    }

    return result;
}

void PdfPage::ensureFontsRegistered(Writer& writer) {
    // 不再自动注册预定义字体，只确保实际使用的字体已注册
    // 字体应该在使用前通过 Document::register_font 显式注册

    PDF_DEBUG("Page font registration check completed - using only explicitly registered fonts");
}

std::set<uint32_t> PdfPage::collectUsedCodepoints(const std::string& font_name) const {
    std::set<uint32_t> codepoints;

    for (const auto& usage : text_usage_) {
        // 如果指定了字体名称，只收集该字体的字符
        if (!font_name.empty() && usage.font_name != font_name) {
            continue;
        }

        // 将UTF-8文本转换为Unicode码点
        const std::string& text = usage.text;
        for (size_t i = 0; i < text.length(); ) {
            uint32_t codepoint = 0;
            size_t bytes = 1;

            unsigned char c = static_cast<unsigned char>(text[i]);

            if (c < 0x80) {
                // ASCII字符
                codepoint = c;
                bytes = 1;
            } else if ((c & 0xE0) == 0xC0) {
                // 2字节UTF-8
                if (i + 1 < text.length()) {
                    codepoint = ((c & 0x1F) << 6) | (text[i + 1] & 0x3F);
                    bytes = 2;
                }
            } else if ((c & 0xF0) == 0xE0) {
                // 3字节UTF-8
                if (i + 2 < text.length()) {
                    codepoint = ((c & 0x0F) << 12) |
                               ((text[i + 1] & 0x3F) << 6) |
                               (text[i + 2] & 0x3F);
                    bytes = 3;
                }
            } else if ((c & 0xF8) == 0xF0) {
                // 4字节UTF-8
                if (i + 3 < text.length()) {
                    codepoint = ((c & 0x07) << 18) |
                               ((text[i + 1] & 0x3F) << 12) |
                               ((text[i + 2] & 0x3F) << 6) |
                               (text[i + 3] & 0x3F);
                    bytes = 4;
                }
            }

            if (codepoint > 0) {
                codepoints.insert(codepoint);
            }

            i += bytes;
        }
    }

    PDF_DEBUG("Collected " + std::to_string(codepoints.size()) + " unique codepoints from page" +
              (font_name.empty() ? "" : " for font: " + font_name));

    return codepoints;
}

// 智能分段相关方法已移除，统一使用UTF-16BE编码

} // namespace tinakit::pdf::core
