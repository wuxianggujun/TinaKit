/**
 * @file document.cpp
 * @brief PDF文档类实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/document.hpp"
#include "tinakit/internal/pdf_document_impl.hpp"
#include "tinakit/core/exceptions.hpp"
#include "tinakit/core/image.hpp"
#include <stdexcept>

namespace tinakit::pdf {

// ========================================
// 构造函数和析构函数
// ========================================

Document Document::create() {
    auto impl = std::make_unique<internal::pdf_document_impl>();
    return Document(std::move(impl));
}

Document Document::load(const std::filesystem::path& file_path) {
    // 避免未使用参数警告
    (void)file_path;

    // TODO: 实现PDF文件读取功能
    throw std::runtime_error("PDF文件读取功能尚未实现");
}

Document::Document(std::unique_ptr<internal::pdf_document_impl> impl)
    : impl_(std::move(impl)) {
}

Document::~Document() = default;

Document::Document(Document&& other) noexcept
    : impl_(std::move(other.impl_)) {
}

Document& Document::operator=(Document&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

// ========================================
// 文档设置
// ========================================

Document& Document::set_page_size(PageSize size, PageOrientation orientation) {
    impl_->set_page_size(size, orientation);
    return *this;
}

Document& Document::set_custom_page_size(double width, double height) {
    impl_->set_custom_page_size(width, height);
    return *this;
}

Document& Document::set_margins(const PageMargins& margins) {
    impl_->set_margins(margins);
    return *this;
}

Document& Document::set_document_info(const DocumentInfo& info) {
    impl_->set_document_info(info);
    return *this;
}

// ========================================
// 页面管理
// ========================================

Document& Document::add_page() {
    impl_->add_page();
    return *this;
}

std::size_t Document::page_count() const {
    return impl_->page_count();
}

// ========================================
// 内容添加
// ========================================

Document& Document::add_text(const std::string& text, const Point& position, const Font& font) {
    impl_->add_text(text, position, font);
    return *this;
}

Document& Document::add_text_block(const std::string& text, const Rect& bounds,
                                  const Font& font, TextAlignment alignment) {
    impl_->add_text_block(text, bounds, font, alignment);
    return *this;
}

Document& Document::add_table(const Table& table, const Point& position) {
    impl_->add_table(table, position);
    return *this;
}

// ========================================
// 图像功能
// ========================================

Document& Document::add_image(const std::string& image_path, const Point& position,
                             double width, double height) {
    impl_->add_image(image_path, position, width, height);
    return *this;
}

Document& Document::add_image(const tinakit::core::Image& image, const Point& position,
                             double width, double height) {
    impl_->add_image(image, position, width, height);
    return *this;
}

Document& Document::add_image(const std::vector<std::uint8_t>& image_data,
                             int width, int height, int channels,
                             const Point& position,
                             double display_width, double display_height) {
    impl_->add_image(image_data, width, height, channels, position, display_width, display_height);
    return *this;
}

// ========================================
// Excel集成功能
// ========================================

Document& Document::add_excel_table(const excel::Worksheet& sheet,
                                    const std::string& range_address,
                                    const Point& position,
                                    bool preserve_formatting) {
    impl_->add_excel_table(sheet, range_address, position, preserve_formatting);
    return *this;
}

Document& Document::add_excel_range(const excel::Range& range,
                                   const Point& position,
                                   bool preserve_formatting) {
    impl_->add_excel_range(range, position, preserve_formatting);
    return *this;
}

Document& Document::add_excel_sheet(const excel::Worksheet& sheet, bool preserve_formatting) {
    impl_->add_excel_sheet(sheet, preserve_formatting);
    return *this;
}

// ========================================
// 文件操作
// ========================================

void Document::save(const std::filesystem::path& file_path) {
    impl_->save(file_path);
}

std::vector<std::uint8_t> Document::save_to_buffer() {
    return impl_->save_to_buffer();
}

} // namespace tinakit::pdf
