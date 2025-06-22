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
#include "tinakit/core/types.hpp"
#include "tinakit/core/color.hpp"
#include "tinakit/pdf/style.hpp"
#include "tinakit/excel/workbook.hpp"

namespace tinakit::pdf {

// 前向声明
namespace internal {
    class pdf_document_impl;
}

// 使用core命名空间中的几何类型
using Point = core::Point;
using Rect = core::Rect;

/**
 * @brief PDF页面大小枚举
 */
enum class PageSize {
    A4,         ///< A4 (210 × 297 mm)
    A3,         ///< A3 (297 × 420 mm)
    Letter,     ///< US Letter (8.5 × 11 inch)
    Legal,      ///< US Legal (8.5 × 14 inch)
    Custom      ///< 自定义大小
};

/**
 * @brief PDF页面方向
 */
enum class PageOrientation {
    Portrait,   ///< 纵向
    Landscape   ///< 横向
};

// TextAlignment 在 style.hpp 中定义

// 使用PDF样式系统中的字体样式
using Font = FontStyle;

/**
 * @brief PDF页面边距
 */
struct PageMargins {
    double top = 72.0;      ///< 上边距 (点)
    double bottom = 72.0;   ///< 下边距 (点)
    double left = 72.0;     ///< 左边距 (点)
    double right = 72.0;    ///< 右边距 (点)
};

/**
 * @brief PDF文档元数据
 */
struct DocumentInfo {
    std::string title;          ///< 文档标题
    std::string author;         ///< 作者
    std::string subject;        ///< 主题
    std::string keywords;       ///< 关键词
    std::string creator = "TinaKit PDF Library";  ///< 创建者
};

/**
 * @brief PDF表格单元格
 */
struct TableCell {
    std::string text;                       ///< 单元格文本
    CellStyle style;                        ///< 单元格样式

    /**
     * @brief 默认构造函数
     */
    TableCell() = default;

    /**
     * @brief 构造函数
     */
    TableCell(const std::string& cell_text, const CellStyle& cell_style = CellStyle())
        : text(cell_text), style(cell_style) {}
};

/**
 * @brief PDF表格行
 */
using TableRow = std::vector<TableCell>;

/**
 * @brief PDF表格
 */
struct Table {
    std::vector<TableRow> rows;         ///< 表格行
    std::vector<double> column_widths;  ///< 列宽度
    TableStyle style;                   ///< 表格样式
    bool has_header = false;            ///< 是否有表头

    /**
     * @brief 默认构造函数
     */
    Table() = default;

    /**
     * @brief 构造函数
     */
    Table(const TableStyle& table_style) : style(table_style) {}
};

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
