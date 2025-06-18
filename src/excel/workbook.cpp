/**
 * @file workbook.cpp
 * @brief Excel 工作簿类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/shared_strings.hpp"
#include "tinakit/excel/style_manager.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/xml_parser.hpp"
#include "tinakit/core/exceptions.hpp"
#include <sstream>
#include <algorithm>
#include <format>
#include <chrono>
#include <iomanip>

namespace tinakit::excel {

/**
 * @class Workbook::Impl
 * @brief Workbook 类的实现细节（PIMPL 模式）
 */
class Workbook::Impl {
public:
    using WorksheetPtr = std::shared_ptr<Worksheet>;
    using ErrorCallback = std::function<void(const std::exception&)>;

public:
    explicit Impl(const std::filesystem::path& path);
    explicit Impl();
    ~Impl() = default;

    // 工作表管理
    Worksheet& get_worksheet(const std::string& name);
    Worksheet& get_worksheet(std::size_t index);
    const Worksheet& get_worksheet(const std::string& name) const;
    const Worksheet& get_worksheet(std::size_t index) const;
    
    Worksheet& add_worksheet(const std::string& name);
    void remove_worksheet(const std::string& name);
    void remove_worksheet(std::size_t index);
    
    std::vector<std::reference_wrapper<Worksheet>> get_worksheets();
    std::vector<std::reference_wrapper<const Worksheet>> get_worksheets() const;
    std::size_t worksheet_count() const noexcept;
    
    Worksheet& get_active_sheet();
    const Worksheet& get_active_sheet() const;

    // 文件操作
    void save(const std::filesystem::path& path = {});
    async::Task<void> save_async(const std::filesystem::path& path = {});
    
    // 回调支持
    void set_error_callback(ErrorCallback callback);
    
    // 属性
    const std::filesystem::path& get_file_path() const noexcept;
    bool has_unsaved_changes() const noexcept;
    std::size_t get_file_size() const;
    
    // 获取管理器
    SharedStrings& get_shared_strings() { return *shared_strings_; }
    const SharedStrings& get_shared_strings() const { return *shared_strings_; }
    StyleManager& get_style_manager() { return *style_manager_; }
    const StyleManager& get_style_manager() const { return *style_manager_; }

    private:
    void load_from_archiver();
    void create_default_structure();
    void parse_workbook_rels();
    void parse_workbook_xml();
    void parse_shared_strings();
    void parse_styles();
    void regenerate_metadata_on_save();
    
    // OpenXML 结构生成
    void generate_root_rels();
    void generate_content_types();
    void generate_workbook_xml();
    void generate_workbook_rels();
    void generate_app_props();
    void generate_core_props();
    void generate_shared_strings();
    void generate_styles();
    void generate_worksheet_xml(std::size_t index);
    
    void report_error(const std::exception& error);

private:
        std::shared_ptr<core::OpenXmlArchiver> archiver_;
    std::filesystem::path original_path_;
    bool is_modified_ = false;
    std::size_t active_sheet_index_ = 0;

        std::vector<WorksheetPtr> worksheets_;
    std::map<std::string, WorksheetPtr, std::less<>> worksheet_by_name_;
    std::map<std::string, std::string, std::less<>> rels_map_;
    
    // 使用独立的管理器
    std::shared_ptr<SharedStrings> shared_strings_;
    std::shared_ptr<StyleManager> style_manager_;
    
    ErrorCallback error_callback_;
};

// =============================================================================
// Impl 实现
// =============================================================================

Workbook::Impl::Impl(const std::filesystem::path& path)
    : original_path_(path),
      shared_strings_(std::make_shared<SharedStrings>()),
      style_manager_(std::make_shared<StyleManager>()) {
    try {
        archiver_ = std::make_shared<core::OpenXmlArchiver>(
            async::sync_wait(core::OpenXmlArchiver::open_from_file(path.string()))
        );
        load_from_archiver();
    } catch (const std::exception& e) {
        report_error(e);
        throw;
    }
}

Workbook::Impl::Impl() 
    : is_modified_(true),
      shared_strings_(std::make_shared<SharedStrings>()),
      style_manager_(std::make_shared<StyleManager>()) {
    try {
        archiver_ = std::make_shared<core::OpenXmlArchiver>(
            core::OpenXmlArchiver::create_in_memory_writer()
        );
        create_default_structure();
    } catch (const std::exception& e) {
        report_error(e);
        throw;
    }
}

Worksheet& Workbook::Impl::get_worksheet(const std::string& name) {
    auto it = worksheet_by_name_.find(name);
    if (it == worksheet_by_name_.end()) {
        throw WorksheetNotFoundException("Worksheet not found: " + name);
    }
    return *it->second;
}

Worksheet& Workbook::Impl::get_worksheet(std::size_t index) {
    if (index >= worksheets_.size()) {
        throw std::out_of_range("Worksheet index out of range: " + std::to_string(index));
    }
    return *worksheets_[index];
}

const Worksheet& Workbook::Impl::get_worksheet(const std::string& name) const {
    auto it = worksheet_by_name_.find(name);
    if (it == worksheet_by_name_.end()) {
        throw WorksheetNotFoundException("Worksheet not found: " + name);
    }
    return *it->second;
}

const Worksheet& Workbook::Impl::get_worksheet(std::size_t index) const {
    if (index >= worksheets_.size()) {
        throw std::out_of_range("Worksheet index out of range: " + std::to_string(index));
    }
    return *worksheets_[index];
}

Worksheet& Workbook::Impl::add_worksheet(const std::string& name) {
    // 检查名称是否已存在
    if (worksheet_by_name_.find(name) != worksheet_by_name_.end()) {
        throw DuplicateWorksheetNameException("Worksheet name already exists: " + name);
    }
    
    // 使用 Worksheet 的工厂方法创建新的工作表
    auto worksheet = Worksheet::create(name);
    
    worksheets_.push_back(worksheet);
    worksheet_by_name_[name] = worksheet;
    is_modified_ = true;
    
    return *worksheet;
}

void Workbook::Impl::remove_worksheet(const std::string& name) {
    if (worksheets_.size() <= 1) {
        throw CannotDeleteLastWorksheetException();
    }
    
    auto it = worksheet_by_name_.find(name);
    if (it == worksheet_by_name_.end()) {
        throw WorksheetNotFoundException("Worksheet not found: " + name);
    }
    
    // 从向量中移除
    auto worksheet_ptr = it->second;
    worksheets_.erase(
        std::remove(worksheets_.begin(), worksheets_.end(), worksheet_ptr),
        worksheets_.end()
    );
    
    // 从映射中移除
    worksheet_by_name_.erase(it);
    is_modified_ = true;
}

void Workbook::Impl::remove_worksheet(std::size_t index) {
    if (index >= worksheets_.size()) {
        throw std::out_of_range("Worksheet index out of range: " + std::to_string(index));
    }
    
    if (worksheets_.size() <= 1) {
        throw CannotDeleteLastWorksheetException();
    }
    
    auto worksheet = worksheets_[index];
    const auto& name = worksheet->name();
    
    worksheets_.erase(worksheets_.begin() + index);
    worksheet_by_name_.erase(name);
    is_modified_ = true;
}

std::vector<std::reference_wrapper<Worksheet>> Workbook::Impl::get_worksheets() {
    std::vector<std::reference_wrapper<Worksheet>> result;
    result.reserve(worksheets_.size());
    for (auto& worksheet : worksheets_) {
        result.emplace_back(*worksheet);
    }
    return result;
}

std::vector<std::reference_wrapper<const Worksheet>> Workbook::Impl::get_worksheets() const {
    std::vector<std::reference_wrapper<const Worksheet>> result;
    result.reserve(worksheets_.size());
    for (const auto& worksheet : worksheets_) {
        result.emplace_back(*worksheet);
    }
    return result;
}

std::size_t Workbook::Impl::worksheet_count() const noexcept {
    return worksheets_.size();
}

Worksheet& Workbook::Impl::get_active_sheet() {
    if (worksheets_.empty()) {
        throw std::runtime_error("No worksheets available");
    }
    return *worksheets_[active_sheet_index_];
}

const Worksheet& Workbook::Impl::get_active_sheet() const {
    if (worksheets_.empty()) {
        throw std::runtime_error("No worksheets available");
    }
    return *worksheets_[active_sheet_index_];
}

void Workbook::Impl::save(const std::filesystem::path& path) {
    try {
        auto save_path = path.empty() ? original_path_ : path;
        if (save_path.empty()) {
            throw IOException("No file path specified for save");
        }
        
        regenerate_metadata_on_save();
        async::sync_wait(archiver_->save_to_file(save_path.string()));
        
        is_modified_ = false;
        if (!path.empty()) {
            original_path_ = path;
        }
    } catch (const std::exception& e) {
        report_error(e);
        throw;
    }
}

async::Task<void> Workbook::Impl::save_async(const std::filesystem::path& path) {
    try {
        auto save_path = path.empty() ? original_path_ : path;
        if (save_path.empty()) {
            throw IOException("No file path specified for save");
        }
        
        regenerate_metadata_on_save();
        co_await archiver_->save_to_file(save_path.string());
        
        is_modified_ = false;
        if (!path.empty()) {
            original_path_ = path;
        }
    } catch (const std::exception& e) {
        report_error(e);
        throw;
    }
}

void Workbook::Impl::set_error_callback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

const std::filesystem::path& Workbook::Impl::get_file_path() const noexcept {
    return original_path_;
}

bool Workbook::Impl::has_unsaved_changes() const noexcept {
    return is_modified_;
}

std::size_t Workbook::Impl::get_file_size() const {
    if (original_path_.empty() || !std::filesystem::exists(original_path_)) {
        return 0;
    }
    return std::filesystem::file_size(original_path_);
}

void Workbook::Impl::load_from_archiver() {
    parse_workbook_rels();
    parse_workbook_xml();
    parse_shared_strings();
    parse_styles();
    // TODO: 解析各个工作表
}

void Workbook::Impl::create_default_structure() {
    // 创建默认的工作表
    add_worksheet("Sheet1");
    
    // 生成默认的 OpenXML 结构
    generate_content_types();
    generate_workbook_xml();
    generate_workbook_rels();
    generate_app_props();
    generate_core_props();
}

void Workbook::Impl::parse_workbook_rels() {
    try {
        const std::string rels_path = "xl/_rels/workbook.xml.rels";
        
        // 使用 async 读取以提高性能
        auto rels_data = async::sync_wait(archiver_->read_file(rels_path));
        
        // 使用流式解析，避免一次性加载整个文档到内存
        std::istringstream stream(std::string(
            reinterpret_cast<const char*>(rels_data.data()), 
            rels_data.size()
        ));
        
        core::XmlParser parser(stream, rels_path);
        
        // 清空之前的关系映射
        rels_map_.clear();
        
        for (auto it = parser.begin(); it != parser.end(); ++it) {
            if (it.is_start_element() && it.name() == "Relationship") {
                // 提取属性，避免多次查找
                auto id = it.attribute("Id");
                auto target = it.attribute("Target");
                auto type = it.attribute("Type");
                
                if (id && target) {
                    // 直接移动字符串，避免拷贝
                    rels_map_.emplace(std::move(*id), std::move(*target));
                    
                    // 识别工作表关系，为后续解析做准备
                    if (type && type->find("worksheet") != std::string::npos) {
                        // 记录工作表路径供后续使用
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        throw ParseException("Failed to parse workbook relationships: " + std::string(e.what()));
    }
}

void Workbook::Impl::parse_workbook_xml() {
    try {
        const std::string workbook_path = "xl/workbook.xml";
        
        auto workbook_data = async::sync_wait(archiver_->read_file(workbook_path));
        
        std::istringstream stream(std::string(
            reinterpret_cast<const char*>(workbook_data.data()), 
            workbook_data.size()
        ));
        
        core::XmlParser parser(stream, workbook_path);
        
        // 预分配工作表容器
        worksheets_.reserve(10);  // 大多数 Excel 文件有少于 10 个工作表
        
        bool in_sheets = false;
        std::size_t sheet_index = 0;
        
        for (auto it = parser.begin(); it != parser.end(); ++it) {
            if (it.is_start_element()) {
                if (it.name() == "sheets") {
                    in_sheets = true;
                } else if (in_sheets && it.name() == "sheet") {
                    // 提取工作表信息
                    auto name = it.attribute("name");
                    auto sheet_id = it.attribute("sheetId");
                    auto r_id = it.attribute("r:id");
                    
                    if (name && r_id) {
                        // 从关系映射中获取实际路径
                        auto rel_it = rels_map_.find(*r_id);
                        if (rel_it != rels_map_.end()) {
                            // 创建工作表对象（延迟加载内容）
                            auto worksheet = Worksheet::create(*name);
                            
                            // 存储工作表
                            worksheets_.push_back(worksheet);
                            worksheet_by_name_[*name] = worksheet;
                            
                            // 更新最大行列数（实际数据将在需要时加载）
                            if (sheet_index == 0) {
                                active_sheet_index_ = 0;  // 第一个工作表为活动工作表
                            }
                            sheet_index++;
                        }
                    }
                }
            } else if (it.is_end_element() && it.name() == "sheets") {
                in_sheets = false;
            }
        }
        
        // 如果没有工作表，这不是有效的 Excel 文件
        if (worksheets_.empty()) {
            throw ParseException("No worksheets found in workbook");
        }
        
    } catch (const ParseException&) {
        throw;  // 重新抛出解析异常
    } catch (const std::exception& e) {
        throw ParseException("Failed to parse workbook.xml: " + std::string(e.what()));
    }
}

void Workbook::Impl::parse_shared_strings() {
    try {
        const std::string shared_strings_path = "xl/sharedStrings.xml";
        
        // 检查文件是否存在（共享字符串表是可选的）
        if (!async::sync_wait(archiver_->has_file(shared_strings_path))) {
            return;  // 没有共享字符串表，直接返回
        }
        
        auto strings_data = async::sync_wait(archiver_->read_file(shared_strings_path));
        
        std::string xml_data(
            reinterpret_cast<const char*>(strings_data.data()), 
            strings_data.size()
        );
        
        // 使用 SharedStrings 对象的 load_from_xml 方法
        shared_strings_->load_from_xml(xml_data);
        
    } catch (const std::exception& e) {
        // 共享字符串解析失败不应该阻止文件打开
        // 记录错误但继续处理
        report_error(ParseException("Warning: Failed to parse shared strings: " + std::string(e.what())));
    }
}

void Workbook::Impl::parse_styles() {
    try {
        const std::string styles_path = "xl/styles.xml";
        
        // 检查文件是否存在（样式表应该始终存在）
        if (!async::sync_wait(archiver_->has_file(styles_path))) {
            // 如果没有样式表，使用默认样式
            style_manager_->initialize_defaults();
            return;
        }
        
        auto styles_data = async::sync_wait(archiver_->read_file(styles_path));
        
        std::string xml_data(
            reinterpret_cast<const char*>(styles_data.data()), 
            styles_data.size()
        );
        
        // 使用 StyleManager 对象的 load_from_xml 方法
        style_manager_->load_from_xml(xml_data);
        
    } catch (const std::exception& e) {
        // 样式解析失败时使用默认样式
        style_manager_->initialize_defaults();
        report_error(ParseException("Warning: Failed to parse styles, using defaults: " + std::string(e.what())));
    }
}

void Workbook::Impl::regenerate_metadata_on_save() {
    if (!is_modified_) return;
    
    // 生成根关系文件
    generate_root_rels();
    
    // 重新生成所有元数据文件
    generate_content_types();
    generate_workbook_xml();
    generate_workbook_rels();
    generate_app_props();
    generate_core_props();
    
    // 生成每个工作表的 XML 文件
    for (std::size_t i = 0; i < worksheets_.size(); ++i) {
        generate_worksheet_xml(i);
    }
    
    // 生成共享字符串文件（如果有）
    if (shared_strings_->count() > 0) {
        generate_shared_strings();
    }
    
    // 生成样式文件
    generate_styles();
}

void Workbook::Impl::generate_root_rels() {
    std::string root_rels = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
  <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
</Relationships>)";
    
    // 转换为字节数组
    std::vector<std::byte> xml_bytes(root_rels.size());
    std::memcpy(xml_bytes.data(), root_rels.data(), root_rels.size());
    
    // 添加到归档
    async::sync_wait(archiver_->add_file("_rels/.rels", std::move(xml_bytes)));
}

void Workbook::Impl::generate_content_types() {
    std::string content_types = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
    <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
    <Default Extension="xml" ContentType="application/xml"/>
    <Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>)";

    for (std::size_t i = 0; i < worksheets_.size(); ++i) {
        content_types += std::format(R"(
    <Override PartName="/xl/worksheets/sheet{}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>)", i + 1);
    }

    // 检查是否有字符串数据需要共享字符串表
    bool has_string_data = false;
    for (const auto& worksheet : worksheets_) {
        for (std::size_t r = 1; r <= worksheet->max_row(); ++r) {
            for (std::size_t c = 1; c <= worksheet->max_column(); ++c) {
                try {
                    auto& cell = worksheet->cell(r, c);
                    if (!cell.empty()) {
                        const auto& value_variant = cell.raw_value();
                        if (std::holds_alternative<std::string>(value_variant)) {
                            has_string_data = true;
                            break;
                        }
                    }
                } catch (...) {
                    // 忽略空单元格
                }
            }
            if (has_string_data) break;
        }
        if (has_string_data) break;
    }

    // 共享字符串内容类型（如果有字符串数据）
    if (has_string_data) {
        content_types += R"(
    <Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>)";
    }

    // 样式内容类型
    content_types += R"(
    <Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
    <Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
    <Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>)";

    content_types += R"(
</Types>)";

    auto content_bytes = std::vector<std::byte>(content_types.size());
    std::memcpy(content_bytes.data(), content_types.data(), content_types.size());

    async::sync_wait(archiver_->add_file("[Content_Types].xml", std::move(content_bytes)));
}

void Workbook::Impl::generate_workbook_xml() {
    // 使用字符串流构建 XML，避免多次字符串拼接
    std::ostringstream xml;
    
    // XML 声明和根元素
    xml << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)" << '\n';
    xml << R"(<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" )"
        << R"(xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">)" << '\n';
    
    // 添加 fileVersion（某些 Excel 版本需要）
    xml << R"(  <fileVersion appName="TinaKit" lastEdited="1" lowestEdited="1" rupBuild="1"/>)" << '\n';
    
    // 添加 workbookPr
    xml << R"(  <workbookPr/>)" << '\n';
    
    // 添加 bookViews
    xml << R"(  <bookViews>)" << '\n';
    xml << R"(    <workbookView windowWidth="16384" windowHeight="8192"/>)" << '\n';
    xml << R"(  </bookViews>)" << '\n';
    
    // 工作表列表
    xml << "  <sheets>\n";
    
    for (std::size_t i = 0; i < worksheets_.size(); ++i) {
        const auto& worksheet = worksheets_[i];
        
        // 生成关系 ID
        std::string r_id = "rId" + std::to_string(i + 1);
        
        xml << "    <sheet name=\"" << worksheet->name() 
            << "\" sheetId=\"" << (i + 1) 
            << "\" r:id=\"" << r_id << "\"/>\n";
    }
    
    xml << "  </sheets>\n";
    
    // 添加 calcPr（计算属性）
    xml << R"(  <calcPr calcId="0"/>)" << '\n';
    
    xml << "</workbook>\n";
    
    // 转换为字节数组
    const std::string& xml_str = xml.str();
    std::vector<std::byte> xml_bytes(xml_str.size());
    std::memcpy(xml_bytes.data(), xml_str.data(), xml_str.size());
    
    // 添加到归档
    async::sync_wait(archiver_->add_file("xl/workbook.xml", std::move(xml_bytes)));
}

void Workbook::Impl::generate_workbook_rels() {
    std::ostringstream xml;

    // XML 声明
    xml << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)" << '\n';
    xml << R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)" << '\n';

    // 为每个工作表生成关系
    for (std::size_t i = 0; i < worksheets_.size(); ++i) {
        xml << R"(  <Relationship Id="rId)" << (i + 1)
            << R"(" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet)"
            << (i + 1) << R"(.xml"/>)" << '\n';
    }

    // 检查是否有字符串数据需要共享字符串表
    bool has_string_data = false;
    for (const auto& worksheet : worksheets_) {
        for (std::size_t r = 1; r <= worksheet->max_row(); ++r) {
            for (std::size_t c = 1; c <= worksheet->max_column(); ++c) {
                try {
                    auto& cell = worksheet->cell(r, c);
                    if (!cell.empty()) {
                        const auto& value_variant = cell.raw_value();
                        if (std::holds_alternative<std::string>(value_variant)) {
                            has_string_data = true;
                            break;
                        }
                    }
                } catch (...) {
                    // 忽略空单元格
                }
            }
            if (has_string_data) break;
        }
        if (has_string_data) break;
    }

    // 共享字符串关系（如果有字符串数据）
    if (has_string_data) {
        std::size_t shared_strings_id = worksheets_.size() + 1;
        xml << R"(  <Relationship Id="rId)" << shared_strings_id
            << R"(" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>)" << '\n';
    }

    // 样式关系
    std::size_t styles_id = worksheets_.size() + (has_string_data ? 2 : 1);
    xml << R"(  <Relationship Id="rId)" << styles_id
        << R"(" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>)" << '\n';

    xml << "</Relationships>\n";

    // 转换为字节数组
    const std::string& xml_str = xml.str();
    std::vector<std::byte> xml_bytes(xml_str.size());
    std::memcpy(xml_bytes.data(), xml_str.data(), xml_str.size());

    // 添加到归档
    async::sync_wait(archiver_->add_file("xl/_rels/workbook.xml.rels", std::move(xml_bytes)));
}

void Workbook::Impl::generate_app_props() {
    std::string app_props = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
  <Application>TinaKit</Application>
  <DocSecurity>0</DocSecurity>
  <ScaleCrop>false</ScaleCrop>
  <HeadingPairs>
    <vt:vector size="2" baseType="variant">
      <vt:variant>
        <vt:lpstr>Worksheets</vt:lpstr>
      </vt:variant>
      <vt:variant>
        <vt:i4>)" + std::to_string(worksheets_.size()) + R"(</vt:i4>
      </vt:variant>
    </vt:vector>
  </HeadingPairs>
  <TitlesOfParts>
    <vt:vector size=")" + std::to_string(worksheets_.size()) + R"(" baseType="lpstr">)";
    
    for (const auto& worksheet : worksheets_) {
        app_props += R"(
      <vt:lpstr>)" + worksheet->name() + R"(</vt:lpstr>)";
    }
    
    app_props += R"(
    </vt:vector>
  </TitlesOfParts>
  <Company></Company>
  <LinksUpToDate>false</LinksUpToDate>
  <SharedDoc>false</SharedDoc>
  <HyperlinksChanged>false</HyperlinksChanged>
  <AppVersion>1.0</AppVersion>
</Properties>)";
    
    // 转换为字节数组
    std::vector<std::byte> xml_bytes(app_props.size());
    std::memcpy(xml_bytes.data(), app_props.data(), app_props.size());
    
    // 添加到归档
    async::sync_wait(archiver_->add_file("docProps/app.xml", std::move(xml_bytes)));
}

void Workbook::Impl::generate_core_props() {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    // 格式化为 ISO 8601 时间字符串
    std::stringstream time_stream;
    time_stream << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    std::string timestamp = time_stream.str();
    
    std::string core_props = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <dc:creator>TinaKit</dc:creator>
  <cp:lastModifiedBy>TinaKit</cp:lastModifiedBy>
  <dcterms:created xsi:type="dcterms:W3CDTF">)" + timestamp + R"(</dcterms:created>
  <dcterms:modified xsi:type="dcterms:W3CDTF">)" + timestamp + R"(</dcterms:modified>
</cp:coreProperties>)";
    
    // 转换为字节数组
    std::vector<std::byte> xml_bytes(core_props.size());
    std::memcpy(xml_bytes.data(), core_props.data(), core_props.size());
    
    // 添加到归档
    async::sync_wait(archiver_->add_file("docProps/core.xml", std::move(xml_bytes)));
}

void Workbook::Impl::generate_shared_strings() {
    try {
        // 生成共享字符串 XML
        std::string xml_content = shared_strings_->generate_xml();
        
        // 转换为字节数组
        std::vector<std::byte> xml_bytes(xml_content.size());
        std::memcpy(xml_bytes.data(), xml_content.data(), xml_content.size());
        
        // 添加到归档
        async::sync_wait(archiver_->add_file("xl/sharedStrings.xml", std::move(xml_bytes)));
    } catch (const std::exception& e) {
        throw IOException("Failed to generate shared strings: " + std::string(e.what()));
    }
}

void Workbook::Impl::generate_styles() {
    try {
        // 生成样式 XML
        std::string xml_content = style_manager_->generate_xml();
        
        // 转换为字节数组
        std::vector<std::byte> xml_bytes(xml_content.size());
        std::memcpy(xml_bytes.data(), xml_content.data(), xml_content.size());
        
        // 添加到归档
        async::sync_wait(archiver_->add_file("xl/styles.xml", std::move(xml_bytes)));
    } catch (const std::exception& e) {
        throw IOException("Failed to generate styles: " + std::string(e.what()));
    }
}

void Workbook::Impl::generate_worksheet_xml(std::size_t index) {
    if (index >= worksheets_.size()) return;
    
    auto& sheet = *worksheets_[index];
    std::ostringstream xml;
    
    // XML 声明和工作表根元素
    xml << R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>)" << '\n';
    xml << R"(<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" )"
        << R"(xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">)" << '\n';
    
    // 添加维度信息（Excel 需要这个来正确显示数据）
    xml << R"(  <dimension ref="A1:)" 
        << column_number_to_name(sheet.max_column()) 
        << sheet.max_row() 
        << R"("/>)" << '\n';
    
    // 添加 sheetViews（Excel 需要这个来正确显示）
    xml << R"(  <sheetViews>)" << '\n';
    if (index == 0) {
        xml << R"(    <sheetView tabSelected="1" workbookViewId="0"/>)" << '\n';
    } else {
        xml << R"(    <sheetView workbookViewId="0"/>)" << '\n';
    }
    xml << R"(  </sheetViews>)" << '\n';
    
    // 添加 sheetFormatPr（设置默认行高和列宽）
    xml << R"(  <sheetFormatPr defaultRowHeight="15"/>)" << '\n';
    
    // 工作表数据
    xml << R"(  <sheetData>)" << '\n';
    
    // 遍历所有非空行
    for (std::size_t r = 1; r <= sheet.max_row(); ++r) {
        bool row_has_data = false;
        std::ostringstream row_xml;
        row_xml << R"(    <row r=")" << r << R"(">)" << '\n';
        
        // 遍历行中的所有单元格
        for (std::size_t c = 1; c <= sheet.max_column(); ++c) {
            try {
                auto& cell = sheet.cell(r, c);
                if (!cell.empty()) {
                    row_has_data = true;
                    row_xml << R"(      <c r=")" << cell.address() << R"(")";
                    
                    // 添加样式索引（使用默认样式 0）
                    row_xml << R"( s="0")";
                    
                    // 处理不同类型的单元格值
                    const auto& value_variant = cell.raw_value();
                    if (std::holds_alternative<std::string>(value_variant)) {
                        auto str_value = std::get<std::string>(value_variant);
                        // 使用共享字符串
                        auto ss_index = shared_strings_->add_string(str_value);
                        row_xml << R"( t="s"><v>)" << ss_index << R"(</v></c>)" << '\n';
                    } else if (std::holds_alternative<double>(value_variant)) {
                        row_xml << R"(><v>)" << std::get<double>(value_variant) << R"(</v></c>)" << '\n';
                    } else if (std::holds_alternative<int>(value_variant)) {
                        row_xml << R"(><v>)" << std::get<int>(value_variant) << R"(</v></c>)" << '\n';
                    } else if (std::holds_alternative<bool>(value_variant)) {
                        row_xml << R"( t="b"><v>)" << (std::get<bool>(value_variant) ? 1 : 0) << R"(</v></c>)" << '\n';
                    }
                }
            } catch (...) {
                // 忽略空单元格
            }
        }
        
        if (row_has_data) {
            row_xml << R"(    </row>)" << '\n';
            xml << row_xml.str();
        }
    }
    
    xml << R"(  </sheetData>)" << '\n';
    xml << R"(</worksheet>)" << '\n';
    
    // 转换为字节数组
    const std::string& xml_str = xml.str();
    std::vector<std::byte> xml_bytes(xml_str.size());
    std::memcpy(xml_bytes.data(), xml_str.data(), xml_str.size());
    
    // 添加到归档
    std::string sheet_path = "xl/worksheets/sheet" + std::to_string(index + 1) + ".xml";
    async::sync_wait(archiver_->add_file(sheet_path, std::move(xml_bytes)));
}

void Workbook::Impl::report_error(const std::exception& error) {
    if (error_callback_) {
        error_callback_(error);
    }
}

// =============================================================================
// Workbook 公共接口实现
// =============================================================================

Workbook::Workbook(std::unique_ptr<Impl> impl) 
    : impl_(std::move(impl)) {
}

Workbook::~Workbook() = default;

Workbook::Workbook(Workbook&& other) noexcept
    : impl_(std::move(other.impl_)) {
}

Workbook& Workbook::operator=(Workbook&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

// 静态工厂方法
Workbook Workbook::open(const std::filesystem::path& path) {
    return Workbook(std::make_unique<Impl>(path));
}

async::Task<Workbook> Workbook::open_async(const std::filesystem::path& path) {
    co_return Workbook(std::make_unique<Impl>(path));
}

Workbook Workbook::create() {
    return Workbook(std::make_unique<Impl>());
}

// 工作表访问
Worksheet& Workbook::operator[](const std::string& name) {
    return impl_->get_worksheet(name);
}

const Worksheet& Workbook::operator[](const std::string& name) const {
    return impl_->get_worksheet(name);
}

Worksheet& Workbook::operator[](std::size_t index) {
    return impl_->get_worksheet(index);
}

const Worksheet& Workbook::operator[](std::size_t index) const {
    return impl_->get_worksheet(index);
}

Worksheet& Workbook::active_sheet() {
    return impl_->get_active_sheet();
}

const Worksheet& Workbook::active_sheet() const {
    return impl_->get_active_sheet();
}

// 工作表管理
Worksheet& Workbook::add_sheet(const std::string& name) {
    return impl_->add_worksheet(name);
}

void Workbook::remove_sheet(const std::string& name) {
    impl_->remove_worksheet(name);
}

void Workbook::remove_sheet(std::size_t index) {
    impl_->remove_worksheet(index);
}

std::vector<std::reference_wrapper<Worksheet>> Workbook::worksheets() {
    return impl_->get_worksheets();
}

std::vector<std::reference_wrapper<const Worksheet>> Workbook::worksheets() const {
    std::vector<std::reference_wrapper<const Worksheet>> result;
    auto worksheets = impl_->get_worksheets();
    result.reserve(worksheets.size());
    for (const auto& ws : worksheets) {
        result.emplace_back(ws.get());
    }
    return result;
}

std::size_t Workbook::sheet_count() const noexcept {
    return impl_->worksheet_count();
}

// 文件操作
void Workbook::save(const std::filesystem::path& path) {
    impl_->save(path);
}

async::Task<void> Workbook::save_async(const std::filesystem::path& path) {
    co_await impl_->save_async(path);
}

void Workbook::save_as(const std::filesystem::path& path) {
    impl_->save(path);
}

async::Task<void> Workbook::save_as_async(const std::filesystem::path& path) {
    co_await impl_->save_async(path);
}

// 回调支持
void Workbook::on_error(ErrorCallback callback) {
    impl_->set_error_callback(std::move(callback));
}

const std::filesystem::path& Workbook::file_path() const noexcept {
    return impl_->get_file_path();
}

bool Workbook::has_unsaved_changes() const noexcept {
    return impl_->has_unsaved_changes();
}

std::size_t Workbook::file_size() const {
    return impl_->get_file_size();
}

SharedStrings& Workbook::shared_strings() {
    return impl_->get_shared_strings();
}

const SharedStrings& Workbook::shared_strings() const {
    return impl_->get_shared_strings();
}

StyleManager& Workbook::style_manager() {
    return impl_->get_style_manager();
}

const StyleManager& Workbook::style_manager() const {
    return impl_->get_style_manager();
}

} // namespace tinakit::excel
