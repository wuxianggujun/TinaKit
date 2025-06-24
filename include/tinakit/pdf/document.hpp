/**
 * @file document.hpp
 * @brief PDF文档处理核心类
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "tinakit/pdf/types.hpp"
#include "tinakit/pdf/style.hpp"
#include "tinakit/pdf/config/font_config.hpp"
#include "tinakit/excel/workbook.hpp"

// 前向声明
namespace tinakit::core {
    class Image;
}

namespace tinakit::pdf {

// 前向声明
namespace internal {
    class pdf_document_impl;
}



// 类型定义在 types.hpp 中
// Point, Rect, Color, Font, PageSize, PageOrientation, PageMargins, DocumentInfo, Table 等

/**
 * @brief PDF文档类
 * 
 * 提供PDF文档的创建、编辑和保存功能，特别优化了与Excel的集成。
 * 
 * @example
 * ```cpp
 * // 创建PDF文档
 * auto pdf = pdf::Document::create();
 * pdf.set_page_size(PageSize::A4);
 * 
 * // 添加页面和内容
 * pdf.add_page();
 * pdf.add_text("Hello, PDF!", {100, 700});
 * 
 * // 从Excel导入数据
 * auto workbook = excel::Workbook::load("data.xlsx");
 * pdf.add_excel_table(workbook.active_sheet().range("A1:E10"));
 * 
 * // 保存文档
 * pdf.save("output.pdf");
 * ```
 */
class Document {
public:
    // ========================================
    // 构造函数和析构函数
    // ========================================
    
    /**
     * @brief 创建新的PDF文档
     * @return PDF文档实例
     */
    static Document create();
    

    
    /**
     * @brief 析构函数
     */
    ~Document();
    
    // 移动语义支持
    Document(Document&& other) noexcept;
    Document& operator=(Document&& other) noexcept;
    
    // 禁用拷贝
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // ========================================
    // 文档设置
    // ========================================
    
    /**
     * @brief 设置页面大小
     * @param size 页面大小
     * @param orientation 页面方向
     */
    Document& set_page_size(PageSize size, PageOrientation orientation = PageOrientation::Portrait);
    
    /**
     * @brief 设置自定义页面大小
     * @param width 页面宽度（点）
     * @param height 页面高度（点）
     */
    Document& set_custom_page_size(double width, double height);
    

    
    /**
     * @brief 设置文档元数据
     * @param info 文档信息
     */
    Document& set_document_info(const DocumentInfo& info);

    /**
     * @brief 设置字体配置
     * @param config 字体配置
     */
    Document& set_font_config(const config::FontConfig& config);

    /**
     * @brief 添加单个字体的特殊配置
     * @param font_config 单个字体配置
     */
    Document& add_individual_font_config(const config::IndividualFontConfig& font_config);

    /**
     * @brief 获取当前字体配置
     */
    const config::FontConfig& get_font_config() const;

    // ========================================
    // 页面管理
    // ========================================
    
    /**
     * @brief 添加新页面
     */
    Document& add_page();
    
    /**
     * @brief 获取当前页面数量
     * @return 页面数量
     */
    std::size_t page_count() const;

    // ========================================
    // 内容添加
    // ========================================
    
    /**
     * @brief 添加文本
     * @param text 文本内容
     * @param position 位置坐标（左下角为原点）
     * @param font 字体设置
     */
    Document& add_text(const std::string& text, const Point& position, const Font& font = {});
    
    /**
     * @brief 添加多行文本
     * @param text 文本内容
     * @param bounds 文本区域
     * @param font 字体设置
     * @param alignment 对齐方式
     */
    Document& add_text_block(const std::string& text, const Rect& bounds,
                            const Font& font = {}, TextAlignment alignment = TextAlignment::Left);
    








    // ========================================
    // 字体管理
    // ========================================

    /**
     * @brief 注册字体（带字体数据）
     * @param font_name 字体名称
     * @param font_data 字体文件数据
     * @param embed_font 是否嵌入字体（默认true）
     * @return 字体资源ID
     */
    std::string register_font(const std::string& font_name,
                             const std::vector<std::uint8_t>& font_data,
                             bool embed_font = true);

    // ========================================
    // 文件操作
    // ========================================

    /**
     * @brief 保存PDF文档
     * @param file_path 保存路径
     */
    void save(const std::filesystem::path& file_path);

    /**
     * @brief 保存到内存缓冲区
     * @return PDF数据
     */
    std::vector<std::uint8_t> save_to_buffer();

private:
    /**
     * @brief 私有构造函数
     * @param impl 实现指针
     */
    explicit Document(std::unique_ptr<internal::pdf_document_impl> impl);
    
    std::unique_ptr<internal::pdf_document_impl> impl_;  ///< PIMPL实现
};

} // namespace tinakit::pdf
