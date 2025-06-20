/**
 * @file workbook_impl.cpp
 * @brief 工作簿内部实现类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/worksheet_impl.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/async.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include <algorithm>
#include <stdexcept>

namespace tinakit::internal {

// ========================================
// 构造函数和析构函数
// ========================================

workbook_impl::workbook_impl() 
    : style_manager_(std::make_shared<excel::StyleManager>()),
      shared_strings_(std::make_shared<excel::SharedStrings>()) {
    create_default_structure();
}

workbook_impl::workbook_impl(const std::filesystem::path& file_path)
    : file_path_(file_path),
      style_manager_(std::make_shared<excel::StyleManager>()),
      shared_strings_(std::make_shared<excel::SharedStrings>()) {
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

void workbook_impl::create_worksheet(const std::string& name) {
    if (has_worksheet(name)) {
        throw std::invalid_argument("Worksheet '" + name + "' already exists");
    }
    
    auto worksheet = std::make_unique<worksheet_impl>(name, *this);
    worksheets_[name] = std::move(worksheet);
    worksheet_order_.push_back(name);
    
    is_dirty_ = true;
}

void workbook_impl::remove_worksheet(const std::string& name) {
    if (!has_worksheet(name)) {
        throw std::invalid_argument("Worksheet '" + name + "' does not exist");
    }
    
    if (worksheet_count() <= 1) {
        throw std::invalid_argument("Cannot remove the last worksheet");
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
        throw std::invalid_argument("Worksheet '" + old_name + "' does not exist");
    }
    
    if (has_worksheet(new_name)) {
        throw std::invalid_argument("Worksheet '" + new_name + "' already exists");
    }
    
    // 移动工作表实现
    auto worksheet = std::move(worksheets_[old_name]);
    worksheet->set_name(new_name);
    worksheets_.erase(old_name);
    worksheets_[new_name] = std::move(worksheet);
    
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

cell_data workbook_impl::get_cell_data(const std::string& sheet_name, const core::Position& pos) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    return worksheet.get_cell_data(pos);
}

void workbook_impl::set_cell_value(const std::string& sheet_name, const core::Position& pos, 
                                  const cell_data::CellValue& value) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    
    cell_data data;
    data.value = value;
    worksheet.set_cell_data(pos, data);
    
    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::set_cell_formula(const std::string& sheet_name, const core::Position& pos, 
                                    const std::string& formula) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    
    auto data = worksheet.get_cell_data(pos);
    data.formula = formula;
    worksheet.set_cell_data(pos, data);
    
    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::set_cell_style(const std::string& sheet_name, const core::Position& pos, 
                                  std::uint32_t style_id) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    
    auto data = worksheet.get_cell_data(pos);
    data.style_id = style_id;
    worksheet.set_cell_data(pos, data);
    
    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::set_range_values(const std::string& sheet_name, const core::range_address& range,
                                    const std::vector<std::vector<cell_data::CellValue>>& values) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    worksheet.set_range_data(range, values);
    mark_worksheet_dirty(sheet_name);
}

void workbook_impl::set_range_style(const std::string& sheet_name, const core::range_address& range,
                                   std::uint32_t style_id) {
    ensure_worksheet_loaded(sheet_name);
    auto& worksheet = get_worksheet_impl(sheet_name);
    
    // 遍历范围内的所有单元格
    for (std::size_t row = range.start.row; row <= range.end.row; ++row) {
        for (std::size_t col = range.start.column; col <= range.end.column; ++col) {
            core::Position pos(row, col);
            auto data = worksheet.get_cell_data(pos);
            data.style_id = style_id;
            worksheet.set_cell_data(pos, data);
        }
    }
    
    mark_worksheet_dirty(sheet_name);
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
        throw std::invalid_argument("Worksheet '" + sheet_name + "' does not exist");
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
        throw std::invalid_argument("Worksheet '" + sheet_name + "' does not exist");
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

        // 标记为未修改
        is_dirty_ = false;

    } catch (const std::exception&) {
        // 如果加载失败，创建默认结构
        create_default_structure();
    }
}

void workbook_impl::create_default_structure() {
    // 创建默认工作表
    create_worksheet("Sheet1");
}

void workbook_impl::parse_workbook_xml() {
    if (!archiver_) return;

    try {
        // 检查是否有 workbook.xml 文件
        if (async::sync_wait(archiver_->has_file("xl/workbook.xml"))) {
            auto xml_data = async::sync_wait(archiver_->read_file("xl/workbook.xml"));
            // TODO: 解析 XML 内容，提取工作表信息
            // 暂时创建默认工作表
            if (worksheets_.empty()) {
                create_worksheet("Sheet1");
            }
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
            // TODO: 解析关系文件，建立工作表与XML文件的映射
        }
    } catch (const std::exception&) {
        // 忽略解析错误
    }
}

void workbook_impl::generate_workbook_xml() {
    // 生成基本的 workbook.xml 内容
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<sheets>)";

    // 添加工作表信息
    std::uint32_t sheet_id = 1;
    for (const auto& sheet_name : worksheet_order_) {
        xml_content += "<sheet name=\"" + sheet_name + "\" sheetId=\"" + std::to_string(sheet_id) + "\" r:id=\"rId" + std::to_string(sheet_id) + "\"/>";
        ++sheet_id;
    }

    xml_content += R"(</sheets>
</workbook>)";

    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("xl/workbook.xml", std::move(xml_bytes)));
}

void workbook_impl::generate_workbook_rels() {
    // 生成基本的 workbook.xml.rels 内容
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">)";

    // 添加工作表关系
    std::uint32_t rel_id = 1;
    for (const auto& sheet_name : worksheet_order_) {
        xml_content += "<Relationship Id=\"rId" + std::to_string(rel_id) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet" + std::to_string(rel_id) + ".xml\"/>";
        ++rel_id;
    }

    xml_content += "</Relationships>";

    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("xl/_rels/workbook.xml.rels", std::move(xml_bytes)));
}

void workbook_impl::save_to_archiver() {
    if (!archiver_) {
        // 创建新的归档器
        auto temp_archiver = core::OpenXmlArchiver::create_in_memory_writer();
        archiver_ = std::make_shared<core::OpenXmlArchiver>(std::move(temp_archiver));
    }

    // 生成并保存工作簿XML
    generate_workbook_xml();
    generate_workbook_rels();

    // 保存所有工作表
    for (auto& [name, worksheet] : worksheets_) {
        if (worksheet->is_dirty()) {
            // TODO: 实现 worksheet 保存
        }
    }

    // TODO: 实现样式和共享字符串保存

    // 保存到文件
    async::sync_wait(archiver_->save_to_file(file_path_.string()));
}

} // namespace tinakit::internal
