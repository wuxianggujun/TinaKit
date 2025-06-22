/**
 * @file writer.cpp
 * @brief PDF核心写入器实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/core/writer.hpp"
#include "tinakit/pdf/core/object.hpp"
#include "tinakit/pdf/core/page.hpp"
#include "tinakit/pdf/binary_writer.hpp"
#include "tinakit/core/logger.hpp"
#include "tinakit/core/unicode.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace tinakit::pdf::core {

// ========================================
// 构造函数和析构函数
// ========================================

Writer::Writer() {
    // 设置默认的字体回退机制
    setupDefaultFontFallbacks();
}

// ========================================
// 对象管理
// ========================================

int Writer::addObject(std::unique_ptr<PdfObject> obj) {
    int id = obj->getId();
    std::string type_name = obj->getTypeName();

    ObjectEntry entry;
    entry.object = std::move(obj);
    entry.written = false;
    entry.offset = 0;

    objects_[id] = std::move(entry);

    PDF_DEBUG("Added object " + std::to_string(id) + " (" + type_name + ") to objects map");

    return id;
}

int Writer::getNextObjectId() {
    return next_object_id_++;
}

PdfObject* Writer::getObject(int id) const {
    auto it = objects_.find(id);
    return (it != objects_.end()) ? it->second.object.get() : nullptr;
}

std::size_t Writer::getObjectCount() const {
    return objects_.size();
}

// ========================================
// 页面管理
// ========================================

PdfPage* Writer::createPage(double width, double height) {
    int page_id = getNextObjectId();
    auto page = std::make_unique<PdfPage>(page_id, width, height);
    PdfPage* page_ptr = page.get();
    
    // 将页面作为对象添加到对象管理中
    // 注意：这里我们需要创建一个包装器，因为PdfPage不是PdfObject
    // 实际的页面对象会在生成PDF时创建
    pages_.push_back(std::move(page));

    return page_ptr;
}

std::size_t Writer::getPageCount() const {
    return pages_.size();
}

PdfPage* Writer::getPage(std::size_t index) const {
    return (index < pages_.size()) ? pages_[index].get() : nullptr;
}

const std::vector<std::unique_ptr<PdfPage>>& Writer::getPages() const {
    return pages_;
}

// ========================================
// 资源管理
// ========================================

std::string Writer::registerFont(const std::string& font_name,
                                const std::vector<std::uint8_t>& font_data,
                                bool embed_font) {
    PDF_DEBUG("Registering font: " + font_name);

    // 检查字体是否已注册
    auto it = font_resources_.find(font_name);
    if (it != font_resources_.end()) {
        PDF_DEBUG("Font already registered: " + it->second);
        return it->second;
    }

    // 生成字体资源ID
    std::string resource_id = "F" + std::to_string(next_resource_id_++);
    font_resources_[font_name] = resource_id;

    // 创建字体对象
    int font_obj_id = getNextObjectId();
    font_object_ids_[font_name] = font_obj_id;  // 记录对象ID

    PDF_DEBUG("Font registered: " + font_name + " -> " + resource_id + " (obj " + std::to_string(font_obj_id) + ")");

    // 检查是否需要Unicode支持（用于中文等）
    bool needsUnicode = (font_name != "Helvetica" && font_name != "Times-Roman" &&
                        font_name != "Courier" && font_name != "Symbol" && font_name != "ZapfDingbats");

    std::unique_ptr<FontObject> font_obj;

    if (needsUnicode || !font_data.empty()) {
        // 创建Type0字体（支持Unicode）
        font_obj = std::make_unique<FontObject>(font_obj_id, font_name, "Type0");
        font_obj->setEncoding("Identity-H");  // Unicode编码

        // 记录字体子类型
        font_subtypes_[font_name] = "Type0";

        // 创建FontDescriptor对象
        int font_desc_id = getNextObjectId();
        auto font_desc_obj = std::make_unique<FontDescriptorObject>(font_desc_id, font_name);

        // 字体嵌入处理
        if (embed_font) {
            if (!font_data.empty()) {
                // 使用提供的字体数据
                int font_file_id = getNextObjectId();
                auto font_file_obj = std::make_unique<FontFileObject>(font_file_id, font_data, "FontFile2");
                addObject(std::move(font_file_obj));

                font_desc_obj->setFontFile(font_file_id, "FontFile2");
                PDF_DEBUG("Font embedded: " + font_name + " (FontFile ID: " + std::to_string(font_file_id) + ")");
            } else {
                // 尝试自动加载系统字体
                std::string font_path = getSystemFontPath(font_name);
                if (!font_path.empty()) {
                    std::vector<uint8_t> auto_font_data = loadFontFile(font_path);
                    if (!auto_font_data.empty()) {
                        int font_file_id = getNextObjectId();
                        auto font_file_obj = std::make_unique<FontFileObject>(font_file_id, auto_font_data, "FontFile2");
                        addObject(std::move(font_file_obj));

                        font_desc_obj->setFontFile(font_file_id, "FontFile2");
                        PDF_DEBUG("System font auto-embedded: " + font_name + " from " + font_path);
                    } else {
                        PDF_DEBUG("Font embedding failed, using reference-only: " + font_name);
                    }
                } else {
                    PDF_DEBUG("System font not found, using reference-only: " + font_name);
                }
            }
        } else {
            PDF_DEBUG("Font embedding disabled, using reference-only: " + font_name);
        }

        addObject(std::move(font_desc_obj));

        // 创建CIDFont对象
        int cid_font_id = getNextObjectId();
        auto cid_font_obj = std::make_unique<CIDFontObject>(cid_font_id, font_name, "CIDFontType0");
        cid_font_obj->setCIDSystemInfo("Adobe", "Identity", 0);
        cid_font_obj->setDefaultWidth(1000);
        cid_font_obj->setFontDescriptor(font_desc_id);  // 设置FontDescriptor引用

        // 将CIDFont添加到对象列表
        addObject(std::move(cid_font_obj));

        // 创建简化的ToUnicode CMap
        int tounicode_id = getNextObjectId();
        auto tounicode_obj = std::make_unique<StreamObject>(tounicode_id);

        // 创建简化的Identity CMap
        std::string cmap_content =
            "/CIDInit /ProcSet findresource begin\n"
            "12 dict begin\n"
            "begincmap\n"
            "/CIDSystemInfo\n"
            "<< /Registry (Adobe)\n"
            "/Ordering (UCS)\n"
            "/Supplement 0\n"
            ">> def\n"
            "/CMapName /Adobe-Identity-UCS def\n"
            "/CMapType 2 def\n"
            "1 begincodespacerange\n"
            "<0000> <FFFF>\n"
            "endcodespacerange\n"
            "1 beginbfrange\n"
            "<0000> <FFFF> <0000>\n"
            "endbfrange\n"
            "endcmap\n"
            "CMapName currentdict /CMap defineresource pop\n"
            "end\n"
            "end\n";

        tounicode_obj->setStreamData(cmap_content);
        addObject(std::move(tounicode_obj));

        // 设置Type0字体的DescendantFonts和ToUnicode引用
        font_obj->setDescendantFont(cid_font_id);
        font_obj->setToUnicode(tounicode_id);

        PDF_DEBUG("Created Type0 font with CIDFont (ID: " + std::to_string(cid_font_id) + ") and ToUnicode (ID: " + std::to_string(tounicode_id) + ") for Unicode support");
    } else {
        // 创建Type1字体（标准字体）
        font_obj = std::make_unique<FontObject>(font_obj_id, font_name, "Type1");
        font_obj->setEncoding("WinAnsiEncoding");

        // 记录字体子类型
        font_subtypes_[font_name] = "Type1";

        PDF_DEBUG("Created Type1 font with WinAnsiEncoding");
    }

    addObject(std::move(font_obj));

    return resource_id;
}

std::string Writer::getFontResourceId(const std::string& font_name) const {
    auto it = font_resources_.find(font_name);
    return (it != font_resources_.end()) ? it->second : "";
}

std::string Writer::getFontSubtype(const std::string& font_name) const {
    auto it = font_subtypes_.find(font_name);
    return (it != font_subtypes_.end()) ? it->second : "";
}

void Writer::registerFontFallback(const std::string& primary_font, const std::string& fallback_font) {
    font_fallbacks_[primary_font] = fallback_font;
    PDF_DEBUG("Font fallback registered: " + primary_font + " -> " + fallback_font);

    // 确保回退字体也被注册
    if (font_resources_.find(fallback_font) == font_resources_.end()) {
        registerFont(fallback_font);
        PDF_DEBUG("Fallback font auto-registered: " + fallback_font);
    }
}

std::string Writer::getFallbackFont(const std::string& font_name) const {
    auto it = font_fallbacks_.find(font_name);
    return (it != font_fallbacks_.end()) ? it->second : "";
}

std::string Writer::selectBestFont(const std::string& text, const std::string& preferred_font) const {
    if (text.empty()) {
        return preferred_font;
    }

    // 检查文本是否包含CJK字符
    bool has_cjk = false;
    bool has_ascii = false;

    has_ascii = !tinakit::core::unicode::contains_non_ascii(text);
    has_cjk = tinakit::core::unicode::contains_cjk(text);

    // 根据文本内容选择最佳字体
    if (has_cjk) {
        // 包含CJK字符，优先使用支持Unicode的字体
        if (isUnicodeFont(preferred_font)) {
            return preferred_font;
        } else {
            // 首选字体不支持Unicode，查找回退字体
            std::string fallback = getFallbackFont(preferred_font);
            if (!fallback.empty() && isUnicodeFont(fallback)) {
                PDF_DEBUG("Using fallback font for CJK text: " + fallback);
                return fallback;
            } else {
                // 没有合适的回退字体，使用默认的CJK字体
                return "SimSun";  // 或其他默认CJK字体
            }
        }
    } else if (has_ascii) {
        // 纯ASCII文本，可以使用任何字体
        return preferred_font;
    }

    return preferred_font;
}

std::string Writer::registerImage(const std::string& image_path) {
    // 加载图像文件
    ImageData image_data = loadImageFile(image_path);
    if (image_data.data.empty()) {
        PDF_ERROR("Failed to load image: " + image_path);
        return "";
    }

    // 使用加载的数据注册图像
    return registerImage(image_data.data, image_data.width, image_data.height, image_data.format);
}

std::string Writer::registerImage(const std::vector<std::uint8_t>& image_data,
                                int width, int height, const std::string& format) {
    if (image_data.empty() || width <= 0 || height <= 0) {
        PDF_ERROR("Invalid image parameters");
        return "";
    }

    std::string resource_id = "Im" + std::to_string(next_resource_id_++);

    // 创建图像对象
    int image_obj_id = getNextObjectId();

    // 确定颜色空间
    std::string color_space = "DeviceRGB";  // 默认RGB
    if (format == "JPEG" || format == "PNG") {
        // 根据数据大小推断颜色空间
        size_t expected_rgb_size = width * height * 3;
        size_t expected_rgba_size = width * height * 4;
        size_t expected_gray_size = width * height * 1;

        if (image_data.size() == expected_gray_size) {
            color_space = "DeviceGray";
        } else if (image_data.size() == expected_rgb_size) {
            color_space = "DeviceRGB";
        } else if (image_data.size() == expected_rgba_size) {
            color_space = "DeviceRGB";  // 忽略Alpha通道
        }
    }

    auto image_obj = std::make_unique<ImageObject>(image_obj_id, image_data, width, height, color_space);
    addObject(std::move(image_obj));

    // 记录图像资源
    image_resources_[resource_id] = resource_id;
    image_object_ids_[resource_id] = image_obj_id;

    PDF_DEBUG("Image registered: " + resource_id + " (Object ID: " + std::to_string(image_obj_id) +
              ", " + std::to_string(width) + "x" + std::to_string(height) + ", " + format + ")");

    return resource_id;
}

// ========================================
// 文档属性
// ========================================

void Writer::setDocumentInfo(const std::string& title,
                            const std::string& author,
                            const std::string& subject,
                            const std::string& creator) {
    doc_info_.title = title;
    doc_info_.author = author;
    doc_info_.subject = subject;
    doc_info_.creator = creator;
}

void Writer::setPdfVersion(const std::string& version) {
    pdf_version_ = version;
}

// ========================================
// 文件生成
// ========================================

void Writer::saveToFile(const std::string& filename) {
    auto data = saveToBuffer();
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法创建文件: " + filename);
    }
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
}

std::vector<std::uint8_t> Writer::saveToBuffer() {
    BinaryWriter writer;  // 使用内存模式
    writeTo(writer);
    return writer.getBuffer();
}

void Writer::writeTo(BinaryWriter& writer) {
    PDF_DEBUG("Starting PDF generation");

    // 1. 创建Catalog对象（但不创建Pages对象，因为需要先知道页面ID）
    createCatalogObject();
    createInfoObject();

    PDF_DEBUG("Created catalog (ID: " + std::to_string(catalog_object_id_) + "), pages will be (ID: " + std::to_string(pages_object_id_) + ")");

    // 清空并重新分配页面对象ID
    page_object_ids_.clear();

    // 2. 为每个页面创建对象
    for (size_t i = 0; i < pages_.size(); ++i) {
        auto& page = pages_[i];
        int content_id = getNextObjectId();
        auto content_obj = page->createContentObject(content_id);
        addObject(std::move(content_obj));

        // 重新分配页面对象ID（不使用页面创建时的旧ID）
        int page_id = getNextObjectId();
        page_object_ids_.push_back(page_id);  // 记录新的页面对象ID

        // 创建页面对象时直接使用新的ID
        auto page_obj = page->createPageObjectWithId(page_id, pages_object_id_, content_id, generateResourceDict());
        addObject(std::move(page_obj));

        PDF_DEBUG("Created page " + std::to_string(i) + ": page_id=" + std::to_string(page_id) + ", content_id=" + std::to_string(content_id));
    }

    // 3. 现在创建Pages对象（包含所有页面引用）
    createPagesObject();

    PDF_DEBUG("Total objects before writing: " + std::to_string(objects_.size()));

    // 4. 写入PDF结构
    writeHeader(writer);
    writeObjects(writer);
    auto xref_offset = writeXrefTable(writer);
    writeTrailer(writer, xref_offset);
}

// ========================================
// 调试和验证
// ========================================

std::string Writer::validate() const {
    std::vector<std::string> errors;
    
    if (pages_.empty()) {
        errors.push_back("文档没有页面");
    }
    
    if (catalog_object_id_ == 0) {
        errors.push_back("缺少目录对象");
    }
    
    if (pages_object_id_ == 0) {
        errors.push_back("缺少页面树对象");
    }
    
    if (!errors.empty()) {
        std::ostringstream oss;
        for (const auto& error : errors) {
            oss << error << "; ";
        }
        return oss.str();
    }
    
    return "";  // 无错误
}

std::string Writer::getStatistics() const {
    std::ostringstream oss;
    oss << "PDF统计信息:\n";
    oss << "  对象数量: " << objects_.size() << "\n";
    oss << "  页面数量: " << pages_.size() << "\n";
    oss << "  字体资源: " << font_resources_.size() << "\n";
    oss << "  图像资源: " << image_resources_.size() << "\n";
    oss << "  PDF版本: " << pdf_version_ << "\n";
    return oss.str();
}

void Writer::setDebugMode(bool enable) {
    debug_mode_ = enable;
}

// ========================================
// 内部方法
// ========================================

void Writer::createCatalogObject() {
    if (catalog_object_id_ == 0) {
        catalog_object_id_ = getNextObjectId();
        pages_object_id_ = getNextObjectId();  // 预分配页面树ID
        
        auto catalog = std::make_unique<CatalogObject>(catalog_object_id_, pages_object_id_);
        addObject(std::move(catalog));
    }
}

void Writer::createPagesObject() {
    if (pages_object_id_ == 0) {
        pages_object_id_ = getNextObjectId();
    }

    auto pages_obj = std::make_unique<PagesObject>(pages_object_id_);

    // 添加所有页面引用（使用新分配的页面对象ID）
    for (int page_id : page_object_ids_) {
        pages_obj->addPageReference(page_id);
    }

    PDF_DEBUG("Pages object created with " + std::to_string(page_object_ids_.size()) + " page references");

    addObject(std::move(pages_obj));
}

void Writer::createInfoObject() {
    int info_id = getNextObjectId();
    auto info_obj = std::make_unique<InfoObject>(info_id);
    
    if (!doc_info_.title.empty()) info_obj->setTitle(doc_info_.title);
    if (!doc_info_.author.empty()) info_obj->setAuthor(doc_info_.author);
    if (!doc_info_.subject.empty()) info_obj->setSubject(doc_info_.subject);
    if (!doc_info_.creator.empty()) info_obj->setCreator(doc_info_.creator);
    info_obj->setProducer(doc_info_.producer);
    
    addObject(std::move(info_obj));
}

void Writer::writeHeader(BinaryWriter& writer) {
    writer.writeLine("%PDF-" + pdf_version_);
    writer.writeLine("%âãÏÓ");  // 二进制注释
}

void Writer::writeObjects(BinaryWriter& writer) {
    PDF_DEBUG("Writing " + std::to_string(objects_.size()) + " objects:");
    for (auto& [id, entry] : objects_) {
        PDF_DEBUG("Writing object " + std::to_string(id));
        entry.offset = writer.getOffsetLong();
        entry.object->writeTo(writer);
        entry.written = true;

        if (debug_mode_) {
            writer.writeComment("对象 " + std::to_string(id) + " 结束");
        }
    }
    PDF_DEBUG("All objects written");
}

std::size_t Writer::writeXrefTable(BinaryWriter& writer) {
    std::size_t xref_offset = writer.getOffsetLong();
    
    writer.writeLine("xref");
    writer.writeLine("0 " + std::to_string(objects_.size() + 1));
    writer.writeLine("0000000000 65535 f ");
    
    for (const auto& [id, entry] : objects_) {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(10) << entry.offset << " 00000 n ";
        writer.writeLine(oss.str());
    }
    
    return xref_offset;
}

void Writer::writeTrailer(BinaryWriter& writer, std::size_t xref_offset) {
    writer.writeLine("trailer");
    writer.writeLine("<<");
    writer.writeLine("/Size " + std::to_string(objects_.size() + 1));
    writer.writeLine("/Root " + std::to_string(catalog_object_id_) + " 0 R");
    writer.writeLine(">>");
    writer.writeLine("startxref");
    writer.writeLine(std::to_string(xref_offset));
    writer.writeLine("%%EOF");
}

std::string Writer::generateResourceDict() const {
    if (font_resources_.empty()) {
        PDF_DEBUG("No font resources, returning empty resource dict");
        return "<<>>";
    }

    std::ostringstream oss;
    oss << "<<\n/Font <<\n";

    for (const auto& [font_name, resource_id] : font_resources_) {
        auto it = font_object_ids_.find(font_name);
        if (it != font_object_ids_.end()) {
            oss << "/" << resource_id << " " << it->second << " 0 R\n";
            PDF_DEBUG("Added font to resource dict: /" + resource_id + " " + std::to_string(it->second) + " 0 R");
        } else {
            PDF_ERROR("Font object ID not found for: " + font_name);
        }
    }

    oss << ">>\n>>";
    std::string result = oss.str();
    PDF_DEBUG("Generated resource dict: " + result);
    return result;
}

bool Writer::isUnicodeFont(const std::string& font_name) const {
    // 检查字体子类型
    std::string subtype = getFontSubtype(font_name);
    return (subtype == "Type0");
}

bool Writer::isCJKCharacter(uint32_t codepoint) const {
    return tinakit::core::unicode::is_cjk_character(codepoint);
}

void Writer::setupDefaultFontFallbacks() {
    // 为常用的西文字体设置中文回退字体
    registerFontFallback("Helvetica", "SimSun");
    registerFontFallback("Times-Roman", "SimSun");
    registerFontFallback("Courier", "SimSun");
    registerFontFallback("Arial", "SimSun");
    registerFontFallback("Times New Roman", "SimSun");

    // 为中文字体设置英文回退字体（虽然通常不需要，但为了完整性）
    registerFontFallback("SimSun", "Helvetica");
    registerFontFallback("NSimSun", "Helvetica");
    registerFontFallback("Microsoft YaHei", "Helvetica");
    registerFontFallback("微软雅黑", "Helvetica");

    PDF_DEBUG("Default font fallbacks configured");
}

} // namespace tinakit::pdf::core
