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
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace tinakit::pdf::core {

// ========================================
// 构造函数和析构函数
// ========================================

Writer::Writer() {
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
                                const std::vector<std::uint8_t>& font_data) {
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

        // 创建FontDescriptor对象
        int font_desc_id = getNextObjectId();
        auto font_desc_obj = std::make_unique<FontDescriptorObject>(font_desc_id, font_name);
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
        PDF_DEBUG("Created Type1 font with WinAnsiEncoding");
    }

    addObject(std::move(font_obj));

    return resource_id;
}

std::string Writer::getFontResourceId(const std::string& font_name) const {
    auto it = font_resources_.find(font_name);
    return (it != font_resources_.end()) ? it->second : "";
}

std::string Writer::registerImage(const std::vector<std::uint8_t>& image_data,
                                int width, int height, const std::string& format) {
    // 避免未使用参数警告
    (void)image_data;
    (void)width;
    (void)height;
    (void)format;

    std::string resource_id = "Im" + std::to_string(next_resource_id_++);
    image_resources_[resource_id] = resource_id;

    // TODO: 实现图像对象创建
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

} // namespace tinakit::pdf::core
