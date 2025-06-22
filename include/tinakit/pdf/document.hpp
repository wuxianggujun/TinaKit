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
     * @brief 从现有PDF文件加载（如果支持）
     * @param file_path PDF文件路径
     * @return PDF文档实例
     * @note 当前版本主要支持创建，读取功能在后续版本实现
     */
    static Document load(const std::filesystem::path& file_path);
    
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
     * @brief 设置页面边距
     * @param margins 边距设置
     */
    Document& set_margins(const PageMargins& margins);
    
    /**
     * @brief 设置文档元数据
     * @param info 文档信息
     */
    Document& set_document_info(const DocumentInfo& info);

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
    
    /**
     * @brief 添加表格
     * @param table 表格数据
     * @param position 表格位置
     */
    Document& add_table(const Table& table, const Point& position);

    // ========================================
    // 图像功能
    // ========================================

    /**
     * @brief 从文件添加图像
     * @param image_path 图像文件路径
     * @param position 图像位置（左下角）
     * @param width 图像宽度（0表示使用原始宽度）
     * @param height 图像高度（0表示使用原始高度或按比例缩放）
     * @return 文档引用（支持链式调用）
     *
     * 支持的格式：JPEG, PNG, BMP, TGA, PSD, GIF, HDR, PIC, PNM
     */
    Document& add_image(const std::string& image_path, const Point& position,
                       double width = 0, double height = 0);

    /**
     * @brief 从core::Image对象添加图像
     * @param image 图像对象
     * @param position 图像位置（左下角）
     * @param width 图像宽度（0表示使用原始宽度）
     * @param height 图像高度（0表示使用原始高度或按比例缩放）
     * @return 文档引用（支持链式调用）
     */
    Document& add_image(const tinakit::core::Image& image, const Point& position,
                       double width = 0, double height = 0);

    /**
     * @brief 从原始数据添加图像
     * @param image_data 图像数据
     * @param width 图像宽度（像素）
     * @param height 图像高度（像素）
     * @param channels 颜色通道数（1=灰度，3=RGB，4=RGBA）
     * @param position 图像位置（左下角）
     * @param display_width 显示宽度（0表示使用原始宽度）
     * @param display_height 显示高度（0表示使用原始高度或按比例缩放）
     * @return 文档引用（支持链式调用）
     */
    Document& add_image(const std::vector<std::uint8_t>& image_data,
                       int width, int height, int channels,
                       const Point& position,
                       double display_width = 0, double display_height = 0);

    // ========================================
    // Excel集成功能
    // ========================================
    
    /**
     * @brief 从Excel工作表添加表格
     * @param sheet Excel工作表
     * @param range_address 范围地址（如"A1:E10"）
     * @param position 表格位置
     * @param preserve_formatting 是否保留Excel格式
     */
    Document& add_excel_table(const excel::Worksheet& sheet,
                             const std::string& range_address,
                             const Point& position,
                             bool preserve_formatting = true);
    
    /**
     * @brief 从Excel Range添加表格
     * @param range Excel范围
     * @param position 表格位置
     * @param preserve_formatting 是否保留Excel格式
     */
    Document& add_excel_range(const excel::Range& range,
                             const Point& position,
                             bool preserve_formatting = true);
    
    /**
     * @brief 添加Excel工作表的完整内容
     * @param sheet Excel工作表
     * @param preserve_formatting 是否保留格式
     */
    Document& add_excel_sheet(const excel::Worksheet& sheet, bool preserve_formatting = true);

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
