/**
 * @file workbook.cpp
 * @brief Excel 工作簿类实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/xml_parser.hpp"
#include "tinakit/core/exceptions.hpp"
#include <sstream>
#include <algorithm>
#include <format>

namespace tinakit::excel {

/**
 * @class Impl
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

    private:
    void load_from_archiver();
    void create_default_structure();
    void parse_workbook_rels();
    void parse_workbook_xml();
    void parse_shared_strings();
    void regenerate_metadata_on_save();
    
    // OpenXML 结构生成
    void generate_content_types();
    void generate_workbook_xml();
    void generate_workbook_rels();
    void generate_app_props();
    void generate_core_props();
    
    void report_error(const std::exception& error);

private:
        std::shared_ptr<core::OpenXmlArchiver> archiver_;
    std::filesystem::path original_path_;
    bool is_modified_ = false;
    std::size_t active_sheet_index_ = 0;

        std::vector<WorksheetPtr> worksheets_;
    std::map<std::string, WorksheetPtr, std::less<>> worksheet_by_name_;
    std::map<std::string, std::string, std::less<>> rels_map_;
    std::vector<std::string> shared_strings_;
    
    ErrorCallback error_callback_;
};

// =============================================================================
// Impl 实现
// =============================================================================

Workbook::Impl::Impl(const std::filesystem::path& path)
    : original_path_(path) {
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
    : is_modified_(true) {
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
    // TODO: 解析 xl/_rels/workbook.xml.rels
}

void Workbook::Impl::parse_workbook_xml() {
    // TODO: 解析 xl/workbook.xml
}

void Workbook::Impl::parse_shared_strings() {
    // TODO: 解析 xl/sharedStrings.xml
}

void Workbook::Impl::regenerate_metadata_on_save() {
    if (!is_modified_) return;
    
    // 重新生成所有元数据文件
    generate_content_types();
    generate_workbook_xml();
    generate_workbook_rels();
    generate_app_props();
    generate_core_props();
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

    content_types += R"(
</Types>)";

    auto content_bytes = std::vector<std::byte>(content_types.size());
    std::memcpy(content_bytes.data(), content_types.data(), content_types.size());
    
    async::sync_wait(archiver_->add_file("[Content_Types].xml", std::move(content_bytes)));
}

void Workbook::Impl::generate_workbook_xml() {
    // TODO: 生成 xl/workbook.xml
}

void Workbook::Impl::generate_workbook_rels() {
    // TODO: 生成 xl/_rels/workbook.xml.rels
}

void Workbook::Impl::generate_app_props() {
    // TODO: 生成 docProps/app.xml
}

void Workbook::Impl::generate_core_props() {
    // TODO: 生成 docProps/core.xml
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

} // namespace tinakit::excel
