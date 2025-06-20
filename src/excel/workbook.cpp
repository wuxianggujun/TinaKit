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

workbook::workbook()
    : impl_(std::make_shared<internal::workbook_impl>()) {
}

workbook::workbook(std::shared_ptr<internal::workbook_impl> impl)
    : impl_(std::move(impl)) {
}

// ========================================
// 静态工厂方法
// ========================================

workbook workbook::load(const std::filesystem::path& file_path) {
    auto impl = std::make_shared<internal::workbook_impl>(file_path);
    return workbook(impl);
}

async::Task<workbook> workbook::load_async(const std::filesystem::path& file_path) {
    // 在后台线程中执行加载操作
    co_return load(file_path);
}

workbook workbook::create() {
    auto impl = std::make_shared<internal::workbook_impl>();
    impl->ensure_default_structure();  // 现在安全地创建默认结构
    return workbook(impl);
}

// ========================================
// 工作表访问
// ========================================

worksheet workbook::get_worksheet(const std::string& name) const {
    if (!impl_->has_worksheet(name)) {
        throw std::invalid_argument("Worksheet '" + name + "' does not exist");
    }
    auto sheet_id = impl_->get_sheet_id(name);
    return worksheet(impl_, sheet_id, name);
}

worksheet workbook::get_worksheet(std::size_t index) const {
    auto names = impl_->worksheet_names();
    if (index >= names.size()) {
        throw std::out_of_range("Worksheet index out of range");
    }
    auto sheet_id = impl_->get_sheet_id(names[index]);
    return worksheet(impl_, sheet_id, names[index]);
}

worksheet workbook::active_sheet() const {
    auto names = impl_->worksheet_names();
    if (names.empty()) {
        throw std::runtime_error("No worksheets available");
    }
    auto sheet_id = impl_->get_sheet_id(names[0]);
    return worksheet(impl_, sheet_id, names[0]); // 第一个工作表作为活动工作表
}

worksheet workbook::operator[](const std::string& name) const {
    return get_worksheet(name);
}

worksheet workbook::operator[](std::size_t index) const {
    return get_worksheet(index);
}

// ========================================
// 工作表管理
// ========================================

worksheet workbook::create_worksheet(const std::string& name) {
    return impl_->create_worksheet(name);  // create_worksheet已经返回正确的worksheet对象
}

void workbook::remove_worksheet(const std::string& name) {
    impl_->remove_worksheet(name);
}

void workbook::rename_worksheet(const std::string& old_name, const std::string& new_name) {
    impl_->rename_worksheet(old_name, new_name);
}

std::vector<std::string> workbook::worksheet_names() const {
    return impl_->worksheet_names();
}

std::size_t workbook::worksheet_count() const noexcept {
    return impl_->worksheet_count();
}

bool workbook::has_worksheet(const std::string& name) const {
    return impl_->has_worksheet(name);
}

// ========================================
// 文件操作
// ========================================

void workbook::save(const std::filesystem::path& file_path) {
    if (file_path.empty()) {
        impl_->save();
    } else {
        impl_->save(file_path);
    }
}

async::Task<void> workbook::save_async(const std::filesystem::path& file_path) {
    // TODO: 实现异步保存
    save(file_path);
    co_return;
}

// ========================================
// 属性和状态
// ========================================

const std::filesystem::path& workbook::file_path() const noexcept {
    return impl_->file_path();
}

bool workbook::has_unsaved_changes() const noexcept {
    // TODO: 实现未保存更改检查
    return false;
}

std::size_t workbook::file_size() const {
    // TODO: 实现文件大小获取
    return 0;
}

// ========================================
// 内部访问
// ========================================

std::shared_ptr<internal::workbook_impl> workbook::impl() const {
    return impl_;
}

} // namespace tinakit::excel
