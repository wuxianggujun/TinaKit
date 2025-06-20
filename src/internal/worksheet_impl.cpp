/**
 * @file worksheet_impl.cpp
 * @brief 工作表内部实现类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/internal/worksheet_impl.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/async.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include <algorithm>

namespace tinakit::internal {

// ========================================
// 构造函数和析构函数
// ========================================

worksheet_impl::worksheet_impl(const std::string& name, workbook_impl& workbook)
    : name_(name), workbook_(workbook) {
}

worksheet_impl::~worksheet_impl() = default;

// ========================================
// 基本属性
// ========================================

const std::string& worksheet_impl::name() const {
    return name_;
}

void worksheet_impl::set_name(const std::string& new_name) {
    name_ = new_name;
    mark_dirty();
}

std::size_t worksheet_impl::max_row() const {
    return max_row_;
}

std::size_t worksheet_impl::max_column() const {
    return max_column_;
}

LoadState worksheet_impl::load_state() const {
    return load_state_;
}

// ========================================
// 单元格数据访问
// ========================================

cell_data worksheet_impl::get_cell_data(const core::Position& pos) {
    ensure_loaded(pos);
    
    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        return it->second;
    }
    
    // 返回空的单元格数据
    return cell_data();
}

void worksheet_impl::set_cell_data(const core::Position& pos, const cell_data& data) {
    cells_[pos] = data;
    update_dimensions(pos);
    mark_dirty();
}

bool worksheet_impl::has_cell_data(const core::Position& pos) const {
    return cells_.find(pos) != cells_.end();
}

void worksheet_impl::remove_cell_data(const core::Position& pos) {
    cells_.erase(pos);
    mark_dirty();
}

std::map<core::Position, cell_data> worksheet_impl::get_range_data(const core::range_address& range) {
    ensure_range_loaded(range);
    
    std::map<core::Position, cell_data> result;
    
    for (std::size_t row = range.start.row; row <= range.end.row; ++row) {
        for (std::size_t col = range.start.column; col <= range.end.column; ++col) {
            core::Position pos(row, col);
            auto it = cells_.find(pos);
            if (it != cells_.end()) {
                result[pos] = it->second;
            }
        }
    }
    
    return result;
}

void worksheet_impl::set_range_data(const core::range_address& range,
                                   const std::vector<std::vector<cell_data::CellValue>>& values) {
    std::size_t row_offset = 0;
    for (const auto& row_values : values) {
        std::size_t col_offset = 0;
        for (const auto& value : row_values) {
            core::Position pos(range.start.row + row_offset, range.start.column + col_offset);
            
            // 检查是否超出范围
            if (pos.row > range.end.row || pos.column > range.end.column) {
                break;
            }
            
            cell_data data(value);
            set_cell_data(pos, data);
            
            ++col_offset;
        }
        ++row_offset;
        
        // 检查是否超出范围
        if (range.start.row + row_offset > range.end.row) {
            break;
        }
    }
}

// ========================================
// 惰性加载管理
// ========================================

void worksheet_impl::ensure_loaded(const core::Position& pos) {
    if (load_state_ == LoadState::NotLoaded) {
        load_from_xml();
    }
    // TODO: 实现按需加载特定位置的数据
}

void worksheet_impl::ensure_range_loaded(const core::range_address& range) {
    if (load_state_ == LoadState::NotLoaded) {
        load_from_xml();
    }
    // TODO: 实现按需加载特定范围的数据
}

void worksheet_impl::load_all() {
    if (load_state_ != LoadState::FullyLoaded) {
        load_from_xml();
        load_state_ = LoadState::FullyLoaded;
    }
}

void worksheet_impl::unload() {
    cells_.clear();
    column_widths_.clear();
    row_heights_.clear();
    merged_ranges_.clear();
    max_row_ = 0;
    max_column_ = 0;
    load_state_ = LoadState::NotLoaded;
}

// ========================================
// 工作表级别操作
// ========================================

void worksheet_impl::insert_rows(std::size_t row, std::size_t count) {
    // 创建新的单元格映射
    std::map<core::Position, cell_data> new_cells;
    
    for (const auto& [pos, data] : cells_) {
        if (pos.row >= row) {
            // 移动到新位置
            core::Position new_pos(pos.row + count, pos.column);
            new_cells[new_pos] = data;
        } else {
            // 保持原位置
            new_cells[pos] = data;
        }
    }
    
    cells_ = std::move(new_cells);
    max_row_ += count;
    mark_dirty();
}

void worksheet_impl::delete_rows(std::size_t row, std::size_t count) {
    // 删除指定范围内的行
    auto it = cells_.begin();
    while (it != cells_.end()) {
        if (it->first.row >= row && it->first.row < row + count) {
            it = cells_.erase(it);
        } else {
            ++it;
        }
    }
    
    // 移动后续行
    std::map<core::Position, cell_data> new_cells;
    for (const auto& [pos, data] : cells_) {
        if (pos.row >= row + count) {
            core::Position new_pos(pos.row - count, pos.column);
            new_cells[new_pos] = data;
        } else {
            new_cells[pos] = data;
        }
    }
    
    cells_ = std::move(new_cells);
    max_row_ = std::max(0, static_cast<int>(max_row_) - static_cast<int>(count));
    mark_dirty();
}

void worksheet_impl::insert_columns(std::size_t column, std::size_t count) {
    std::map<core::Position, cell_data> new_cells;
    
    for (const auto& [pos, data] : cells_) {
        if (pos.column >= column) {
            core::Position new_pos(pos.row, pos.column + count);
            new_cells[new_pos] = data;
        } else {
            new_cells[pos] = data;
        }
    }
    
    cells_ = std::move(new_cells);
    max_column_ += count;
    mark_dirty();
}

void worksheet_impl::delete_columns(std::size_t column, std::size_t count) {
    // 删除指定范围内的列
    auto it = cells_.begin();
    while (it != cells_.end()) {
        if (it->first.column >= column && it->first.column < column + count) {
            it = cells_.erase(it);
        } else {
            ++it;
        }
    }
    
    // 移动后续列
    std::map<core::Position, cell_data> new_cells;
    for (const auto& [pos, data] : cells_) {
        if (pos.column >= column + count) {
            core::Position new_pos(pos.row, pos.column - count);
            new_cells[new_pos] = data;
        } else {
            new_cells[pos] = data;
        }
    }
    
    cells_ = std::move(new_cells);
    max_column_ = std::max(0, static_cast<int>(max_column_) - static_cast<int>(count));
    mark_dirty();
}

void worksheet_impl::set_column_width(std::size_t column, double width) {
    column_widths_[column] = width;
    mark_dirty();
}

double worksheet_impl::get_column_width(std::size_t column) const {
    auto it = column_widths_.find(column);
    return (it != column_widths_.end()) ? it->second : 8.43; // Excel 默认列宽
}

void worksheet_impl::set_row_height(std::size_t row, double height) {
    row_heights_[row] = height;
    mark_dirty();
}

double worksheet_impl::get_row_height(std::size_t row) const {
    auto it = row_heights_.find(row);
    return (it != row_heights_.end()) ? it->second : 15.0; // Excel 默认行高
}

// ========================================
// 内部状态管理
// ========================================

void worksheet_impl::mark_dirty() {
    is_dirty_ = true;
}

bool worksheet_impl::is_dirty() const {
    return is_dirty_;
}

void worksheet_impl::clear_dirty() {
    is_dirty_ = false;
}

// ========================================
// 私有方法
// ========================================

void worksheet_impl::load_from_xml() {
    // TODO: 实现从 XML 加载数据的逻辑
    load_state_ = LoadState::FullyLoaded;
}

void worksheet_impl::update_dimensions(const core::Position& pos) {
    max_row_ = std::max(max_row_, pos.row);
    max_column_ = std::max(max_column_, pos.column);
}

void worksheet_impl::parse_cell_data(const std::string& xml_content) {
    // TODO: 实现 XML 解析逻辑
}

void worksheet_impl::save_to_archiver(core::OpenXmlArchiver& archiver) {
    // 生成工作表XML并保存到归档器
    auto xml_content = generate_worksheet_xml();

    // 确定文件路径（基于工作表在工作簿中的位置）
    auto workbook_names = workbook_.worksheet_names();
    auto it = std::find(workbook_names.begin(), workbook_names.end(), name_);
    std::size_t sheet_index = (it != workbook_names.end()) ?
        std::distance(workbook_names.begin(), it) + 1 : 1;

    std::string file_path = "xl/worksheets/sheet" + std::to_string(sheet_index) + ".xml";

    // 转换为字节数组
    std::vector<std::byte> xml_bytes;
    for (char c : xml_content) {
        xml_bytes.push_back(static_cast<std::byte>(c));
    }

    // 保存到归档器
    async::sync_wait(archiver.add_file(file_path, std::move(xml_bytes)));
}

std::string worksheet_impl::generate_worksheet_xml() {
    std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">)";

    // 添加工作表数据
    if (max_row_ > 0 && max_column_ > 0) {
        xml_content += "<sheetData>";

        // 按行组织单元格数据
        std::map<std::size_t, std::vector<std::pair<std::size_t, cell_data>>> rows_data;
        for (const auto& [pos, data] : cells_) {
            rows_data[pos.row].emplace_back(pos.column, data);
        }

        // 生成行数据
        for (auto& [row_num, row_cells] : rows_data) {
            xml_content += "<row r=\"" + std::to_string(row_num) + "\">";

            // 排序列（按列号排序）
            std::sort(row_cells.begin(), row_cells.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });

            // 生成单元格数据
            for (const auto& [col_num, cell_data] : row_cells) {
                // 简单的列名转换（A, B, C...）
                std::string col_name;
                std::size_t temp_col = col_num;
                while (temp_col > 0) {
                    col_name = char('A' + (temp_col - 1) % 26) + col_name;
                    temp_col = (temp_col - 1) / 26;
                }
                std::string cell_ref = col_name + std::to_string(row_num);
                xml_content += "<c r=\"" + cell_ref + "\"";

                if (cell_data.style_id != 0) {
                    xml_content += " s=\"" + std::to_string(cell_data.style_id) + "\"";
                }

                // 添加值
                xml_content += ">";
                if (cell_data.formula) {
                    xml_content += "<f>" + *cell_data.formula + "</f>";
                }

                // 根据值类型添加内容
                std::visit([&xml_content](const auto& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        if (!value.empty()) {
                            xml_content += "<v>" + value + "</v>";
                        }
                    } else if constexpr (std::is_same_v<T, double>) {
                        xml_content += "<v>" + std::to_string(value) + "</v>";
                    } else if constexpr (std::is_same_v<T, int>) {
                        xml_content += "<v>" + std::to_string(value) + "</v>";
                    } else if constexpr (std::is_same_v<T, bool>) {
                        xml_content += "<v>" + std::string(value ? "1" : "0") + "</v>";
                    }
                }, cell_data.value);

                xml_content += "</c>";
            }

            xml_content += "</row>";
        }

        xml_content += "</sheetData>";
    }

    xml_content += "</worksheet>";
    return xml_content;
}

} // namespace tinakit::internal
