/**
 * @file worksheet_impl.cpp
 * @brief 工作表内部实现类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/internal/worksheet_impl.hpp"
#include "tinakit/internal/coordinate_utils.hpp"
#include "tinakit/excel/range.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/async.hpp"
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/xml_parser.hpp"
#include "tinakit/excel/openxml_namespaces.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <limits>
#include <chrono>

namespace tinakit::internal {

using namespace utils;

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

excel::Range worksheet_impl::get_used_range() const {
    if (cells_.empty()) {
        return excel::Range(); // 返回空范围
    }

    // 找到最小和最大的行列
    std::size_t min_row = std::numeric_limits<std::size_t>::max();
    std::size_t max_row = 0;
    std::size_t min_col = std::numeric_limits<std::size_t>::max();
    std::size_t max_col = 0;

    for (const auto& [pos, data] : cells_) {
        min_row = std::min(min_row, pos.row);
        max_row = std::max(max_row, pos.row);
        min_col = std::min(min_col, pos.column);
        max_col = std::max(max_col, pos.column);
    }

    // 创建范围地址
    core::Coordinate top_left(min_row, min_col);
    core::Coordinate bottom_right(max_row, max_col);
    core::range_address range_addr(top_left, bottom_right);

    // 返回空的Range（因为新的Range需要workbook_impl参数）
    // TODO: 这个方法需要重新设计，因为新的Range需要workbook_impl
    return excel::Range();
}

LoadState worksheet_impl::load_state() const {
    return load_state_;
}

// ========================================
// 单元格数据访问
// ========================================

cell_data worksheet_impl::get_cell_data(const core::Coordinate& pos) {
    ensure_loaded(pos);

    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        return it->second;
    }

    // 返回空的单元格数据
    return cell_data();
}

cell_data worksheet_impl::get_cell_data(const core::Coordinate& pos) const {
    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        return it->second;
    }

    // 返回空的单元格数据
    return cell_data();
}

void worksheet_impl::set_cell_data(const core::Coordinate& pos, const cell_data& data) {
    cells_[pos] = data;
    update_dimensions(pos);
    mark_dirty();
}

bool worksheet_impl::has_cell_data(const core::Coordinate& pos) const {
    return cells_.find(pos) != cells_.end();
}

void worksheet_impl::remove_cell_data(const core::Coordinate& pos) {
    cells_.erase(pos);
    mark_dirty();
}

std::map<core::Coordinate, cell_data> worksheet_impl::get_range_data(const core::range_address& range) {
    ensure_range_loaded(range);
    
    std::map<core::Coordinate, cell_data> result;
    
    for (std::size_t row = range.start.row; row <= range.end.row; ++row) {
        for (std::size_t col = range.start.column; col <= range.end.column; ++col) {
            core::Coordinate pos(row, col);
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
            core::Coordinate pos(range.start.row + row_offset, range.start.column + col_offset);
            
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

void worksheet_impl::ensure_loaded(const core::Coordinate& /* pos */) {
    if (load_state_ == LoadState::NotLoaded) {
        load_from_xml();
    }
    // TODO: 实现按需加载特定位置的数据
}

void worksheet_impl::ensure_range_loaded(const core::range_address& /* range */) {
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
    std::map<core::Coordinate, cell_data> new_cells;
    
    for (const auto& [pos, data] : cells_) {
        if (pos.row >= row) {
            // 移动到新位置
            core::Coordinate new_pos(pos.row + count, pos.column);
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
    std::map<core::Coordinate, cell_data> new_cells;
    for (const auto& [pos, data] : cells_) {
        if (pos.row >= row + count) {
            core::Coordinate new_pos(pos.row - count, pos.column);
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
    std::map<core::Coordinate, cell_data> new_cells;
    
    for (const auto& [pos, data] : cells_) {
        if (pos.column >= column) {
            core::Coordinate new_pos(pos.row, pos.column + count);
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
    std::map<core::Coordinate, cell_data> new_cells;
    for (const auto& [pos, data] : cells_) {
        if (pos.column >= column + count) {
            core::Coordinate new_pos(pos.row, pos.column - count);
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
    try {
        // 确定工作表文件路径
        const auto workbook_names = workbook_.worksheet_names();
        auto it = std::find(workbook_names.begin(), workbook_names.end(), name_);
        std::size_t sheet_index = (it != workbook_names.end()) ?
            std::distance(workbook_names.begin(), it) + 1 : 1;

        std::string file_path = "xl/worksheets/sheet" + std::to_string(sheet_index) + ".xml";

        // 从归档器中读取工作表XML
        auto archiver = workbook_.get_archiver();
        if (archiver && async::sync_wait(archiver->has_file(file_path))) {
            auto xml_data = async::sync_wait(archiver->read_file(file_path));

            // 将字节数据转换为字符串
            std::string xml_content(
                reinterpret_cast<const char*>(xml_data.data()),
                xml_data.size()
            );

            // 解析单元格数据
            parse_cell_data(xml_content);
        } else {
            // 尝试其他可能的文件路径
            std::vector<std::string> possible_paths = {
                "xl/worksheets/sheet1.xml",
                "xl/worksheets/" + name_ + ".xml"
            };

            for (const auto& path : possible_paths) {
                if (archiver && async::sync_wait(archiver->has_file(path))) {
                    auto xml_data = async::sync_wait(archiver->read_file(path));
                    std::string xml_content(
                        reinterpret_cast<const char*>(xml_data.data()),
                        xml_data.size()
                    );
                    parse_cell_data(xml_content);
                    break;
                }
            }
        }

        load_state_ = LoadState::FullyLoaded;
    } catch (const std::exception&) {
        // 如果加载失败，标记为已加载但保持空状态
        load_state_ = LoadState::FullyLoaded;
    }
}

void worksheet_impl::update_dimensions(const core::Coordinate& pos) {
    max_row_ = std::max(max_row_, pos.row);
    max_column_ = std::max(max_column_, pos.column);
}

void worksheet_impl::parse_cell_data(const std::string& xml_content) {
    try {
        // 性能监控：记录开始时间
        auto start_time = std::chrono::high_resolution_clock::now();

        // 使用 XmlParser 解析工作表数据
        std::istringstream xml_stream(xml_content);
        core::XmlParser parser(xml_stream, name_ + ".xml");

        // 启用错误恢复模式，以便在遇到未知属性时继续解析
        parser.set_error_recovery(true);

        // 使用单次遍历解析多种元素类型以提高性能
        const std::string& main_ns = excel::openxml_ns::main;

        std::map<std::pair<std::string, std::string>, std::function<void(core::XmlParser::iterator&)>> handlers = {
            // 单元格数据解析（在主命名空间中）
            {{main_ns, "c"}, [this, &parser](core::XmlParser::iterator& it) {
                parse_single_cell(it, parser);
            }},

            // 单元格数据解析（无命名空间，因为子元素可能有xmlns=""）
            {{"", "c"}, [this, &parser](core::XmlParser::iterator& it) {
                parse_single_cell(it, parser);
            }},

            // 条件格式解析（在主命名空间中）
            {{main_ns, "conditionalFormatting"}, [this, &parser](core::XmlParser::iterator& it) {
                parse_conditional_formatting(it, parser);
            }},

            // 条件格式解析（无命名空间）
            {{"", "conditionalFormatting"}, [this, &parser](core::XmlParser::iterator& it) {
                parse_conditional_formatting(it, parser);
            }},

            // 合并单元格解析（在主命名空间中）
            {{main_ns, "mergeCells"}, [this, &parser](core::XmlParser::iterator& it) {
                parse_merged_cells(it, parser);
            }},

            // 合并单元格解析（无命名空间）
            {{"", "mergeCells"}, [this, &parser](core::XmlParser::iterator& it) {
                parse_merged_cells(it, parser);
            }}
        };

        // 使用优化的单次遍历解析
        parser.parse_multiple_elements_ns(handlers);

        // 检查是否有解析错误
        if (auto error = parser.get_last_error()) {
            std::cerr << "XML解析警告: " << error->message
                     << " at line " << error->line << ", column " << error->column << std::endl;
        }

        // 性能监控：记录结束时间和统计信息
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // 在调试模式下输出性能信息
        #ifdef _DEBUG
        std::cout << "📊 XML解析性能统计 (" << name_ << "):" << std::endl;
        std::cout << "  - 解析时间: " << duration.count() << " μs" << std::endl;
        std::cout << "  - XML大小: " << xml_content.size() << " bytes" << std::endl;
        std::cout << "  - 单元格数量: " << cells_.size() << std::endl;
        std::cout << "  - 条件格式数量: " << conditional_formats_.size() << std::endl;
        std::cout << "  - 合并单元格数量: " << merged_ranges_.size() << std::endl;
        if (duration.count() > 0) {
            std::cout << "  - 处理速度: " << (xml_content.size() * 1000000 / duration.count()) << " bytes/s" << std::endl;
        }
        #endif

    } catch (const std::exception& e) {
        // 解析失败时忽略错误，但记录日志用于调试
        std::cerr << "XML解析异常: " << e.what() << std::endl;
    }
}

void worksheet_impl::parse_single_cell(core::XmlParser::iterator& it, core::XmlParser& parser) {
    // 获取单元格引用（如 A1, B2 等）
    auto cell_ref = it.attribute("r");
    auto cell_type = it.attribute("t");
    auto style_id = it.attribute("s");

    if (cell_ref && !cell_ref->empty()) {
        // 解析单元格位置
        auto pos = core::Coordinate::from_address(*cell_ref);

        // 创建单元格数据
        cell_data data;
        data.style_id = style_id ? std::stoul(*style_id) : 0;

        // 查找单元格值
        std::string cell_value;

        // 在当前单元格元素中查找值
        auto current_it = it;
        ++current_it; // 移动到单元格内容

        while (current_it != parser.end() &&
               !(current_it.is_end_element() && current_it.name() == "c")) {

            if (current_it.is_start_element() && current_it.name() == "v") {
                cell_value = current_it.text_content();
                break;
            } else if (current_it.is_start_element() && current_it.name() == "f") {
                // 处理公式
                data.formula = current_it.text_content();
            } else if (current_it.is_start_element() && current_it.name() == "is") {
                // 处理内联字符串：查找 <is><t>...</t></is> 结构
                auto is_it = current_it;
                ++is_it;
                while (is_it != parser.end() &&
                       !(is_it.is_end_element() && is_it.name() == "is")) {
                    if (is_it.is_start_element() && is_it.name() == "t") {
                        cell_value = is_it.text_content();
                        break;
                    }
                    ++is_it;
                }
                break;
            }
            ++current_it;
        }

        // 根据单元格类型设置值
        if (cell_type && *cell_type == "s") {
            // 共享字符串：需要从共享字符串表中查找实际文本
            if (!cell_value.empty()) {
                try {
                    std::uint32_t index = std::stoul(cell_value);
                    auto shared_strings = this->workbook_.get_shared_strings();
                    if (shared_strings && index < shared_strings->count()) {
                        data.value = shared_strings->get_string(index);
                    } else {
                        // 如果索引无效，使用索引值作为后备
                        data.value = cell_value;
                    }
                } catch (...) {
                    // 如果解析索引失败，使用原始值
                    data.value = cell_value;
                }
            }
        } else if (cell_type && *cell_type == "inlineStr") {
            // 内联字符串
            data.value = cell_value;
        } else if (cell_type && *cell_type == "b") {
            // 布尔值
            data.value = (cell_value == "1");
        } else if (!cell_type || cell_type->empty() || *cell_type == "n") {
            // 数字
            if (!cell_value.empty()) {
                try {
                    if (cell_value.find('.') != std::string::npos) {
                        data.value = std::stod(cell_value);
                    } else {
                        data.value = std::stoi(cell_value);
                    }
                } catch (...) {
                    data.value = cell_value;
                }
            }
        } else {
            // 其他类型作为字符串处理
            data.value = cell_value;
        }

        // 存储单元格数据
        // 只要有值或者有样式ID就存储单元格
        bool has_value = false;
        if (std::holds_alternative<std::string>(data.value)) {
            has_value = !std::get<std::string>(data.value).empty();
        } else {
            has_value = true; // 非字符串值都认为有值
        }

        if (has_value || data.style_id != 0 || data.formula) {
            this->cells_[pos] = data;
            this->update_dimensions(pos);
        }
    }
}

void worksheet_impl::parse_conditional_formatting(core::XmlParser::iterator& it, core::XmlParser& parser) {
    auto sqref = it.attribute("sqref");
    if (sqref && !sqref->empty()) {
        excel::ConditionalFormat format;
        format.range = *sqref;
        format.priority = 1; // 默认优先级

        // 解析条件格式规则
        auto current_it = it;
        ++current_it;

        while (current_it != parser.end() &&
               !(current_it.is_end_element() && current_it.name() == "conditionalFormatting")) {

            if (current_it.is_start_element() && current_it.name() == "cfRule") {
                excel::ConditionalFormatRule rule;

                // 解析规则属性
                auto type_attr = current_it.attribute("type");
                auto operator_attr = current_it.attribute("operator");
                auto dxf_id_attr = current_it.attribute("dxfId");
                auto priority_attr = current_it.attribute("priority");

                if (type_attr) {
                    rule.type = this->string_to_conditional_format_type(*type_attr);
                }
                if (operator_attr) {
                    rule.operator_type = this->string_to_conditional_format_operator(*operator_attr);
                }
                if (dxf_id_attr) {
                    rule.dxf_id = std::stoul(*dxf_id_attr);
                }
                if (priority_attr) {
                    format.priority = std::stoi(*priority_attr);
                }

                // 解析公式
                auto rule_it = current_it;
                ++rule_it;
                while (rule_it != parser.end() &&
                       !(rule_it.is_end_element() && rule_it.name() == "cfRule")) {
                    if (rule_it.is_start_element() && rule_it.name() == "formula") {
                        rule.formulas.push_back(rule_it.text_content());
                    }
                    ++rule_it;
                }

                format.rules.push_back(rule);
            }
            ++current_it;
        }

        if (!format.rules.empty()) {
            this->conditional_formats_.push_back(format);
        }
    }
}

// ========================================
// 合并单元格实现
// ========================================

void worksheet_impl::add_merged_range(const core::range_address& range) {
    // 验证范围有效性
    if (range.start.row > range.end.row || range.start.column > range.end.column) {
        throw std::invalid_argument("Invalid merge range: start position must be before end position");
    }

    if (range.start.row == range.end.row && range.start.column == range.end.column) {
        // 单个单元格不需要合并
        return;
    }

    // 检查是否与现有合并范围冲突
    for (const auto& existing_range : merged_ranges_) {
        if (ranges_overlap(range, existing_range)) {
            throw std::invalid_argument("Merge range conflicts with existing merged range");
        }
    }

    // 添加到合并范围列表
    merged_ranges_.push_back(range);
    mark_dirty();
}

void worksheet_impl::remove_merged_range(const core::range_address& range) {
    auto it = std::find(merged_ranges_.begin(), merged_ranges_.end(), range);
    if (it != merged_ranges_.end()) {
        merged_ranges_.erase(it);
        mark_dirty();
    }
}

const std::vector<core::range_address>& worksheet_impl::get_merged_ranges() const {
    return merged_ranges_;
}

bool worksheet_impl::is_merged_range(const core::range_address& range) const {
    return std::find(merged_ranges_.begin(), merged_ranges_.end(), range) != merged_ranges_.end();
}

bool worksheet_impl::ranges_overlap(const core::range_address& range1, const core::range_address& range2) const {
    // 检查两个范围是否重叠
    return !(range1.end.row < range2.start.row ||
             range1.start.row > range2.end.row ||
             range1.end.column < range2.start.column ||
             range1.start.column > range2.end.column);
}

void worksheet_impl::parse_merged_cells(core::XmlParser::iterator& it, core::XmlParser& parser) {
    std::cout << "🔍 开始解析合并单元格..." << std::endl;

    // 解析mergeCells元素中的mergeCell子元素
    auto current_it = it;
    ++current_it;

    while (current_it != parser.end() &&
           !(current_it.is_end_element() && current_it.name() == "mergeCells")) {

        if (current_it.is_start_element() && current_it.name() == "mergeCell") {
            auto ref_attr = current_it.attribute("ref");
            if (ref_attr && !ref_attr->empty()) {
                std::cout << "  📋 发现合并单元格: " << *ref_attr << std::endl;
                try {
                    // 解析范围字符串（如 "A1:C3"）
                    auto range = internal::utils::CoordinateUtils::string_to_range_address(*ref_attr);
                    merged_ranges_.push_back(range);
                    std::cout << "    ✅ 解析成功" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "    ❌ 解析合并单元格范围失败: " << *ref_attr << " - " << e.what() << std::endl;
                }
            }
        }
        ++current_it;
    }

    std::cout << "🔍 合并单元格解析完成，总数: " << merged_ranges_.size() << std::endl;
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
    std::ostringstream oss;
    core::XmlSerializer serializer(oss, "worksheet.xml");

    // XML声明
    serializer.xml_declaration("1.0", "UTF-8", "yes");

    // 开始worksheet元素（使用命名空间）
    serializer.start_element(excel::openxml_ns::main, "worksheet");

    // 声明命名空间（必须在start_element之后）
    serializer.namespace_declaration(excel::openxml_ns::main, "");
    serializer.namespace_declaration(excel::openxml_ns::rel, excel::openxml_ns::r_prefix);

    // 添加工作表数据 - 只要有单元格数据就生成
    if (!cells_.empty()) {
        serializer.start_element("sheetData");

        // 按行组织单元格数据
        std::map<std::size_t, std::vector<std::pair<std::size_t, cell_data>>> rows_data;
        for (const auto& [pos, data] : cells_) {
            rows_data[pos.row].emplace_back(pos.column, data);
        }

        // 生成行数据
        for (auto& [row_num, row_cells] : rows_data) {
            serializer.start_element("row");
            serializer.attribute("r", std::to_string(row_num));

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

                serializer.start_element("c");
                serializer.attribute("r", cell_ref);

                if (cell_data.style_id != 0) {
                    serializer.attribute("s", std::to_string(cell_data.style_id));
                }

                // 根据值类型添加类型属性和内容
                // 为了避免lambda捕获问题，先获取需要的对象
                auto shared_strings = workbook_.get_shared_strings();

                std::visit([&serializer, shared_strings, this](const auto& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        if (!value.empty()) {
                            // 智能选择字符串存储方式
                            if (this->should_use_inline_string(value)) {
                                // 使用内联字符串格式
                                serializer.attribute("t", "inlineStr");
                                serializer.start_element("is");
                                serializer.element("t", value);
                                serializer.end_element(); // is
                            } else {
                                // 使用共享字符串格式
                                if (shared_strings) {
                                    std::uint32_t index = shared_strings->add_string(value);
                                    serializer.attribute("t", "s");
                                    serializer.element("v", std::to_string(index));
                                } else {
                                    // 回退到内联字符串
                                    serializer.attribute("t", "inlineStr");
                                    serializer.start_element("is");
                                    serializer.element("t", value);
                                    serializer.end_element(); // is
                                }
                            }
                        }
                        // 空字符串不需要特殊处理
                    } else if constexpr (std::is_same_v<T, double>) {
                        // 数字类型：使用数值格式
                        serializer.attribute("t", "n");
                        serializer.element("v", std::to_string(value));
                    } else if constexpr (std::is_same_v<T, int>) {
                        // 整数类型：使用数值格式
                        serializer.attribute("t", "n");
                        serializer.element("v", std::to_string(value));
                    } else if constexpr (std::is_same_v<T, bool>) {
                        // 布尔类型：使用布尔格式
                        serializer.attribute("t", "b");
                        serializer.element("v", value ? "1" : "0");
                    }
                }, cell_data.value);

                // 添加公式（如果有）
                if (cell_data.formula) {
                    serializer.element("f", *cell_data.formula);
                }

                serializer.end_element(); // c
            }

            serializer.end_element(); // row
        }

        serializer.end_element(); // sheetData
    }

    // 添加条件格式
    if (!conditional_formats_.empty()) {
        for (const auto& format : conditional_formats_) {
            serializer.start_element("conditionalFormatting");
            serializer.attribute("sqref", format.range);

            for (const auto& rule : format.rules) {
                serializer.start_element("cfRule");
                serializer.attribute("type", conditional_format_type_to_string(rule.type));
                serializer.attribute("operator", conditional_format_operator_to_string(rule.operator_type));
                if (rule.dxf_id.has_value()) {
                    serializer.attribute("dxfId", std::to_string(rule.dxf_id.value()));
                }
                serializer.attribute("priority", std::to_string(format.priority));

                // 添加公式
                for (const auto& formula : rule.formulas) {
                    serializer.element("formula", formula);
                }

                serializer.end_element(); // cfRule
            }
            serializer.end_element(); // conditionalFormatting
        }
    }

    // 添加合并单元格
    if (!merged_ranges_.empty()) {
        std::cout << "📋 生成合并单元格XML，数量: " << merged_ranges_.size() << std::endl;

        serializer.start_element("mergeCells");
        serializer.attribute("count", std::to_string(merged_ranges_.size()));

        for (const auto& range : merged_ranges_) {
            serializer.start_element("mergeCell");

            // 生成范围字符串（如 "A1:C3"）
            std::string range_str = internal::utils::CoordinateUtils::range_address_to_string(range);
            std::cout << "  - 合并范围: " << range_str << std::endl;
            serializer.attribute("ref", range_str);

            serializer.end_element(); // mergeCell
        }

        serializer.end_element(); // mergeCells
    }

    serializer.end_element(); // worksheet

    // 获取生成的XML内容
    std::string xml_content = oss.str();

    // 调试输出：打印生成的工作表XML
    std::cout << "\n=== 生成的工作表XML (" << name_ << ") ===" << std::endl;
    std::cout << xml_content << std::endl;
    std::cout << "=== 工作表XML结束 ===" << std::endl;

    return xml_content;
}

bool worksheet_impl::should_use_inline_string(const std::string& str) const {
    try {
        // Excel的高效策略：简单规则，快速判断

        // 1. 空字符串使用内联
        if (str.empty()) {
            return true;
        }

        // 2. 很短的字符串（≤3字符）使用内联
        if (str.length() <= 3) {
            return true;
        }

        // 3. 很长的字符串（>50字符）使用内联（避免共享字符串表过大）
        if (str.length() > 50) {
            return true;
        }

        // 4. 简化的数字检测：只检查前几个字符
        std::size_t check_length = std::min(str.length(), static_cast<std::size_t>(10));
        std::size_t digit_count = 0;

        for (std::size_t i = 0; i < check_length; ++i) {
            if (std::isdigit(static_cast<unsigned char>(str[i]))) {
                digit_count++;
            }
        }

        // 如果前面大部分是数字，使用内联
        if (digit_count * 2 > check_length) {
            return true;
        }

        // 5. 其他情况使用共享字符串（Excel的默认行为）
        return false;

    } catch (...) {
        // 如果出现任何异常，默认使用内联字符串（更安全）
        return true;
    }
}

// ========================================
// 条件格式实现
// ========================================

void worksheet_impl::add_conditional_format(const excel::ConditionalFormat& format) {
    conditional_formats_.push_back(format);
    mark_dirty();
}

const std::vector<excel::ConditionalFormat>& worksheet_impl::get_conditional_formats() const {
    return conditional_formats_;
}

std::optional<std::uint32_t> worksheet_impl::apply_conditional_format(const core::Coordinate& pos) {
    // 遍历所有条件格式，按优先级排序
    std::vector<std::pair<int, const excel::ConditionalFormat*>> sorted_formats;
    for (const auto& format : conditional_formats_) {
        sorted_formats.emplace_back(format.priority, &format);
    }

    // 按优先级排序（数字越小优先级越高）
    std::sort(sorted_formats.begin(), sorted_formats.end());

    // 检查每个条件格式是否适用于当前单元格
    for (const auto& [priority, format] : sorted_formats) {
        if (is_cell_in_range(pos, format->range)) {
            for (const auto& rule : format->rules) {
                if (evaluate_conditional_rule(pos, rule)) {
                    // 找到匹配的规则，返回对应的dxf_id
                    if (rule.dxf_id.has_value()) {
                        return rule.dxf_id.value();
                    }
                }
            }
        }
    }

    return std::nullopt; // 没有匹配的条件格式
}

bool worksheet_impl::is_cell_in_range(const core::Coordinate& pos, const std::string& range_str) const {
    try {
        auto range_addr = utils::CoordinateUtils::string_to_range_address(range_str);
        return pos.row >= range_addr.start.row && pos.row <= range_addr.end.row &&
               pos.column >= range_addr.start.column && pos.column <= range_addr.end.column;
    } catch (...) {
        return false;
    }
}

bool worksheet_impl::evaluate_conditional_rule(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const {
    // 获取单元格的值
    auto cell_value = get_cell_value_for_condition(pos);

    switch (rule.type) {
        case excel::ConditionalFormatType::CellValue:
            return evaluate_cell_value_condition(cell_value, rule);

        case excel::ConditionalFormatType::ContainsText:
            return evaluate_text_condition(cell_value, rule);

        case excel::ConditionalFormatType::Expression:
            return evaluate_expression_condition(pos, rule);

        case excel::ConditionalFormatType::DuplicateValues:
            return evaluate_duplicate_values_condition(pos, rule);

        case excel::ConditionalFormatType::UniqueValues:
            return evaluate_unique_values_condition(pos, rule);

        default:
            return false;
    }
}

double worksheet_impl::get_cell_value_for_condition(const core::Coordinate& pos) const {
    if (has_cell_data(pos)) {
        auto cell_data = get_cell_data(pos);

        if (std::holds_alternative<double>(cell_data.value)) {
            return std::get<double>(cell_data.value);
        } else if (std::holds_alternative<int>(cell_data.value)) {
            return static_cast<double>(std::get<int>(cell_data.value));
        } else if (std::holds_alternative<std::string>(cell_data.value)) {
            try {
                return std::stod(std::get<std::string>(cell_data.value));
            } catch (...) {
                return 0.0;
            }
        } else if (std::holds_alternative<bool>(cell_data.value)) {
            return std::get<bool>(cell_data.value) ? 1.0 : 0.0;
        }
    }
    return 0.0;
}

std::string worksheet_impl::get_cell_text_for_condition(const core::Coordinate& pos) const {
    if (has_cell_data(pos)) {
        auto cell_data = get_cell_data(pos);

        if (std::holds_alternative<std::string>(cell_data.value)) {
            return std::get<std::string>(cell_data.value);
        } else if (std::holds_alternative<double>(cell_data.value)) {
            return std::to_string(std::get<double>(cell_data.value));
        } else if (std::holds_alternative<int>(cell_data.value)) {
            return std::to_string(std::get<int>(cell_data.value));
        } else if (std::holds_alternative<bool>(cell_data.value)) {
            return std::get<bool>(cell_data.value) ? "TRUE" : "FALSE";
        }
    }
    return "";
}

bool worksheet_impl::evaluate_cell_value_condition(double cell_value, const excel::ConditionalFormatRule& rule) const {
    if (rule.formulas.empty()) return false;

    double condition_value;
    try {
        condition_value = std::stod(rule.formulas[0]);
    } catch (...) {
        return false;
    }

    switch (rule.operator_type) {
        case excel::ConditionalFormatOperator::GreaterThan:
            return cell_value > condition_value;
        case excel::ConditionalFormatOperator::GreaterThanOrEqual:
            return cell_value >= condition_value;
        case excel::ConditionalFormatOperator::LessThan:
            return cell_value < condition_value;
        case excel::ConditionalFormatOperator::LessThanOrEqual:
            return cell_value <= condition_value;
        case excel::ConditionalFormatOperator::Equal:
            return std::abs(cell_value - condition_value) < 1e-9;
        case excel::ConditionalFormatOperator::NotEqual:
            return std::abs(cell_value - condition_value) >= 1e-9;
        case excel::ConditionalFormatOperator::Between:
            if (rule.formulas.size() >= 2) {
                try {
                    double min_val = std::stod(rule.formulas[0]);
                    double max_val = std::stod(rule.formulas[1]);
                    return cell_value >= min_val && cell_value <= max_val;
                } catch (...) {
                    return false;
                }
            }
            return false;
        case excel::ConditionalFormatOperator::NotBetween:
            if (rule.formulas.size() >= 2) {
                try {
                    double min_val = std::stod(rule.formulas[0]);
                    double max_val = std::stod(rule.formulas[1]);
                    return cell_value < min_val || cell_value > max_val;
                } catch (...) {
                    return false;
                }
            }
            return false;
        default:
            return false;
    }
}

bool worksheet_impl::evaluate_text_condition(double cell_value, const excel::ConditionalFormatRule& rule) const {
    // 对于文本条件，我们需要获取单元格的文本表示
    // 这里简化处理，实际应该根据单元格位置获取文本
    return false; // TODO: 实现文本条件评估
}

bool worksheet_impl::evaluate_expression_condition(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const {
    // TODO: 实现表达式条件评估（需要公式引擎支持）
    return false;
}

bool worksheet_impl::evaluate_duplicate_values_condition(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const {
    // TODO: 实现重复值条件评估
    return false;
}

bool worksheet_impl::evaluate_unique_values_condition(const core::Coordinate& pos, const excel::ConditionalFormatRule& rule) const {
    // TODO: 实现唯一值条件评估
    return false;
}

std::string worksheet_impl::conditional_format_type_to_string(excel::ConditionalFormatType type) const {
    switch (type) {
        case excel::ConditionalFormatType::CellValue: return "cellIs";
        case excel::ConditionalFormatType::Expression: return "expression";
        case excel::ConditionalFormatType::ContainsText: return "containsText";
        case excel::ConditionalFormatType::NotContainsText: return "notContainsText";
        case excel::ConditionalFormatType::BeginsWith: return "beginsWith";
        case excel::ConditionalFormatType::EndsWith: return "endsWith";
        case excel::ConditionalFormatType::DuplicateValues: return "duplicateValues";
        case excel::ConditionalFormatType::UniqueValues: return "uniqueValues";
        default: return "cellIs";
    }
}

std::string worksheet_impl::conditional_format_operator_to_string(excel::ConditionalFormatOperator op) const {
    switch (op) {
        case excel::ConditionalFormatOperator::GreaterThan: return "greaterThan";
        case excel::ConditionalFormatOperator::GreaterThanOrEqual: return "greaterThanOrEqual";
        case excel::ConditionalFormatOperator::LessThan: return "lessThan";
        case excel::ConditionalFormatOperator::LessThanOrEqual: return "lessThanOrEqual";
        case excel::ConditionalFormatOperator::Equal: return "equal";
        case excel::ConditionalFormatOperator::NotEqual: return "notEqual";
        case excel::ConditionalFormatOperator::Between: return "between";
        case excel::ConditionalFormatOperator::NotBetween: return "notBetween";
        case excel::ConditionalFormatOperator::ContainsText: return "containsText";
        case excel::ConditionalFormatOperator::NotContains: return "notContains";
        case excel::ConditionalFormatOperator::BeginsWith: return "beginsWith";
        case excel::ConditionalFormatOperator::EndsWith: return "endsWith";
        default: return "equal";
    }
}

excel::ConditionalFormatType worksheet_impl::string_to_conditional_format_type(const std::string& str) const {
    if (str == "cellIs") return excel::ConditionalFormatType::CellValue;
    if (str == "expression") return excel::ConditionalFormatType::Expression;
    if (str == "containsText") return excel::ConditionalFormatType::ContainsText;
    if (str == "notContainsText") return excel::ConditionalFormatType::NotContainsText;
    if (str == "beginsWith") return excel::ConditionalFormatType::BeginsWith;
    if (str == "endsWith") return excel::ConditionalFormatType::EndsWith;
    if (str == "duplicateValues") return excel::ConditionalFormatType::DuplicateValues;
    if (str == "uniqueValues") return excel::ConditionalFormatType::UniqueValues;
    return excel::ConditionalFormatType::CellValue; // 默认值
}

excel::ConditionalFormatOperator worksheet_impl::string_to_conditional_format_operator(const std::string& str) const {
    if (str == "greaterThan") return excel::ConditionalFormatOperator::GreaterThan;
    if (str == "greaterThanOrEqual") return excel::ConditionalFormatOperator::GreaterThanOrEqual;
    if (str == "lessThan") return excel::ConditionalFormatOperator::LessThan;
    if (str == "lessThanOrEqual") return excel::ConditionalFormatOperator::LessThanOrEqual;
    if (str == "equal") return excel::ConditionalFormatOperator::Equal;
    if (str == "notEqual") return excel::ConditionalFormatOperator::NotEqual;
    if (str == "between") return excel::ConditionalFormatOperator::Between;
    if (str == "notBetween") return excel::ConditionalFormatOperator::NotBetween;
    if (str == "containsText") return excel::ConditionalFormatOperator::ContainsText;
    if (str == "notContains") return excel::ConditionalFormatOperator::NotContains;
    if (str == "beginsWith") return excel::ConditionalFormatOperator::BeginsWith;
    if (str == "endsWith") return excel::ConditionalFormatOperator::EndsWith;
    return excel::ConditionalFormatOperator::Equal; // 默认值
}

} // namespace tinakit::internal
