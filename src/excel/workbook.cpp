/**
 * @file workbook.cpp
 * @brief Excel 工作簿句柄类实现
 * @author TinaKit Team
 * @date 2025-6-20
 */

#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/core/exceptions.hpp"
#include <stdexcept>

namespace tinakit::excel {

// ========================================
// 构造函数和析构函数
// ========================================

Workbook::Workbook()
    : impl_(std::make_shared<internal::workbook_impl>()) {
}

Workbook::Workbook(std::shared_ptr<internal::workbook_impl> impl)
    : impl_(std::move(impl)) {
}

// ========================================
// 静态工厂方法
// ========================================

Workbook Workbook::load(const std::filesystem::path& file_path) {
    auto impl = std::make_shared<internal::workbook_impl>(file_path);
    return Workbook(impl);
}

async::Task<Workbook> Workbook::load_async(const std::filesystem::path& file_path) {
    // 在后台线程中执行加载操作
    co_return load(file_path);
}

Workbook Workbook::create() {
    auto impl = std::make_shared<internal::workbook_impl>();
    // 不再自动创建默认结构，延迟到真正需要时创建
    return Workbook(impl);
}

// ========================================
// 工作表访问
// ========================================

Worksheet Workbook::get_worksheet(const std::string& name) const {
    if (!impl_->has_worksheet(name)) {
        throw WorksheetNotFoundException(name);
    }
    auto sheet_id = impl_->get_sheet_id(name);
    return Worksheet(impl_, sheet_id, name);
}

Worksheet Workbook::get_worksheet(std::size_t index) const {
    auto names = impl_->worksheet_names();
    if (index >= names.size()) {
        throw std::out_of_range("Worksheet index out of range");
    }
    auto sheet_id = impl_->get_sheet_id(names[index]);
    return Worksheet(impl_, sheet_id, names[index]);
}

Worksheet Workbook::active_sheet() const {
    impl_->ensure_has_worksheet();  // 确保至少有一个工作表
    auto names = impl_->worksheet_names();
    auto sheet_id = impl_->get_sheet_id(names[0]);
    return Worksheet(impl_, sheet_id, names[0]); // 第一个工作表作为活动工作表
}

Worksheet Workbook::operator[](const std::string& name) const {
    return get_worksheet(name);
}

Worksheet Workbook::operator[](std::size_t index) const {
    return get_worksheet(index);
}

// ========================================
// 工作表管理
// ========================================

Worksheet Workbook::create_worksheet(const std::string& name) {
    return impl_->create_worksheet(name);  // create_worksheet已经返回正确的worksheet对象
}

void Workbook::remove_worksheet(const std::string& name) {
    impl_->remove_worksheet(name);
}

void Workbook::rename_worksheet(const std::string& old_name, const std::string& new_name) {
    impl_->rename_worksheet(old_name, new_name);
}

std::vector<std::string> Workbook::worksheet_names() const {
    impl_->ensure_has_worksheet();  // 确保至少有一个工作表
    return impl_->worksheet_names();
}

std::size_t Workbook::worksheet_count() const noexcept {
    // 注意：这里不调用 ensure_has_worksheet()，保持真实的工作表数量
    // 如果用户需要确保有工作表，应该调用 worksheet_names() 或 active_sheet()
    return impl_->worksheet_count();
}

bool Workbook::has_worksheet(const std::string& name) const {
    return impl_->has_worksheet(name);
}

// ========================================
// 公式计算
// ========================================

void Workbook::recalculate_formulas(const std::string& sheet_name) {
    impl_->recalculate_formulas(sheet_name);
}

// ========================================
// 文件操作
// ========================================

void Workbook::save(const std::filesystem::path& file_path) {
    if (file_path.empty()) {
        impl_->save();
    } else {
        impl_->save(file_path);
    }
}

async::Task<void> Workbook::save_async(const std::filesystem::path& file_path) {
    // TODO: 实现异步保存
    save(file_path);
    co_return;
}

// ========================================
// 属性和状态
// ========================================

const std::filesystem::path& Workbook::file_path() const noexcept {
    return impl_->file_path();
}

bool Workbook::has_unsaved_changes() const noexcept {
    // TODO: 实现未保存更改检查
    return false;
}

std::size_t Workbook::file_size() const {
    // TODO: 实现文件大小获取
    return 0;
}

// ========================================
// 内部访问
// ========================================

std::shared_ptr<internal::workbook_impl> Workbook::impl() const {
    return impl_;
}

} // namespace tinakit::excel
