/**
 * @file workbook_impl.cpp
 * @brief 工作簿内部实现类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/worksheet_impl.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/excel/formula_engine.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/async.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/xml_parser.hpp"

#include "tinakit/excel/openxml_namespaces.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace tinakit::internal {

// ========================================
// 构造函数和析构函数
// ========================================

workbook_impl::workbook_impl()
    : style_manager_(std::make_shared<excel::StyleManager>()),
      shared_strings_(std::make_shared<excel::SharedStrings>()),
      string_pool_(std::make_unique<core::StringPool>()),
      cell_data_pool_(std::make_unique<core::MemoryPool<cell_data>>()),
      formula_engine_(std::make_unique<excel::FormulaEngine>(this)) {
    // 不在构造函数中创建默认结构，延迟到需要时创建
}

workbook_impl::workbook_impl(const std::filesystem::path& file_path)
    : file_path_(file_path),
      style_manager_(std::make_shared<excel::StyleManager>()),
      shared_strings_(std::make_shared<excel::SharedStrings>()),
      string_pool_(std::make_unique<core::StringPool>()),
      cell_data_pool_(std::make_unique<core::MemoryPool<cell_data>>()),
      formula_engine_(std::make_unique<excel::FormulaEngine>(this)) {
    load_from_file();
}

workbook_impl::~workbook_impl() = default;

// ========================================
// 工作表管理
// ========================================

std::size_t workbook_impl::worksheet_count() const {
    return worksheet_order_.size();
}

std::vector<std::string> workbook_impl::worksheet_names() const {
    return worksheet_order_;
}

bool workbook_impl::has_worksheet(const std::string& name) const {
    return worksheets_.find(name) != worksheets_.end();
}

bool workbook_impl::has_worksheet(std::uint32_t sheet_id) const {
    return sheet_id_to_name_.find(sheet_id) != sheet_id_to_name_.end();
}

std::string workbook_impl::get_sheet_name(std::uint32_t sheet_id) const {
    auto it = sheet_id_to_name_.find(sheet_id);
    if (it != sheet_id_to_name_.end()) {
        return it->second;
    }
    throw std::invalid_argument("Sheet ID " + std::to_string(sheet_id) + " does not exist");
}

std::uint32_t workbook_impl::get_sheet_id(const std::string& name) const {
    auto it = sheet_name_to_id_.find(name);
    if (it != sheet_name_to_id_.end()) {
        return it->second;
    }
    throw std::invalid_argument("Sheet '" + name + "' does not exist");
}

void workbook_impl::ensure_has_worksheet() {
    if (worksheets_.empty()) {
        create_worksheet("Sheet1");
        if (active_sheet_name_.empty()) {
            active_sheet_name_ = "Sheet1";
        }
    }
}

excel::Worksheet workbook_impl::create_worksheet(const std::string& name) {
    if (has_worksheet(name)) {
        throw DuplicateWorksheetNameException(name);
    }

    // 分配新的sheet_id
    std::uint32_t sheet_id = next_sheet_id_++;

    // 建立映射关系
    sheet_name_to_id_[name] = sheet_id;
    sheet_id_to_name_[sheet_id] = name;

    auto worksheet = std::make_unique<worksheet_impl>(name, *this);
    worksheets_[name] = std::move(worksheet);
    worksheet_order_.push_back(name);

    // 如果这是第一个工作表，设为活动工作表
    if (active_sheet_name_.empty()) {
        active_sheet_name_ = name;
    }

    is_dirty_ = true;

    // 返回工作表对象
    return excel::Worksheet(shared_from_this(), sheet_id, name);
}

void workbook_impl::remove_worksheet(const std::string& name) {
    if (!has_worksheet(name)) {
        throw WorksheetNotFoundException(name);
    }

    if (worksheet_count() <= 1) {
        throw CannotDeleteLastWorksheetException();
    }
    
    worksheets_.erase(name);
    worksheet_order_.erase(
        std::remove(worksheet_order_.begin(), worksheet_order_.end(), name),
        worksheet_order_.end()
    );
    
    is_dirty_ = true;
}

void workbook_impl::rename_worksheet(const std::string& old_name, const std::string& new_name) {
    if (!has_worksheet(old_name)) {
        throw WorksheetNotFoundException(old_name);
    }

    if (has_worksheet(new_name)) {
        throw DuplicateWorksheetNameException(new_name);
    }

    // 获取sheet_id（在移动之前）
    auto sheet_id = sheet_name_to_id_[old_name];

    // 移动工作表实现
    auto worksheet = std::move(worksheets_[old_name]);
    worksheet->set_name(new_name);
    worksheets_.erase(old_name);
    worksheets_[new_name] = std::move(worksheet);

    // 更新ID映射（关键！）
    sheet_name_to_id_.erase(old_name);
    sheet_name_to_id_[new_name] = sheet_id;
    sheet_id_to_name_[sheet_id] = new_name;  // 更新反向映射

    // 更新顺序列表
    auto it = std::find(worksheet_order_.begin(), worksheet_order_.end(), old_name);
    if (it != worksheet_order_.end()) {
        *it = new_name;
    }

    is_dirty_ = true;
}

// ========================================
// 单元格数据访问
// ========================================

cell_data workbook_impl::get_cell_data(const std::string& sheet_name, const core::Coordinate& pos) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    return worksheet.get_cell_data(pos);
}

cell_data workbook_impl::get_cell_data(const std::string& sheet_name, const core::Coordinate& pos) const {
    auto it = worksheets_.find(sheet_name);
    if (it == worksheets_.end()) {
        return cell_data(); // 返回默认的空cell_data
    }
    return it->second->get_cell_data(pos);
}

cell_data workbook_impl::get_cell_data(std::uint32_t sheet_id, const core::Coordinate& pos) {
    std::string sheet_name = get_sheet_name(sheet_id);
    return get_cell_data(sheet_name, pos);
}

void workbook_impl::set_cell_value(const std::string& sheet_name, const core::Coordinate& pos,
                                  const cell_data::CellValue& value) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    cell_data data;

    // 性能优化：对字符串值使用全局字符串缓存
    if (std::holds_alternative<std::string>(value)) {
        const auto& str_value = std::get<std::string>(value);
        // 使用CacheManager的字符串缓存
        auto& string_cache = core::CacheManager::instance().string_cache();
        auto string_id = string_cache.intern_string(str_value);
        auto optimized_str = string_cache.get_string(string_id);
        data.value = std::string(optimized_str);
    } else {
        data.value = value;
    }

    worksheet.set_cell_data(pos, data);
    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::set_cell_value(std::uint32_t sheet_id, const core::Coordinate& pos,
                                  const cell_data::CellValue& value) {
    std::string sheet_name = get_sheet_name(sheet_id);
    set_cell_value(sheet_name, pos, value);
}

void workbook_impl::set_cell_formula(const std::string& sheet_name, const core::Coordinate& pos,
                                    const std::string& formula) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    auto data = worksheet.get_cell_data(pos);
    data.formula = formula;
    worksheet.set_cell_data(pos, data);

    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::set_cell_formula(std::uint32_t sheet_id, const core::Coordinate& pos,
                                    const std::string& formula) {
    std::string sheet_name = get_sheet_name(sheet_id);
    set_cell_formula(sheet_name, pos, formula);
}

void workbook_impl::set_cell_style(const std::string& sheet_name, const core::Coordinate& pos,
                                  std::uint32_t style_id) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    auto data = worksheet.get_cell_data(pos);
    data.style_id = style_id;
    worksheet.set_cell_data(pos, data);

    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::set_cell_style(std::uint32_t sheet_id, const core::Coordinate& pos,
                                  std::uint32_t style_id) {
    std::string sheet_name = get_sheet_name(sheet_id);
    set_cell_style(sheet_name, pos, style_id);
}



void workbook_impl::batch_set_cell_values(const std::string& sheet_name,
                                         const std::vector<std::tuple<core::Coordinate, cell_data::CellValue>>& operations) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    // 使用全局缓存管理器
    auto& cache_manager = core::CacheManager::instance();
    auto& string_cache = cache_manager.string_cache();

    // 性能优化：批量处理，减少重复的查找和验证
    for (const auto& [pos, value] : operations) {
        cell_data data;

        // 使用全局字符串缓存优化字符串存储
        if (std::holds_alternative<std::string>(value)) {
            const auto& str_value = std::get<std::string>(value);
            auto string_id = string_cache.intern_string(str_value);
            auto optimized_str = string_cache.get_string(string_id);
            data.value = std::string(optimized_str);
        } else {
            data.value = value;
        }

        worksheet.set_cell_data(pos, data);
    }

    mark_worksheet_dirty(sheet_name);
}

workbook_impl::PerformanceStats workbook_impl::get_performance_stats() const {
    PerformanceStats stats;

    // 使用CacheManager获取统计信息
    auto& cache_manager = core::CacheManager::instance();
    auto& string_cache = cache_manager.string_cache();
    auto& cell_cache = cache_manager.cell_cache();

    stats.string_pool_size = string_cache.string_pool_.size();
    stats.cell_cache_hits = 0;  // cell_cache.hit_count(); // 需要添加这个方法
    stats.cell_cache_misses = 0; // cell_cache.miss_count(); // 需要添加这个方法
    stats.cache_hit_ratio = 0.0; // cell_cache.hit_ratio(); // 需要添加这个方法

    return stats;
}

// ========================================
// 样式管理
// ========================================

excel::StyleManager& workbook_impl::style_manager() {
    return *style_manager_;
}

const excel::StyleManager& workbook_impl::style_manager() const {
    return *style_manager_;
}

excel::SharedStrings& workbook_impl::shared_strings() {
    return *shared_strings_;
}

const excel::SharedStrings& workbook_impl::shared_strings() const {
    return *shared_strings_;
}

// ========================================
// 文件操作
// ========================================

void workbook_impl::save(const std::filesystem::path& file_path) {
    file_path_ = file_path;
    save();
}

void workbook_impl::save() {
    if (file_path_.empty()) {
        throw std::invalid_argument("No file path specified");
    }

    ensure_has_worksheet();  // 确保至少有一个工作表
    save_to_archiver();
    is_dirty_ = false;

    // 清除所有工作表的修改标志
    for (auto& [name, worksheet] : worksheets_) {
        worksheet->clear_dirty();
    }
}

const std::filesystem::path& workbook_impl::file_path() const {
    return file_path_;
}

// ========================================
// 内部状态管理
// ========================================

void workbook_impl::ensure_worksheet_loaded(const std::string& sheet_name) {
    if (!has_worksheet(sheet_name)) {
        throw WorksheetNotFoundException(sheet_name);
    }

    auto& worksheet = get_worksheet_impl(sheet_name);
    if (worksheet.load_state() == LoadState::NotLoaded) {
        // 触发惰性加载
        worksheet.load_all();
    }
}

void workbook_impl::mark_worksheet_dirty(const std::string& sheet_name) {
    if (has_worksheet(sheet_name)) {
        get_worksheet_impl(sheet_name).mark_dirty();
    }
    is_dirty_ = true;
}

worksheet_impl& workbook_impl::get_worksheet_impl(const std::string& sheet_name) {
    auto it = worksheets_.find(sheet_name);
    if (it == worksheets_.end()) {
        throw WorksheetNotFoundException(sheet_name);
    }
    return *it->second;
}

// ========================================
// 私有方法
// ========================================

void workbook_impl::load_from_file() {
    try {
        // 使用 OpenXmlArchiver 打开文件
        archiver_ = std::make_shared<core::OpenXmlArchiver>(
            async::sync_wait(core::OpenXmlArchiver::open_from_file(file_path_.string())));

        // 解析工作簿结构
        parse_workbook_xml();
        parse_workbook_rels();

        // 加载样式信息
        load_styles_xml();

        // 加载共享字符串
        load_shared_strings_xml();

        // 标记为未修改
        is_dirty_ = false;

    } catch (const std::exception&) {
        // 如果加载失败，创建默认结构
        create_default_structure();
    }
}

void workbook_impl::create_default_structure() {
    // 只有在没有工作表时才创建默认工作表
    if (worksheets_.empty()) {
        create_worksheet("Sheet1");
        // 设置活动工作表
        if (active_sheet_name_.empty()) {
            active_sheet_name_ = "Sheet1";
        }
    }
}

void workbook_impl::parse_workbook_xml() {
    if (!archiver_) return;

    try {
        // 检查是否有 workbook.xml 文件
        if (async::sync_wait(archiver_->has_file("xl/workbook.xml"))) {
            auto xml_data = async::sync_wait(archiver_->read_file("xl/workbook.xml"));

            // 将字节数据转换为字符串
            std::string xml_content(
                reinterpret_cast<const char*>(xml_data.data()),
                xml_data.size()
            );

            // 使用 XmlParser 解析工作表信息
            std::istringstream xml_stream(xml_content);
            core::XmlParser parser(xml_stream, "workbook.xml");

            // 解析工作表列表
            parser.for_each_element("sheet", [this](core::XmlParser::iterator& it) {
                auto name = it.attribute("name");
                auto sheet_id = it.attribute("sheetId");
                // 尝试不同的r:id属性格式
                auto r_id = it.attribute("r:id");
                if (!r_id) {
                    r_id = it.attribute(excel::OpenXmlNamespaces::relationship_attr("id"));
                }

                if (name && !name->empty()) {
                    // 创建工作表（如果不存在）
                    if (!has_worksheet(*name)) {
                        create_worksheet(*name);
                    }
                }
            });

            // 如果没有找到工作表，不自动创建，延迟到需要时创建
            // 这与延迟创建策略保持一致
        } else {
            create_default_structure();
        }
    } catch (const std::exception&) {
        create_default_structure();
    }
}

void workbook_impl::parse_workbook_rels() {
    if (!archiver_) return;

    try {
        // 检查是否有 workbook.xml.rels 文件
        if (async::sync_wait(archiver_->has_file("xl/_rels/workbook.xml.rels"))) {
            auto xml_data = async::sync_wait(archiver_->read_file("xl/_rels/workbook.xml.rels"));

            // 将字节数据转换为字符串
            std::string xml_content(
                reinterpret_cast<const char*>(xml_data.data()),
                xml_data.size()
            );

            // 使用 XmlParser 解析关系信息
            std::istringstream xml_stream(xml_content);
            core::XmlParser parser(xml_stream, "workbook.xml.rels");

            // 解析关系映射
            parser.for_each_element("Relationship", [this](core::XmlParser::iterator& it) {
                auto id = it.attribute("Id");
                auto type = it.attribute("Type");
                auto target = it.attribute("Target");

                // 检查是否是工作表关系
                if (type && target && type->find("worksheet") != std::string::npos && !target->empty()) {
                    // 存储关系映射（可以用于后续的工作表加载）
                    // 这里可以建立 rId 到工作表文件路径的映射
                }
            });
        }
    } catch (const std::exception&) {
        // 忽略解析错误
    }
}

void workbook_impl::load_styles_xml() {
    if (!archiver_) return;

    try {
        // 检查是否有 styles.xml 文件
        if (async::sync_wait(archiver_->has_file("xl/styles.xml"))) {
            auto xml_data = async::sync_wait(archiver_->read_file("xl/styles.xml"));

            // 将字节数据转换为字符串
            std::string xml_content(
                reinterpret_cast<const char*>(xml_data.data()),
                xml_data.size()
            );

            // 使用 StyleManager 加载样式
            style_manager_->load_from_xml(xml_content);

        } else {
            // 如果没有样式文件，确保有默认样式
            style_manager_->initialize_defaults();
        }
    } catch (const std::exception& e) {
        // 如果加载失败，使用默认样式
        style_manager_->clear();
        style_manager_->initialize_defaults();
    }
}

void workbook_impl::load_shared_strings_xml() {
    if (!archiver_) return;

    try {
        // 检查是否有 sharedStrings.xml 文件
        if (async::sync_wait(archiver_->has_file("xl/sharedStrings.xml"))) {
            auto xml_data = async::sync_wait(archiver_->read_file("xl/sharedStrings.xml"));

            // 将字节数据转换为字符串
            std::string xml_content(
                reinterpret_cast<const char*>(xml_data.data()),
                xml_data.size()
            );

            // 加载共享字符串数据
            shared_strings_->load_from_xml(xml_content);

        }
    } catch (const std::exception& e) {
    }
}

void workbook_impl::generate_workbook_xml() {
    std::ostringstream oss;
    core::XmlSerializer serializer(oss, "workbook.xml");

    // XML声明
    serializer.xml_declaration("1.0", "UTF-8", "yes");

    // 开始workbook元素（使用命名空间）
    serializer.start_element(excel::openxml_ns::main, "workbook");

    // 声明命名空间（必须在start_element之后）
    serializer.namespace_declaration(excel::openxml_ns::main, "");
    serializer.namespace_declaration(excel::openxml_ns::rel, excel::openxml_ns::r_prefix);

    // 开始sheets元素（明确使用主命名空间）
    serializer.start_element(excel::openxml_ns::main, "sheets");

    // 添加工作表信息
    std::uint32_t sheet_id = 1;
    for (const auto& sheet_name : worksheet_order_) {
        serializer.start_element(excel::openxml_ns::main, "sheet");
        serializer.attribute("name", sheet_name);
        serializer.attribute("sheetId", std::to_string(sheet_id));
        serializer.attribute(excel::openxml_ns::rel, "id", "rId" + std::to_string(sheet_id));
        serializer.end_element(); // sheet
        ++sheet_id;
    }

    serializer.end_element(); // sheets
    serializer.end_element(); // workbook

    // 获取生成的XML内容
    std::string xml_content = oss.str();



    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("xl/workbook.xml", std::move(xml_bytes)));
}

void workbook_impl::generate_workbook_rels() {
    std::ostringstream oss;
    core::XmlSerializer serializer(oss, "workbook.xml.rels");

    // XML声明
    serializer.xml_declaration("1.0", "UTF-8", "yes");

    // 开始Relationships元素（使用命名空间）
    serializer.start_element(excel::openxml_ns::pkg_rel, "Relationships");

    // 声明命名空间
    serializer.namespace_declaration(excel::openxml_ns::pkg_rel, "");

    // 添加工作表关系
    std::uint32_t rel_id = 1;
    for (const auto& sheet_name : worksheet_order_) {
        serializer.start_element(excel::openxml_ns::pkg_rel, "Relationship");
        serializer.attribute("Id", "rId" + std::to_string(rel_id));
        serializer.attribute("Type", excel::openxml_ns::rel + "/worksheet");
        serializer.attribute("Target", "worksheets/sheet" + std::to_string(rel_id) + ".xml");
        serializer.end_element(); // Relationship
        ++rel_id;
    }

    // 添加样式关系（关键！）
    serializer.start_element(excel::openxml_ns::pkg_rel, "Relationship");
    serializer.attribute("Id", "rId" + std::to_string(rel_id));
    serializer.attribute("Type", excel::openxml_ns::rel + "/styles");
    serializer.attribute("Target", "styles.xml");
    serializer.end_element(); // Relationship
    ++rel_id;

    // 添加共享字符串关系
    serializer.start_element(excel::openxml_ns::pkg_rel, "Relationship");
    serializer.attribute("Id", "rId" + std::to_string(rel_id));
    serializer.attribute("Type", excel::openxml_ns::rel + "/sharedStrings");
    serializer.attribute("Target", "sharedStrings.xml");
    serializer.end_element(); // Relationship

    serializer.end_element(); // Relationships

    // 获取生成的XML内容
    std::string xml_content = oss.str();

    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("xl/_rels/workbook.xml.rels", std::move(xml_bytes)));
}

void workbook_impl::generate_content_types() {
    std::ostringstream oss;
    core::XmlSerializer serializer(oss, "[Content_Types].xml");

    // XML声明
    serializer.xml_declaration("1.0", "UTF-8", "yes");

    // 开始Types元素（使用命名空间）
    serializer.start_element(excel::openxml_ns::ct, "Types");

    // 声明命名空间
    serializer.namespace_declaration(excel::openxml_ns::ct, "");

    // 添加默认扩展名
    serializer.start_element(excel::openxml_ns::ct, "Default");
    serializer.attribute("Extension", "rels");
    serializer.attribute("ContentType", "application/vnd.openxmlformats-package.relationships+xml");
    serializer.end_element(); // Default

    serializer.start_element(excel::openxml_ns::ct, "Default");
    serializer.attribute("Extension", "xml");
    serializer.attribute("ContentType", "application/xml");
    serializer.end_element(); // Default

    // 添加Override元素
    serializer.start_element(excel::openxml_ns::ct, "Override");
    serializer.attribute("PartName", "/xl/workbook.xml");
    serializer.attribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml");
    serializer.end_element(); // Override

    serializer.start_element(excel::openxml_ns::ct, "Override");
    serializer.attribute("PartName", "/xl/styles.xml");
    serializer.attribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml");
    serializer.end_element(); // Override

    serializer.start_element(excel::openxml_ns::ct, "Override");
    serializer.attribute("PartName", "/xl/sharedStrings.xml");
    serializer.attribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml");
    serializer.end_element(); // Override

    // 添加工作表内容类型
    std::uint32_t sheet_id = 1;
    for (const auto& sheet_name : worksheet_order_) {
        serializer.start_element(excel::openxml_ns::ct, "Override");
        serializer.attribute("PartName", "/xl/worksheets/sheet" + std::to_string(sheet_id) + ".xml");
        serializer.attribute("ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml");
        serializer.end_element(); // Override
        ++sheet_id;
    }

    serializer.end_element(); // Types

    // 获取生成的XML内容
    std::string xml_content = oss.str();



    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("[Content_Types].xml", std::move(xml_bytes)));
}

void workbook_impl::generate_main_rels() {
    std::ostringstream oss;
    core::XmlSerializer serializer(oss, ".rels");

    // XML声明
    serializer.xml_declaration("1.0", "UTF-8", "yes");

    // 开始Relationships元素（使用命名空间）
    serializer.start_element(excel::openxml_ns::pkg_rel, "Relationships");

    // 声明命名空间
    serializer.namespace_declaration(excel::openxml_ns::pkg_rel, "");

    // 添加主文档关系
    serializer.start_element(excel::openxml_ns::pkg_rel, "Relationship");
    serializer.attribute("Id", "rId1");
    serializer.attribute("Type", excel::openxml_ns::rel + "/officeDocument");
    serializer.attribute("Target", "xl/workbook.xml");
    serializer.end_element(); // Relationship

    serializer.end_element(); // Relationships

    // 获取生成的XML内容
    std::string xml_content = oss.str();

    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("_rels/.rels", std::move(xml_bytes)));
}

void workbook_impl::generate_styles_xml() {
    // 使用 StyleManager 生成完整的样式XML
    std::string xml_content = style_manager_->generate_xml();



    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("xl/styles.xml", std::move(xml_bytes)));
}

void workbook_impl::generate_shared_strings_xml() {
    // 使用共享字符串管理器生成XML
    std::string xml_content;



    if (shared_strings_ && shared_strings_->count() > 0) {
        xml_content = shared_strings_->generate_xml();
    } else {
        // 如果没有共享字符串，使用序列化器生成空的共享字符串表
        std::ostringstream oss;
        core::XmlSerializer serializer(oss, "sharedStrings.xml");

        serializer.xml_declaration("1.0", "UTF-8", "yes");
        serializer.start_element(excel::openxml_ns::main, "sst");
        serializer.namespace_declaration(excel::openxml_ns::main, "");
        serializer.attribute("count", "0");
        serializer.attribute("uniqueCount", "0");
        serializer.end_element(); // sst

        xml_content = oss.str();
    }

    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("xl/sharedStrings.xml", std::move(xml_bytes)));
}

void workbook_impl::save_to_archiver() {
    if (!archiver_) {
        // 创建新的归档器
        auto temp_archiver = core::OpenXmlArchiver::create_in_memory_writer();
        archiver_ = std::make_shared<core::OpenXmlArchiver>(std::move(temp_archiver));
    }

    // 1. 生成 [Content_Types].xml
    generate_content_types();

    // 2. 生成主关系文件 _rels/.rels
    generate_main_rels();

    // 3. 生成并保存工作簿XML
    generate_workbook_xml();
    generate_workbook_rels();

    // 4. 生成样式文件
    generate_styles_xml();

    // 5. 先保存所有工作表（这会填充共享字符串表）
    for (auto& [name, worksheet] : worksheets_) {
        worksheet->save_to_archiver(*archiver_);
    }

    // 6. 最后生成共享字符串文件（包含所有字符串）
    generate_shared_strings_xml();

    // 7. 保存到文件
    async::sync_wait(archiver_->save_to_file(file_path_.string()));
}

// ========================================
// 新增的委托方法实现
// ========================================



std::optional<cell_data> workbook_impl::get_cell_data(const core::Coordinate& pos) {
    if (active_sheet_name_.empty()) {
        return std::nullopt;
    }

    try {
        auto data = get_cell_data(active_sheet_name_, pos);
        return data;
    } catch (...) {
        return std::nullopt;
    }
}

excel::Range workbook_impl::get_used_range(const std::string& sheet_name) {
    ensure_worksheet_loaded(sheet_name);

    auto it = worksheets_.find(sheet_name);
    if (it == worksheets_.end()) {
        return excel::Range(); // 返回空范围
    }

    // 获取工作表的使用范围
    auto& worksheet = *it->second;
    auto used_range = worksheet.get_used_range();

    return used_range;
}

void workbook_impl::add_conditional_format(const std::string& sheet_name, const excel::ConditionalFormat& format) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    worksheet.add_conditional_format(format);
    is_dirty_ = true;
}

const std::vector<excel::ConditionalFormat>& workbook_impl::get_conditional_formats(const std::string& sheet_name) const {
    const_cast<workbook_impl*>(this)->ensure_worksheet_loaded(sheet_name);
    const auto& worksheet = const_cast<workbook_impl*>(this)->get_worksheet_impl(sheet_name);
    return worksheet.get_conditional_formats();
}



worksheet_impl& workbook_impl::get_worksheet_impl_public(const std::string& sheet_name) {
    ensure_worksheet_loaded(sheet_name);
    return get_worksheet_impl(sheet_name);
}

// ========================================
// Range 批量操作方法实现
// ========================================

void workbook_impl::set_range_values(const std::string& sheet_name,
                                     const core::range_address& range_addr,
                                     const std::vector<std::vector<cell_data::CellValue>>& values) {
    ensure_worksheet_loaded(sheet_name);

    // 计算范围大小
    std::size_t rows = range_addr.end.row - range_addr.start.row + 1;
    std::size_t cols = range_addr.end.column - range_addr.start.column + 1;

    // 批量设置值
    for (std::size_t r = 0; r < rows && r < values.size(); ++r) {
        const auto& row_values = values[r];
        for (std::size_t c = 0; c < cols && c < row_values.size(); ++c) {
            core::Coordinate pos(range_addr.start.row + r, range_addr.start.column + c);
            set_cell_value(sheet_name, pos, row_values[c]);
        }
    }
}

template<typename T>
void workbook_impl::set_range_value_uniform(const std::string& sheet_name,
                                           const core::range_address& range_addr,
                                           const T& value) {
    ensure_worksheet_loaded(sheet_name);

    // 批量设置相同值
    for (std::size_t r = range_addr.start.row; r <= range_addr.end.row; ++r) {
        for (std::size_t c = range_addr.start.column; c <= range_addr.end.column; ++c) {
            core::Coordinate pos(r, c);
            set_cell_value(sheet_name, pos, cell_data::CellValue(value));
        }
    }
}

// 显式实例化常用类型
template void workbook_impl::set_range_value_uniform<std::string>(const std::string&, const core::range_address&, const std::string&);
template void workbook_impl::set_range_value_uniform<int>(const std::string&, const core::range_address&, const int&);
template void workbook_impl::set_range_value_uniform<double>(const std::string&, const core::range_address&, const double&);
template void workbook_impl::set_range_value_uniform<bool>(const std::string&, const core::range_address&, const bool&);

void workbook_impl::set_range_style(const std::string& sheet_name,
                                   const core::range_address& range_addr,
                                   std::uint32_t style_id) {
    ensure_worksheet_loaded(sheet_name);

    // 批量设置样式
    for (std::size_t r = range_addr.start.row; r <= range_addr.end.row; ++r) {
        for (std::size_t c = range_addr.start.column; c <= range_addr.end.column; ++c) {
            core::Coordinate pos(r, c);
            set_cell_style(sheet_name, pos, style_id);
        }
    }
}

void workbook_impl::clear_range(const std::string& sheet_name,
                               const core::range_address& range_addr) {
    ensure_worksheet_loaded(sheet_name);

    // 批量清除内容
    for (std::size_t r = range_addr.start.row; r <= range_addr.end.row; ++r) {
        for (std::size_t c = range_addr.start.column; c <= range_addr.end.column; ++c) {
            core::Coordinate pos(r, c);
            // 设置为空字符串来清除内容
            set_cell_value(sheet_name, pos, cell_data::CellValue(std::string("")));
        }
    }
}

std::vector<std::vector<cell_data::CellValue>> workbook_impl::get_range_values(
    const std::string& sheet_name,
    const core::range_address& range_addr) const {

    std::vector<std::vector<cell_data::CellValue>> result;

    // 计算范围大小
    std::size_t rows = range_addr.end.row - range_addr.start.row + 1;
    std::size_t cols = range_addr.end.column - range_addr.start.column + 1;

    result.reserve(rows);

    // 批量获取值
    for (std::size_t r = 0; r < rows; ++r) {
        std::vector<cell_data::CellValue> row_values;
        row_values.reserve(cols);

        for (std::size_t c = 0; c < cols; ++c) {
            core::Coordinate pos(range_addr.start.row + r, range_addr.start.column + c);
            auto cell_data = get_cell_data(sheet_name, pos);
            row_values.push_back(cell_data.value);
        }

        result.push_back(std::move(row_values));
    }

    return result;
}

void workbook_impl::merge_range(const std::string& sheet_name,
                               const core::range_address& range_addr) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    // 添加到合并范围列表
    worksheet.add_merged_range(range_addr);
    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::unmerge_range(const std::string& sheet_name,
                                 const core::range_address& range_addr) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    // 从合并范围列表中移除
    worksheet.remove_merged_range(range_addr);
    mark_worksheet_dirty(sheet_name);
}

bool workbook_impl::is_range_merged(const std::string& sheet_name,
                                   const core::range_address& range_addr) const {
    auto it = worksheets_.find(sheet_name);
    if (it == worksheets_.end()) {
        return false;
    }

    return it->second->is_range_merged(range_addr);
}

void workbook_impl::copy_range(const std::string& source_sheet,
                              const core::range_address& source_range,
                              const std::string& dest_sheet,
                              const core::range_address& dest_range) {
    ensure_worksheet_loaded(source_sheet);
    ensure_worksheet_loaded(dest_sheet);

    // 获取源范围的值
    auto values = get_range_values(source_sheet, source_range);

    // 设置到目标范围
    set_range_values(dest_sheet, dest_range, values);

    // 复制样式（简化实现，只复制第一个单元格的样式到整个目标范围）
    auto source_cell_data = get_cell_data(source_sheet, source_range.start);
    if (source_cell_data.style_id > 0) {
        set_range_style(dest_sheet, dest_range, source_cell_data.style_id);
    }
}

void workbook_impl::move_range(const std::string& source_sheet,
                              const core::range_address& source_range,
                              const std::string& dest_sheet,
                              const core::range_address& dest_range) {
    // 先复制
    copy_range(source_sheet, source_range, dest_sheet, dest_range);

    // 然后清除源范围
    clear_range(source_sheet, source_range);
}

// ========================================
// 公式计算
// ========================================

excel::FormulaEngine& workbook_impl::formula_engine() {
    return *formula_engine_;
}

const excel::FormulaEngine& workbook_impl::formula_engine() const {
    return *formula_engine_;
}

cell_data::CellValue workbook_impl::calculate_formula(const std::string& sheet_name, const core::Coordinate& pos) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    if (!worksheet.has_cell_data(pos)) {
        return std::monostate{}; // 返回真正的空单元格状态
    }

    auto cell_data_obj = worksheet.get_cell_data(pos);
    if (!cell_data_obj.formula.has_value() || cell_data_obj.formula->empty()) {
        return cell_data_obj.value;
    }

    try {
        auto result = formula_engine_->evaluate(*cell_data_obj.formula, sheet_name);

        // 将FormulaResult转换为CellValue
        if (std::holds_alternative<double>(result)) {
            return std::get<double>(result);
        } else if (std::holds_alternative<std::string>(result)) {
            return std::get<std::string>(result);
        } else if (std::holds_alternative<bool>(result)) {
            return std::get<bool>(result);
        } else {
            return std::monostate{}; // 返回空值状态
        }
    } catch (const excel::FormulaException& e) {
        // 公式计算错误，返回错误字符串
        return std::string("#ERROR: ") + e.what();
    }
}

void workbook_impl::recalculate_formulas(const std::string& sheet_name) {
    if (sheet_name.empty()) {
        // 重新计算所有工作表的公式
        for (const auto& name : worksheet_order_) {
            recalculate_formulas(name);
        }
        return;
    }

    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);

    // 遍历所有单元格，重新计算包含公式的单元格
    auto max_row = worksheet.max_row();
    auto max_col = worksheet.max_column();

    for (std::size_t row = 1; row <= max_row; ++row) {
        for (std::size_t col = 1; col <= max_col; ++col) {
            core::Coordinate pos(row, col);
            if (worksheet.has_cell_data(pos)) {
                auto cell_data_obj = worksheet.get_cell_data(pos);
                if (cell_data_obj.formula.has_value() && !cell_data_obj.formula->empty()) {
                    // 重新计算公式并更新单元格值
                    auto calculated_value = calculate_formula(sheet_name, pos);
                    cell_data_obj.value = calculated_value;
                    worksheet.set_cell_data(pos, cell_data_obj);
                }
            }
        }
    }

    mark_worksheet_dirty(sheet_name);
}

} // namespace tinakit::internal
