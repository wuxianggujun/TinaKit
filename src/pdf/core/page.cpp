/**
 * @file page.cpp
 * @brief PDF页面管理实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/core/page.hpp"
#include "tinakit/pdf/core/object.hpp"
#include "tinakit/core/logger.hpp"
#include <sstream>
#include <iomanip>
#include <utf8.h>
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
    std::ostringstream oss;
    oss << formatFloat(x) << " " << formatFloat(y) << " Td\n";
    addContent(oss.str());
}

void PdfPage::moveTextPosition(double dx, double dy) {
    std::ostringstream oss;
    oss << formatFloat(dx) << " " << formatFloat(dy) << " Td\n";
    addContent(oss.str());
}

void PdfPage::showText(const std::string& text) {
    PDF_DEBUG("PdfPage::showText: '" + text + "'");

    // 检查当前字体类型
    bool isUnicodeFont = (current_font_subtype_ == "Type0");

    if (!isUnicodeFont) {
        // 非Unicode字体，直接使用ASCII格式
        std::ostringstream oss;
        oss << "(" << escapeText(text) << ") Tj\n";
        addContent(oss.str());
        PDF_DEBUG("Using ASCII format for non-Unicode font");
        return;
    }

    // Unicode字体：检查是否包含中文
    if (!containsNonASCII(text)) {
        // 纯ASCII文本，使用括号格式避免间距问题
        std::ostringstream oss;
        oss << "(" << escapeText(text) << ") Tj\n";
        addContent(oss.str());
        PDF_DEBUG("Using ASCII format for pure ASCII text in Unicode font");
    } else {
        // 包含中文的文本，使用简单的分段处理
        std::string current_ascii;
        std::string current_unicode;
        bool in_unicode = false;

        auto it = text.begin();
        while (it != text.end()) {
            unsigned char c = static_cast<unsigned char>(*it);

            if (c <= 127) {
                // ASCII字符
                if (in_unicode && !current_unicode.empty()) {
                    // 输出之前的Unicode段
                    std::ostringstream oss;
                    oss << "<FEFF";
                    try {
                        std::u16string utf16_string;
                        utf8::utf8to16(current_unicode.begin(), current_unicode.end(), std::back_inserter(utf16_string));
                        for (char16_t ch : utf16_string) {
                            oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<uint16_t>(ch);
                        }
                    } catch (const std::exception&) {
                        // 回退处理
                    }
                    oss << "> Tj\n";
                    addContent(oss.str());
                    current_unicode.clear();
                }
                current_ascii += c;
                in_unicode = false;
                ++it;
            } else {
                // 非ASCII字符（可能是UTF-8多字节）
                if (!in_unicode && !current_ascii.empty()) {
                    // 输出之前的ASCII段
                    std::ostringstream oss;
                    oss << "(" << escapeText(current_ascii) << ") Tj\n";
                    addContent(oss.str());
                    current_ascii.clear();
                }

                // 读取完整的UTF-8字符
                auto char_start = it;
                try {
                    utf8::next(it, text.end());
                    current_unicode.append(char_start, it);
                } catch (const std::exception&) {
                    current_unicode += *it;
                    ++it;
                }
                in_unicode = true;
            }
        }

        // 输出剩余的段
        if (!current_ascii.empty()) {
            std::ostringstream oss;
            oss << "(" << escapeText(current_ascii) << ") Tj\n";
            addContent(oss.str());
        }
        if (!current_unicode.empty()) {
            std::ostringstream oss;
            oss << "<FEFF";
            try {
                std::u16string utf16_string;
                utf8::utf8to16(current_unicode.begin(), current_unicode.end(), std::back_inserter(utf16_string));
                for (char16_t ch : utf16_string) {
                    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<uint16_t>(ch);
                }
            } catch (const std::exception&) {
                // 回退处理
            }
            oss << "> Tj\n";
            addContent(oss.str());
        }

        PDF_DEBUG("Mixed text processed with inline segmentation");
    }
}

void PdfPage::showTextLine(const std::string& text) {
    std::ostringstream oss;
    oss << "(" << escapeText(text) << ") '\n";
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

std::string PdfPage::escapeText(const std::string& text) const {
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

} // namespace tinakit::pdf::core
