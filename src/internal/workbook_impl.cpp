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
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/async.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/xml_parser.hpp"
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
      shared_strings_(std::make_shared<excel::SharedStrings>()) {
    // 不在构造函数中创建默认结构，延迟到需要时创建
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

void workbook_impl::ensure_default_structure() {
    if (worksheets_.empty()) {
        create_default_structure();
    }
}

excel::Worksheet workbook_impl::create_worksheet(const std::string& name) {
    if (has_worksheet(name)) {
        throw std::invalid_argument("Worksheet '" + name + "' already exists");
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

cell_data workbook_impl::get_cell_data(std::uint32_t sheet_id, const core::Position& pos) {
    std::string sheet_name = get_sheet_name(sheet_id);
    return get_cell_data(sheet_name, pos);
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

void workbook_impl::set_cell_value(std::uint32_t sheet_id, const core::Position& pos,
                                  const cell_data::CellValue& value) {
    std::string sheet_name = get_sheet_name(sheet_id);
    set_cell_value(sheet_name, pos, value);
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

void workbook_impl::set_cell_formula(std::uint32_t sheet_id, const core::Position& pos,
                                    const std::string& formula) {
    std::string sheet_name = get_sheet_name(sheet_id);
    set_cell_formula(sheet_name, pos, formula);
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

void workbook_impl::set_cell_style(std::uint32_t sheet_id, const core::Position& pos,
                                  std::uint32_t style_id) {
    std::string sheet_name = get_sheet_name(sheet_id);
    set_cell_style(sheet_name, pos, style_id);
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
                    r_id = it.attribute("http://schemas.openxmlformats.org/officeDocument/2006/relationships#id");
                }

                if (name && !name->empty()) {
                    // 创建工作表（如果不存在）
                    if (!has_worksheet(*name)) {
                        create_worksheet(*name);
                    }
                }
            });

            // 如果没有找到工作表，创建默认工作表
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

            std::cout << "✅ 样式文件加载成功，样式数量: " << style_manager_->cell_style_count() << std::endl;
        } else {
            std::cout << "⚠️  未找到样式文件，使用默认样式" << std::endl;
            // 如果没有样式文件，确保有默认样式
            style_manager_->initialize_defaults();
        }
    } catch (const std::exception& e) {
        std::cout << "❌ 加载样式文件失败: " << e.what() << "，使用默认样式" << std::endl;
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

            std::cout << "✅ 共享字符串文件加载成功，字符串数量: " << shared_strings_->count() << std::endl;
        } else {
            std::cout << "⚠️  未找到共享字符串文件" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ 加载共享字符串文件失败: " << e.what() << std::endl;
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

    // 添加样式关系（关键！）
    xml_content += "<Relationship Id=\"rId" + std::to_string(rel_id) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>";
    ++rel_id;

    // 添加共享字符串关系
    xml_content += "<Relationship Id=\"rId" + std::to_string(rel_id) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings\" Target=\"sharedStrings.xml\"/>";

    xml_content += "</Relationships>";

    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("xl/_rels/workbook.xml.rels", std::move(xml_bytes)));
}

void workbook_impl::generate_content_types() {
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>
<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>)";

    // 添加工作表内容类型
    std::uint32_t sheet_id = 1;
    for (const auto& sheet_name : worksheet_order_) {
        xml_content += "<Override PartName=\"/xl/worksheets/sheet" + std::to_string(sheet_id) + ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>";
        ++sheet_id;
    }

    xml_content += "</Types>";

    // 保存到归档器
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }
    async::sync_wait(archiver_->add_file("[Content_Types].xml", std::move(xml_bytes)));
}

void workbook_impl::generate_main_rels() {
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>)";

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

    // 调试输出：打印生成的样式XML
    std::cout << "\n=== 生成的样式XML (styles.xml) ===" << std::endl;
    std::cout << xml_content << std::endl;
    std::cout << "=== 样式XML结束 ===" << std::endl;

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

    // 调试输出
    if (shared_strings_) {
        std::cout << "共享字符串管理器存在，字符串数量: " << shared_strings_->count() << std::endl;
    } else {
        std::cout << "共享字符串管理器不存在！" << std::endl;
    }

    if (shared_strings_ && shared_strings_->count() > 0) {
        std::cout << "生成包含 " << shared_strings_->count() << " 个字符串的共享字符串XML" << std::endl;
        xml_content = shared_strings_->generate_xml();
    } else {
        std::cout << "生成空的共享字符串XML" << std::endl;
        // 如果没有共享字符串，生成空的共享字符串表
        xml_content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="0" uniqueCount="0">
</sst>)";
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



std::optional<cell_data> workbook_impl::get_cell_data(const core::Position& pos) {
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

    auto it = worksheets_.find(sheet_name);
    if (it != worksheets_.end()) {
        // 这里应该将条件格式添加到工作表中
        // 暂时只标记为已修改
        it->second->mark_dirty();
        is_dirty_ = true;
    }
}



worksheet_impl& workbook_impl::get_worksheet_impl_public(const std::string& sheet_name) {
    ensure_worksheet_loaded(sheet_name);
    return get_worksheet_impl(sheet_name);
}

} // namespace tinakit::internal
