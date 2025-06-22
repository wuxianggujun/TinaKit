/**
 * @file test_basic_pdf.cpp
 * @brief PDF基本功能测试
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "test_framework.hpp"
#include "tinakit/pdf/document.hpp"
#include "tinakit/pdf/types.hpp"
#include <filesystem>

using namespace tinakit::pdf;
using namespace tinakit::test;

// ========================================
// PDF文档创建测试
// ========================================

TEST_CASE(PDFBasic, CreateDocument) {
    auto doc = Document::create();
    ASSERT_EQ(doc.page_count(), 0u);
}

TEST_CASE(PDFBasic, AddPage) {
    auto doc = Document::create();
    doc.add_page();
    ASSERT_EQ(doc.page_count(), 1u);

    doc.add_page();
    ASSERT_EQ(doc.page_count(), 2u);
}

TEST_CASE(PDFBasic, SetPageSize) {
    auto doc = Document::create();

    // 测试不同页面大小
    ASSERT_NO_THROW(doc.set_page_size(PageSize::A4, PageOrientation::Portrait));
    ASSERT_NO_THROW(doc.set_page_size(PageSize::A3, PageOrientation::Landscape));
    ASSERT_NO_THROW(doc.set_custom_page_size(500, 700));
}

TEST_CASE(PDFBasic, SetDocumentInfo) {
    auto doc = Document::create();

    DocumentInfo info;
    info.title = "测试文档";
    info.author = "TinaKit";
    info.subject = "PDF测试";
    info.keywords = "test, pdf";

    ASSERT_NO_THROW(doc.set_document_info(info));
}

// ========================================
// PDF内容添加测试
// ========================================

TEST_CASE(PDFContent, AddText) {
    auto doc = Document::create();
    doc.add_page();

    Font font;
    font.family = "Arial";
    font.size = 12;
    font.color = tinakit::Color::Black;

    Point position(100, 700);
    ASSERT_NO_THROW(doc.add_text("Hello, PDF!", position, font));
}

TEST_CASE(PDFContent, AddTextBlock) {
    auto doc = Document::create();
    doc.add_page();

    Font font;
    font.family = "Arial";
    font.size = 12;

    Rect bounds(100, 600, 400, 100);
    std::string text = "这是一个长文本块，用于测试文本块功能。";

    ASSERT_NO_THROW(doc.add_text_block(text, bounds, font, TextAlignment::Left));
}

TEST_CASE(PDFContent, AddMultipleTexts) {
    auto doc = Document::create();
    doc.add_page();

    Font title_font;
    title_font.family = "Arial";
    title_font.size = 18;
    title_font.bold = true;
    title_font.color = tinakit::Color::Blue;

    Font body_font;
    body_font.family = "Arial";
    body_font.size = 12;
    body_font.color = tinakit::Color::Black;

    ASSERT_NO_THROW(doc.add_text("PDF标题", Point(100, 750), title_font));
    ASSERT_NO_THROW(doc.add_text("这是正文内容", Point(100, 700), body_font));
    ASSERT_NO_THROW(doc.add_text("支持中文显示", Point(100, 680), body_font));
}

// ========================================
// PDF文件操作测试
// ========================================

TEST_CASE(PDFFile, SaveToFile) {
    // 清理可能存在的测试文件
    if (std::filesystem::exists("test_output.pdf")) {
        std::filesystem::remove("test_output.pdf");
    }

    auto doc = Document::create();
    doc.add_page();

    Font font;
    font.family = "Arial";
    font.size = 12;

    doc.add_text("Test PDF", Point(100, 700), font);

    // 保存文件
    ASSERT_NO_THROW(doc.save("test_output.pdf"));

    // 检查文件是否存在
    ASSERT_TRUE(std::filesystem::exists("test_output.pdf"));

    // 检查文件大小（应该大于0）
    auto file_size = std::filesystem::file_size("test_output.pdf");
    ASSERT_GT(file_size, 0u);

    // 清理测试文件
    std::filesystem::remove("test_output.pdf");
}

TEST_CASE(PDFFile, SaveToBuffer) {
    auto doc = Document::create();
    doc.add_page();

    Font font;
    font.family = "Arial";
    font.size = 12;

    doc.add_text("Test PDF Buffer", Point(100, 700), font);

    // 保存到缓冲区
    auto buffer = doc.save_to_buffer();

    // 缓冲区应该不为空
    ASSERT_GT(buffer.size(), 0u);

    // PDF文件应该以%PDF开头
    if (buffer.size() >= 4) {
        std::string header(buffer.begin(), buffer.begin() + 4);
        ASSERT_EQ(header, "%PDF");
    }
}

// ========================================
// PDF类型系统测试
// ========================================

TEST_CASE(PDFTypes, BasicTypes) {
    // 测试基本类型
    Point p(100, 200);
    ASSERT_EQ(p.x, 100.0);
    ASSERT_EQ(p.y, 200.0);

    Rect r(10, 20, 300, 400);
    ASSERT_EQ(r.x, 10.0);
    ASSERT_EQ(r.y, 20.0);
    ASSERT_EQ(r.width, 300.0);
    ASSERT_EQ(r.height, 400.0);

    Color c(255, 128, 64);
    ASSERT_EQ(c.red(), 255);
    ASSERT_EQ(c.green(), 128);
    ASSERT_EQ(c.blue(), 64);
    ASSERT_EQ(c.alpha(), 255);  // 默认alpha
}

TEST_CASE(PDFTypes, PredefinedColors) {
    // 测试预定义颜色
    ASSERT_EQ(tinakit::Color::Black.red(), 0);
    ASSERT_EQ(tinakit::Color::Black.green(), 0);
    ASSERT_EQ(tinakit::Color::Black.blue(), 0);

    ASSERT_EQ(tinakit::Color::White.red(), 255);
    ASSERT_EQ(tinakit::Color::White.green(), 255);
    ASSERT_EQ(tinakit::Color::White.blue(), 255);

    ASSERT_EQ(tinakit::Color::Red.red(), 255);
    ASSERT_EQ(tinakit::Color::Red.green(), 0);
    ASSERT_EQ(tinakit::Color::Red.blue(), 0);
}

TEST_CASE(PDFTypes, PageSizeConversion) {
    // 测试页面大小转换
    auto [width, height] = page_size_to_points(PageSize::A4, PageOrientation::Portrait);
    ASSERT_EQ(width, 595.0);
    ASSERT_EQ(height, 842.0);

    // 测试横向
    auto [width_l, height_l] = page_size_to_points(PageSize::A4, PageOrientation::Landscape);
    ASSERT_EQ(width_l, 842.0);
    ASSERT_EQ(height_l, 595.0);
}

TEST_CASE(PDFTypes, UnitConversion) {
    // 测试单位转换
    ASSERT_EQ(mm_to_points(25.4), 72.0);  // 1 inch = 25.4 mm = 72 points
    ASSERT_EQ(inches_to_points(1.0), 72.0);
    ASSERT_EQ(points_to_mm(72.0), 25.4);
    ASSERT_EQ(points_to_inches(72.0), 1.0);
}

// ========================================
// PDF表格测试
// ========================================

TEST_CASE(PDFTable, TableCreation) {
    Table table;

    // 添加表头
    TableRow header;
    header.push_back(TableCell("列1"));
    header.push_back(TableCell("列2"));
    header.push_back(TableCell("列3"));
    table.add_row(header);

    // 添加数据行
    table.add_row({"数据1", "数据2", "数据3"});
    table.add_row({"数据4", "数据5", "数据6"});

    ASSERT_EQ(table.row_count(), 3u);
    ASSERT_EQ(table.column_count(), 3u);
}

TEST_CASE(PDFTable, AddTableToPDF) {
    Table table;
    table.add_row({"姓名", "年龄", "部门"});
    table.add_row({"张三", "25", "技术部"});
    table.add_row({"李四", "30", "销售部"});

    auto doc = Document::create();
    doc.add_page();

    ASSERT_NO_THROW(doc.add_table(table, Point(100, 500)));
}

// ========================================
// PDF集成测试
// ========================================

TEST_CASE(PDFIntegration, ChainedOperations) {
    // 测试链式操作
    auto doc = Document::create();

    DocumentInfo info;
    info.title = "链式操作测试";
    info.author = "TinaKit";

    PageMargins margins;
    margins.top = 50;
    margins.bottom = 50;
    margins.left = 40;
    margins.right = 40;

    Font font;
    font.family = "Arial";
    font.size = 14;
    font.bold = true;

    // 链式操作
    ASSERT_NO_THROW(
        doc.set_document_info(info)
           .set_page_size(PageSize::A4)
           .set_margins(margins)
           .add_page()
           .add_text("标题", Point(100, 750), font)
           .add_text("内容", Point(100, 700))
    );

    ASSERT_EQ(doc.page_count(), 1u);
}
