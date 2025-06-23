/**
 * @file writer.cpp
 * @brief PDF核心写入器实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/core/writer.hpp"
#include "tinakit/pdf/core/object.hpp"
#include "tinakit/pdf/core/page.hpp"
#include "tinakit/pdf/core/binary_writer.hpp"
#include "tinakit/core/logger.hpp"
#include "tinakit/core/unicode.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <set>
#include <filesystem>

namespace tinakit::pdf::core {

// ========================================
// 构造函数和析构函数
// ========================================

    Writer::Writer() : font_manager_(std::make_unique<FontManager>()),
                       subset_manager_(std::make_unique<FontSubsetManager>()) {
        // PDF写入器初始化
        PDF_DEBUG("PDF Writer initialized with FontManager and FontSubsetManager");
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

    PdfObject *Writer::getObject(int id) const {
        auto it = objects_.find(id);
        return (it != objects_.end()) ? it->second.object.get() : nullptr;
    }

    std::size_t Writer::getObjectCount() const {
        return objects_.size();
    }

// ========================================
// 页面管理
// ========================================

    PdfPage *Writer::createPage(double width, double height) {
        int page_id = getNextObjectId();
        auto page = std::make_unique<PdfPage>(page_id, width, height);
        PdfPage *page_ptr = page.get();

        // 设置Writer引用，用于字符使用记录
        page_ptr->setWriter(this);

        // 将页面作为对象添加到对象管理中
        // 注意：这里我们需要创建一个包装器，因为PdfPage不是PdfObject
        // 实际的页面对象会在生成PDF时创建
        pages_.push_back(std::move(page));

        return page_ptr;
    }

    std::size_t Writer::getPageCount() const {
        return pages_.size();
    }

    PdfPage *Writer::getPage(std::size_t index) const {
        return (index < pages_.size()) ? pages_[index].get() : nullptr;
    }

    const std::vector<std::unique_ptr<PdfPage>> &Writer::getPages() const {
        return pages_;
    }

// ========================================
// 资源管理
// ========================================

    std::string Writer::registerFont(const std::string &font_name,
                                     const std::vector<std::uint8_t> &font_data,
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

        // 使用FontManager加载字体
        bool font_loaded = false;
        if (!font_data.empty()) {
            font_loaded = font_manager_->loadFont(font_data, font_name);
        } else {
            // 尝试从系统路径加载字体
            std::string font_path = getSystemFontPath(font_name);
            if (!font_path.empty()) {
                font_loaded = font_manager_->loadFont(font_path, font_name);
            }
        }

        if (font_loaded) {
            PDF_DEBUG("Font loaded into FontManager: " + font_name);
        } else {
            PDF_WARN("Failed to load font into FontManager: " + font_name + ", using fallback");
        }

        PDF_DEBUG(
                "Font registered: " + font_name + " -> " + resource_id + " (obj " + std::to_string(font_obj_id) + ")");

        // 统一使用Type0字体以支持UTF-16BE编码
        std::unique_ptr<FontObject> font_obj;
        // 统一创建Type0字体（支持UTF-16BE编码）
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
                        auto font_file_obj = std::make_unique<FontFileObject>(font_file_id, auto_font_data,
                                                                              "FontFile2");
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

        // 创建CIDFont对象 - 使用CIDFontType2用于TrueType字体
        int cid_font_id = getNextObjectId();
        auto cid_font_obj = std::make_unique<CIDFontObject>(cid_font_id, font_name, "CIDFontType2");
        cid_font_obj->setCIDSystemInfo("Adobe", "Identity", 0);

        // 设置合理的默认宽度和字符宽度表
        // 根据字体类型设置不同的默认宽度
        bool is_cjk_font = (font_name == "SimSun" || font_name == "NSimSun" || font_name == "Microsoft YaHei" ||
                            font_name == "SourceHanSansSC-Regular" || font_name.find("SourceHan") != std::string::npos);
        int default_width = is_cjk_font ? 1000 : 500;  // CJK字体用1000，西文字体用500
        cid_font_obj->setDefaultWidth(default_width);

        // 收集页面中实际使用的字符，生成完整的宽度数组
        std::string w_array;
        if (font_manager_->isFontLoaded(font_name)) {
            // 收集所有页面中使用该字体的字符
            std::set<uint32_t> used_codepoints = collectUsedCodepoints(font_name);

            // 添加基本ASCII字符范围（确保基本字符有宽度）
            for (uint32_t i = 0x20; i <= 0x7E; ++i) {
                used_codepoints.insert(i);
            }
            // 添加常用货币符号
            used_codepoints.insert(0x00A5);  // ¥
            used_codepoints.insert(0xFFE5);  // ￥

            std::vector<uint32_t> codepoints_vec(used_codepoints.begin(), used_codepoints.end());
            w_array = font_manager_->generateWidthArray(font_name, 12.0, codepoints_vec);
            PDF_DEBUG("Generated dynamic width array for " + std::to_string(codepoints_vec.size()) +
                      " characters using FontManager for: " + font_name);
        } else {
            // 回退到硬编码宽度数组
            w_array = generateWidthArray(font_name);
            PDF_DEBUG("Using fallback width array for: " + font_name);
        }

        if (!w_array.empty()) {
            cid_font_obj->setWidths(w_array);
            PDF_DEBUG("Set width array for font: " + font_name);
        }

        cid_font_obj->setFontDescriptor(font_desc_id);  // 设置FontDescriptor引用

        // 关键修复：设置CIDToGIDMap为Identity（CID == GID）
        cid_font_obj->setCIDToGIDMap("/Identity");
        PDF_DEBUG("Set CIDToGIDMap to /Identity for font: " + font_name);

        // 将CIDFont添加到对象列表
        addObject(std::move(cid_font_obj));

        // 创建完整的ToUnicode CMap，支持文本复制
        int tounicode_id = getNextObjectId();
        auto tounicode_obj = std::make_unique<StreamObject>(tounicode_id);

        // 创建包含具体字符映射的ToUnicode CMap
        std::string cmap_content;
        if (font_manager_->isFontLoaded(font_name)) {
            // 使用FontManager生成动态ToUnicode CMap
            std::vector<uint32_t> common_chars;
            // 添加ASCII字符范围
            for (uint32_t i = 0x20; i <= 0x7E; ++i) {
                common_chars.push_back(i);
            }
            // 添加常用货币符号
            common_chars.push_back(0x00A5);  // ¥
            common_chars.push_back(0xFFE5);  // ￥

            cmap_content = font_manager_->generateToUnicodeCMap(font_name, common_chars);
            PDF_DEBUG("Generated dynamic ToUnicode CMap using FontManager for: " + font_name);
        } else {
            // 回退到硬编码CMap
            cmap_content = generateToUnicodeCMap();
            PDF_DEBUG("Using fallback ToUnicode CMap for: " + font_name);
        }

        tounicode_obj->setStreamData(cmap_content);
        addObject(std::move(tounicode_obj));

        // 设置Type0字体的DescendantFonts和ToUnicode引用
        font_obj->setDescendantFont(cid_font_id);
        font_obj->setToUnicode(tounicode_id);

        PDF_DEBUG("Created Type0 font with CIDFont (ID: " + std::to_string(cid_font_id) + ") and ToUnicode (ID: " +
                  std::to_string(tounicode_id) + ") for UTF-16BE support");

        addObject(std::move(font_obj));

        return resource_id;
    }

    std::string Writer::getFontResourceId(const std::string &font_name) const {
        auto it = font_resources_.find(font_name);
        return (it != font_resources_.end()) ? it->second : "";
    }

    std::string Writer::getFontSubtype(const std::string &font_name) const {
        auto it = font_subtypes_.find(font_name);
        return (it != font_subtypes_.end()) ? it->second : "";
    }

    FontManager *Writer::getFontManager() const {
        return font_manager_.get();
    }

// 字体回退相关方法已移除，简化字体处理逻辑

    std::string Writer::registerImage(const std::string &image_path) {
        // 加载图像文件
        ImageData image_data = loadImageFile(image_path);
        if (image_data.data.empty()) {
            PDF_ERROR("Failed to load image: " + image_path);
            return "";
        }

        // 使用加载的数据注册图像
        return registerImage(image_data.data, image_data.width, image_data.height, image_data.format);
    }

    std::string Writer::registerImage(const std::vector<std::uint8_t> &image_data,
                                      int width, int height, const std::string &format) {
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

    void Writer::setDocumentInfo(const std::string &title,
                                 const std::string &author,
                                 const std::string &subject,
                                 const std::string &creator) {
        doc_info_.title = title;
        doc_info_.author = author;
        doc_info_.subject = subject;
        doc_info_.creator = creator;
    }

    void Writer::setPdfVersion(const std::string &version) {
        pdf_version_ = version;
    }

// ========================================
// 文件生成
// ========================================

    void Writer::saveToFile(const std::string &filename) {
        auto data = saveToBuffer();

        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("无法创建文件: " + filename);
        }

        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
    }

    std::vector<std::uint8_t> Writer::saveToBuffer() {
        BinaryWriter writer;  // 使用内存模式
        writeTo(writer);
        return writer.getBuffer();
    }

    void Writer::writeTo(BinaryWriter &writer) {
        PDF_DEBUG("Starting PDF generation");

        // 1. 创建Catalog对象（但不创建Pages对象，因为需要先知道页面ID）
        createCatalogObject();
        createInfoObject();

        PDF_DEBUG("Created catalog (ID: " + std::to_string(catalog_object_id_) + "), pages will be (ID: " +
                  std::to_string(pages_object_id_) + ")");

        // 清空并重新分配页面对象ID
        page_object_ids_.clear();

        // 2. 预先扫描所有页面内容，确保字体已注册
        PDF_DEBUG("Pre-scanning pages to register fonts...");
        for (size_t i = 0; i < pages_.size(); ++i) {
            auto &page = pages_[i];
            page->ensureFontsRegistered(*this);  // 确保页面使用的字体已注册
        }
        PDF_DEBUG("Font pre-registration completed. Total fonts: " + std::to_string(font_resources_.size()));

        // 3. 执行字体子集化（在所有文本添加完成后）
        performFontSubsetting();

        // 3. 为每个页面创建对象（现在字体已经注册，资源字典将包含正确的字体引用）
        for (size_t i = 0; i < pages_.size(); ++i) {
            auto &page = pages_[i];
            int content_id = getNextObjectId();
            auto content_obj = page->createContentObject(content_id);
            addObject(std::move(content_obj));

            // 重新分配页面对象ID（不使用页面创建时的旧ID）
            int page_id = getNextObjectId();
            page_object_ids_.push_back(page_id);  // 记录新的页面对象ID

            // 创建页面对象时直接使用新的ID
            auto page_obj = page->createPageObjectWithId(page_id, pages_object_id_, content_id, generateResourceDict());
            addObject(std::move(page_obj));

            PDF_DEBUG("Created page " + std::to_string(i) + ": page_id=" + std::to_string(page_id) + ", content_id=" +
                      std::to_string(content_id));
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
        for (int page_id: page_object_ids_) {
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

    void Writer::writeHeader(BinaryWriter &writer) {
        writer.writeLine("%PDF-" + pdf_version_);
        writer.writeLine("%âãÏÓ");  // 二进制注释
    }

    void Writer::writeObjects(BinaryWriter &writer) {
        PDF_DEBUG("Writing " + std::to_string(objects_.size()) + " objects:");

        // 调试：检查对象ID的连续性
        std::vector<int> object_ids;
        for (const auto &[id, entry]: objects_) {
            object_ids.push_back(id);
        }
        std::sort(object_ids.begin(), object_ids.end());

        PDF_DEBUG("Object IDs: " + [&]() {
            std::ostringstream oss;
            for (size_t i = 0; i < object_ids.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << object_ids[i];
            }
            return oss.str();
        }());

        // 检查是否有ID跳跃
        for (size_t i = 1; i < object_ids.size(); ++i) {
            if (object_ids[i] != object_ids[i - 1] + 1) {
                PDF_WARN("Object ID gap detected: " + std::to_string(object_ids[i - 1]) + " -> " +
                         std::to_string(object_ids[i]));
            }
        }

        for (auto &[id, entry]: objects_) {
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

    std::size_t Writer::writeXrefTable(BinaryWriter &writer) {
        std::size_t xref_offset = writer.getOffsetLong();

        // 找出最大对象ID
        int max_id = 0;
        for (const auto &[id, entry]: objects_) {
            max_id = std::max(max_id, id);
        }

        writer.writeLine("xref");
        writer.writeLine("0 " + std::to_string(max_id + 1));

        // 写入对象0（总是free）
        writer.writeLine("0000000000 65535 f ");

        // 写入从1到max_id的所有对象条目
        for (int id = 1; id <= max_id; ++id) {
            auto it = objects_.find(id);
            if (it != objects_.end()) {
                // 对象存在，写入其偏移量
                std::ostringstream oss;
                oss << std::setfill('0') << std::setw(10) << it->second.offset << " 00000 n ";
                writer.writeLine(oss.str());
            } else {
                // 对象不存在，标记为free
                writer.writeLine("0000000000 65535 f ");
                PDF_DEBUG("Object " + std::to_string(id) + " marked as free in xref table");
            }
        }

        PDF_DEBUG("Xref table written with " + std::to_string(max_id + 1) + " entries (0 to " + std::to_string(max_id) +
                  ")");
        return xref_offset;
    }

    void Writer::writeTrailer(BinaryWriter &writer, std::size_t xref_offset) {
        // 计算正确的Size值（最大对象ID + 1）
        int max_id = 0;
        for (const auto &[id, entry]: objects_) {
            max_id = std::max(max_id, id);
        }

        writer.writeLine("trailer");
        writer.writeLine("<<");
        writer.writeLine("/Size " + std::to_string(max_id + 1));
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

        for (const auto &[font_name, resource_id]: font_resources_) {
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

    std::set<uint32_t> Writer::collectUsedCodepoints(const std::string &font_name) const {
        // 这个方法现在通过页面收集字符码点
        std::set<uint32_t> all_codepoints;

        // 遍历所有页面，收集实际使用的字符
        for (const auto &page: pages_) {
            auto page_codepoints = page->collectUsedCodepoints(font_name);
            all_codepoints.insert(page_codepoints.begin(), page_codepoints.end());
        }

        PDF_DEBUG("Collected " + std::to_string(all_codepoints.size()) + " unique codepoints from all pages" +
                  (font_name.empty() ? "" : " for font: " + font_name));

        return all_codepoints;
    }

    bool Writer::isUnicodeFont(const std::string &font_name) const {
        // 检查字体子类型
        std::string subtype = getFontSubtype(font_name);
        return (subtype == "Type0");
    }

    bool Writer::isCJKCharacter(uint32_t codepoint) const {
        return tinakit::core::unicode::is_cjk_character(codepoint);
    }

    std::string Writer::generateWidthArray(const std::string &font_name) const {
        // 为ASCII字符生成宽度数组，解决英文字符间距过大的问题
        // 注意：只返回数组体，不包含/W关键字
        std::ostringstream oss;
        oss << "[ ";

        // ASCII字符范围 (0x0020-0x007E)，设置合适的宽度
        // 使用正确的PDF语法：单个CID用[width]，连续区段用 cFirst cLast width

        // 空格字符 (0x0020) - 单个CID必须用方括号
        oss << "32 [250] ";

        // 标点符号 (0x0021-0x0023) - 连续区段不用方括号
        oss << "33 35 350 ";  // !"#

        // 美元符号 '$' (ASCII 36) - 单独设置避免与数字紧贴
        oss << "36 [525] ";

        // 其他标点符号 (0x0025-0x002F) - 连续区段
        oss << "37 47 350 ";  // %&'()*+,-./ (增加宽度以改善间距)

        // 数字 (0x0030-0x0039) - 连续区段
        oss << "48 57 550 ";  // 0123456789 (增加宽度以改善间距)

        // 更多标点 (0x003A-0x0040) - 连续区段
        oss << "58 64 350 ";  // :;<=>?@ (增加宽度以改善间距)

        // 大写字母 (0x0041-0x005A) - 连续区段
        bool is_cjk_font = (font_name == "SimSun" || font_name == "NSimSun");
        oss << "65 90 " << (is_cjk_font ? 600 : 550) << " ";  // A-Z

        // 更多标点 (0x005B-0x0060) - 连续区段
        oss << "91 96 350 ";  // [\]^_` (增加宽度以改善间距)

        // 小写字母 (0x0061-0x007A) - 连续区段
        oss << "97 122 " << (is_cjk_font ? 550 : 500) << " ";  // a-z (增加宽度以改善间距)

        // 最后的标点 (0x007B-0x007E) - 连续区段
        oss << "123 126 350 ";  // {|}~ (增加宽度以改善间距)

        // 货币符号 - 单独设置避免间距问题
        // ASCII ¥ (U+00A5, CID = 165)
        oss << "165 [525] ";

        // 全角 ￥ (U+FFE5, CID = 65509) - SimSun常用
        oss << "65509 [525] ";

        oss << "]";

        PDF_DEBUG("Generated width array for " + font_name + ": " + oss.str());
        return oss.str();
    }

    std::string Writer::generateToUnicodeCMap() const {
        std::ostringstream oss;

        oss << "/CIDInit /ProcSet findresource begin\n";
        oss << "12 dict begin\n";
        oss << "begincmap\n";
        oss << "/CIDSystemInfo\n";
        oss << "<< /Registry (Adobe)\n";
        oss << "   /Ordering (UCS)\n";
        oss << "   /Supplement 0\n";
        oss << ">> def\n";
        oss << "/CMapName /Adobe-Identity-UCS def\n";
        oss << "/CMapType 2 def\n";
        oss << "1 begincodespacerange\n";
        oss << "<0000> <FFFF>\n";
        oss << "endcodespacerange\n";

        // 添加具体的字符映射，支持文本复制
        // ASCII字符(95) + 货币符号(3) + 中文字符(22) = 120个字符
        oss << "120 beginbfchar\n";

        // ASCII字符映射 (0x20-0x7E)
        for (int i = 0x20; i <= 0x7E; ++i) {
            oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << i << "> ";
            oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << i << ">\n";
        }

        // 货币符号映射（重要：确保复制时不丢失）
        std::vector<std::pair<uint16_t, uint16_t>> currency_chars = {
                {0x0024, 0x0024}, // $ (美元符号)
                {0x00A5, 0x00A5}, // ¥ (ASCII日元/人民币符号)
                {0xFFE5, 0xFFE5}, // ￥ (全角日元/人民币符号)
        };

        for (const auto &[cid, unicode]: currency_chars) {
            oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << cid << "> ";
            oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << unicode << ">\n";
        }

        // 常用中文字符映射示例（这里只添加几个示例）
        // 实际应用中可以根据需要添加更多字符
        std::vector<std::pair<uint16_t, uint16_t>> chinese_chars = {
                {0x4E2D, 0x4E2D}, // 中
                {0x6587, 0x6587}, // 文
                {0x4E16, 0x4E16}, // 世
                {0x754C, 0x754C}, // 界
                {0x4F60, 0x4F60}, // 你
                {0x597D, 0x597D}, // 好
                {0x6D4B, 0x6D4B}, // 测
                {0x8BD5, 0x8BD5}, // 试
                {0x4F7F, 0x4F7F}, // 使
                {0x7528, 0x7528}, // 用
                {0x5F00, 0x5F00}, // 开
                {0x53D1, 0x53D1}, // 发
                {0x5E93, 0x5E93}, // 库
                {0x6709, 0x6709}, // 有
                {0x8DA3, 0x8DA3}, // 趣
                {0x4EF7, 0x4EF7}, // 价
                {0x683C, 0x683C}, // 格
                {0x4EBA, 0x4EBA}, // 人
                {0x6C11, 0x6C11}, // 民
                {0x5E01, 0x5E01}, // 币
                {0x4F18, 0x4F18}, // 优
                {0x60E0, 0x60E0}, // 惠
        };

        for (const auto &[cid, unicode]: chinese_chars) {
            oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << cid << "> ";
            oss << "<" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << unicode << ">\n";
        }

        oss << "endbfchar\n";
        oss << "endcmap\n";
        oss << "CMapName currentdict /CMap defineresource pop\n";
        oss << "end\n";
        oss << "end\n";

        PDF_DEBUG("Generated ToUnicode CMap with character mappings");
        return oss.str();
    }


    std::string Writer::registerFontWithSubsetting(const std::string &font_name,
                                                   const std::vector<std::uint8_t> &font_data,
                                                   bool enable_subsetting,
                                                   bool embed_font) {
        PDF_DEBUG("Registering font with new subset manager: " + font_name +
                  " (" + std::to_string(font_data.size()) + " bytes, subsetting: " +
                  (enable_subsetting ? "enabled" : "disabled") + ")");

        // 1. 注册字体到新的子集化管理器
        subset_manager_->registerFont(font_name, font_data, enable_subsetting, embed_font);

        // 2. 注册字体到FontManager（用于文本整形和GID转换）
        if (!font_manager_->loadFont(font_data, font_name)) {
            PDF_ERROR("Failed to load font into FontManager: " + font_name);
            return "";
        }

        // 3. 创建字体资源ID
        std::string resource_id = "F" + std::to_string(next_resource_id_++);
        font_resources_[font_name] = resource_id;

        // 4. 记录字体子类型
        font_subtypes_[font_name] = "Type0";

        // 5. 立即创建基本的字体对象（使用原始字体），后续会被子集化结果替换
        createBasicFontObjects(font_name, font_data);

        PDF_DEBUG("Font registered with subset manager: " + font_name + " -> " + resource_id);
        return resource_id;
    }

    void Writer::performFontSubsetting() {
        PDF_DEBUG("Starting font subsetting with new manager...");

        // 使用新的字体子集化管理器
        auto subset_results = subset_manager_->performSubsetting();

        // 为每个字体创建PDF对象
        for (const auto &result: subset_results) {
            createFontObjectsFromSubset(result);
        }

        // 显示统计信息
        PDF_DEBUG(subset_manager_->getStatistics());
        PDF_DEBUG("Font subsetting process completed with new manager");
    }


    void Writer::recordTextUsage(const std::string &font_name, const std::string &text) {
        if (subset_manager_) {
            subset_manager_->recordTextUsage(font_name, text);
        }
    }

    void Writer::createFontObjectsFromSubset(const FontSubsetResult &result) {
        const std::string &font_name = result.font_name;

        PDF_DEBUG("Updating font objects with subset data for: " + font_name +
                  " (size: " + std::to_string(result.subset_size) + " bytes)");

        // 获取最终的字体数据（子集或原始）
        auto font_data = subset_manager_->getFinalFontData(font_name);
        if (font_data.empty()) {
            PDF_ERROR("No font data available for: " + font_name);
            return;
        }

        // 检查是否已有字体对象
        auto font_id_it = font_object_ids_.find(font_name);
        if (font_id_it == font_object_ids_.end()) {
            PDF_ERROR("No existing font objects found for: " + font_name);
            return;
        }

        // 如果子集化没有减少大小，就不需要更新
        if (result.subset_size >= result.original_size) {
            PDF_DEBUG("Subset size not reduced, keeping existing font objects for: " + font_name);
            return;
        }

        // 找到现有的FontFile对象并更新其数据
        bool font_file_updated = false;
        for (auto &[obj_id, obj_entry]: objects_) {
            auto *font_file_obj = dynamic_cast<FontFileObject *>(obj_entry.object.get());
            if (font_file_obj && font_file_obj->getStreamSize() == result.original_size) {
                // 这很可能是我们要更新的字体文件对象
                // 由于FontFileObject的设计，我们需要重新创建它
                auto new_font_file_obj = std::make_unique<FontFileObject>(obj_id, font_data, "FontFile2");
                obj_entry.object = std::move(new_font_file_obj);
                font_file_updated = true;
                PDF_DEBUG("Updated FontFile object " + std::to_string(obj_id) + " with subset data");
                break;
            }
        }

        if (!font_file_updated) {
            PDF_WARN("Could not find FontFile object to update for: " + font_name);
        }

        PDF_DEBUG("Font subset update completed for " + font_name);
    }

    void Writer::createBasicFontObjects(const std::string &font_name, const std::vector<std::uint8_t> &font_data) {
        PDF_DEBUG("Creating basic font objects for: " + font_name + " (temporary, will be replaced by subset)");

        // 创建字体对象ID
        int font_object_id = getNextObjectId();
        font_object_ids_[font_name] = font_object_id;

        // 创建Type0字体对象
        auto font_obj = std::make_unique<FontObject>(font_object_id, font_name, "Type0");
        font_obj->setEncoding("Identity-H");  // Unicode编码

        // 创建CID字体对象
        int cid_font_id = getNextObjectId();
        auto cid_font_obj = std::make_unique<CIDFontObject>(cid_font_id, font_name, "CIDFontType2");
        cid_font_obj->setCIDSystemInfo("Adobe", "Identity", 0);

        // 设置合理的默认宽度和字符宽度表
        bool is_cjk_font = (font_name == "SimSun" || font_name == "NSimSun" ||
                            font_name == "SourceHanSansSC-Regular" || font_name.find("SourceHan") != std::string::npos);
        int default_width = is_cjk_font ? 1000 : 500;
        cid_font_obj->setDefaultWidth(default_width);

        // 生成宽度数组
        // 收集所有可能使用的字符，包括中文
        std::set<uint32_t> all_chars;

        // 基本ASCII字符
        for (uint32_t i = 0x20; i <= 0x7E; ++i) {
            all_chars.insert(i);
        }

        // 货币符号
        all_chars.insert(0x00A5);  // ¥
        all_chars.insert(0xFFE5);  // ￥

        // 常用中文字符（选择性添加，避免数组过大）
        std::vector<uint32_t> common_chinese = {
                0x4E00, 0x4E01, 0x4E03, 0x4E07, 0x4E08, 0x4E09, 0x4E0A, 0x4E0B, 0x4E0D, 0x4E0E,  // 一二三万丈与上下不与
                0x4E2D, 0x4E2A, 0x4E2B, 0x4E2D, 0x4E3A, 0x4E3B, 0x4E3D, 0x4E3E, 0x4E48, 0x4E49,  // 中个乃中为主丽乾么义
                0x4EBA, 0x4EC0, 0x4ECA, 0x4ECB, 0x4ECD, 0x4ED6, 0x4ED7, 0x4ED8, 0x4ED9, 0x4EE3,  // 人什今介仍他仗付仙代
                0x4F60, 0x4F7F, 0x4F86, 0x4F8B, 0x4F9B, 0x4F9D, 0x4FA0, 0x4FA1, 0x4FAE, 0x4FAF,  // 你使來例供依俠價侮俯
                0x5065, 0x5074, 0x5076, 0x507D, 0x5085, 0x508D, 0x5091, 0x5098, 0x50AC, 0x50AD,  // 偵側偶偽停偍偑偘催偭
                0x5168, 0x516B, 0x516C, 0x516D, 0x5171, 0x5175, 0x5176, 0x5177, 0x517C, 0x5180,  // 全八公六共兵其具兼兀
                0x4E16, 0x754C, 0x6D4B, 0x8BD5, 0x4EF7, 0x683C, 0x82F1, 0x6587, 0x7EAF, 0x6D4B   // 世界测试价格英文纯测
        };

        for (uint32_t cp: common_chinese) {
            all_chars.insert(cp);
        }

        std::vector<uint32_t> codepoints_vec(all_chars.begin(), all_chars.end());
        std::string w_array = font_manager_->generateWidthArray(font_name, 12.0, codepoints_vec);
        PDF_DEBUG("Generated comprehensive width array with " + std::to_string(all_chars.size()) + " characters for: " +
                  font_name);

        if (!w_array.empty()) {
            cid_font_obj->setWidths(w_array);
            PDF_DEBUG("Set width array for font: " + font_name);
        }

        // 设置CIDToGIDMap为Identity
        cid_font_obj->setCIDToGIDMap("/Identity");

        // 创建字体文件对象
        int font_file_id = getNextObjectId();
        auto font_file_obj = std::make_unique<FontFileObject>(font_file_id, font_data, "FontFile2");

        // 创建字体描述符对象
        int font_descriptor_id = getNextObjectId();
        auto font_descriptor_obj = std::make_unique<FontDescriptorObject>(font_descriptor_id, font_name);

        // 为中文字体设置正确的标志
        if (is_cjk_font) {
            font_descriptor_obj->setFlags(4);  // Nonsymbolic for CJK fonts
            font_descriptor_obj->setFontBBox({-1000, -1000, 2000, 1000});  // 更大的边界框适应中文字符
            font_descriptor_obj->setFontMetrics(1000, -200, 1000, 100);   // 中文字体的典型度量
        } else {
            font_descriptor_obj->setFlags(32);  // Symbolic for other fonts
            font_descriptor_obj->setFontBBox({-1000, -1000, 1000, 1000});
            font_descriptor_obj->setFontMetrics(800, -200, 800, 80);
        }

        font_descriptor_obj->setFontFile(font_file_id, "FontFile2");

        // 创建ToUnicode CMap对象
        int tounicode_id = getNextObjectId();

        // 收集所有可能使用的字符，包括中文
        std::vector<uint32_t> cmap_chars;

        // 基本ASCII字符
        for (uint32_t i = 0x20; i <= 0x7E; ++i) {
            cmap_chars.push_back(i);
        }

        // 货币符号
        cmap_chars.push_back(0x00A5);  // ¥
        cmap_chars.push_back(0xFFE5);  // ￥

        // 常用中文字符范围
        for (uint32_t i = 0x4E00; i <= 0x9FFF; ++i) {  // CJK统一汉字
            cmap_chars.push_back(i);
        }

        // 中文标点符号
        for (uint32_t i = 0x3000; i <= 0x303F; ++i) {  // CJK符号和标点
            cmap_chars.push_back(i);
        }

        std::string cmap_content = font_manager_->generateToUnicodeCMap(font_name, cmap_chars);
        PDF_DEBUG("Generated comprehensive ToUnicode CMap with " + std::to_string(cmap_chars.size()) +
                  " characters for: " + font_name);

        auto tounicode_obj = std::make_unique<StreamObject>(tounicode_id);
        tounicode_obj->setStreamData(cmap_content);

        // 设置字体关系
        cid_font_obj->setFontDescriptor(font_descriptor_id);
        font_obj->setDescendantFont(cid_font_id);
        font_obj->setToUnicode(tounicode_id);

        // 添加所有对象到映射表
        ObjectEntry font_entry;
        font_entry.object = std::move(font_obj);
        objects_[font_object_id] = std::move(font_entry);

        ObjectEntry cid_font_entry;
        cid_font_entry.object = std::move(cid_font_obj);
        objects_[cid_font_id] = std::move(cid_font_entry);

        ObjectEntry font_file_entry;
        font_file_entry.object = std::move(font_file_obj);
        objects_[font_file_id] = std::move(font_file_entry);

        ObjectEntry font_descriptor_entry;
        font_descriptor_entry.object = std::move(font_descriptor_obj);
        objects_[font_descriptor_id] = std::move(font_descriptor_entry);

        ObjectEntry tounicode_entry;
        tounicode_entry.object = std::move(tounicode_obj);
        objects_[tounicode_id] = std::move(tounicode_entry);

        PDF_DEBUG("Basic font objects created for " + font_name + ": " +
                  "Font=" + std::to_string(font_object_id) +
                  ", CIDFont=" + std::to_string(cid_font_id) +
                  ", FontFile=" + std::to_string(font_file_id) +
                  ", FontDescriptor=" + std::to_string(font_descriptor_id) +
                  ", ToUnicode=" + std::to_string(tounicode_id));
    }

} // namespace tinakit::pdf::core
