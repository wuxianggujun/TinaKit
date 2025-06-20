#include <iostream>
#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/style_template.hpp"
#include "tinakit/excel/worksheet_range.hpp"

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== TinaKit 样式模板和范围操作测试 ===" << std::endl;
        
        // 创建工作簿
        auto workbook = tinakit::excel::Workbook::create();
        
        // ========================================
        // 1. 样式模板测试
        // ========================================
        std::cout << "创建样式模板测试工作表..." << std::endl;
        // TODO: 实现 create_sheet 方法后启用
        // auto& template_sheet = workbook.create_sheet("样式模板测试");
        
        // 创建自定义样式模板
        std::cout << "创建自定义样式模板..." << std::endl;
        
        // 标题样式
        auto title_style = StyleTemplate()
            .font("微软雅黑", 18)
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align_horizontal(Alignment::Horizontal::Center)
            .align_vertical(Alignment::Vertical::Center);
        
        // 表头样式
        auto header_style = StyleTemplate()
            .font("Calibri", 12)
            .bold()
            .color(Color::Black)
            .background_color(Color::LightGray)
            .align_horizontal(Alignment::Horizontal::Center)
            .border(BorderType::All, BorderStyle::Thin);
        
        // 数据样式
        auto data_style = StyleTemplate()
            .font("Calibri", 11)
            .color(Color::Black)
            .align_vertical(Alignment::Vertical::Center)
            .border(BorderType::All, BorderStyle::Thin);
        
        // 高亮样式
        auto highlight_style = StyleTemplate()
            .background_color(Color::Yellow)
            .bold();
        
        // 应用样式模板到单个单元格
        std::cout << "应用样式模板到单个单元格..." << std::endl;
        // TODO: 实现 create_sheet 方法后启用
        /*
        template_sheet["A1"].value("样式模板演示").style(title_style);

        // 表头数据
        template_sheet["A3"].value("姓名").style(header_style);
        template_sheet["B3"].value("年龄").style(header_style);
        template_sheet["C3"].value("部门").style(header_style);
        template_sheet["D3"].value("薪资").style(header_style);

        // 数据行
        template_sheet["A4"].value("张三").style(data_style);
        template_sheet["B4"].value(28).style(data_style);
        template_sheet["C4"].value("技术部").style(data_style);
        template_sheet["D4"].value(8000).style(data_style);

        template_sheet["A5"].value("李四").style(data_style);
        template_sheet["B5"].value(32).style(data_style);
        template_sheet["C5"].value("销售部").style(data_style);
        template_sheet["D5"].value(12000).style(highlight_style); // 高薪高亮
        */
        
        // ========================================
        // 2. 预定义样式模板测试
        // ========================================
        std::cout << "测试预定义样式模板..." << std::endl;

        // TODO: 实现 create_sheet 方法后启用
        /*
        template_sheet["A7"].value("预定义样式演示").style(StyleTemplates::title());
        template_sheet["A8"].value("副标题").style(StyleTemplates::subtitle());
        template_sheet["A9"].value("表头样式").style(StyleTemplates::header());
        template_sheet["A10"].value("数据样式").style(StyleTemplates::data());
        template_sheet["A11"].value("高亮样式").style(StyleTemplates::highlight());
        template_sheet["A12"].value("警告样式").style(StyleTemplates::warning());
        template_sheet["A13"].value("错误样式").style(StyleTemplates::error());
        template_sheet["A14"].value("成功样式").style(StyleTemplates::success());
        */
        
        // ========================================
        // 3. 范围操作测试
        // ========================================
        std::cout << "创建范围操作测试工作表..." << std::endl;
        // TODO: 实现 create_sheet 方法后启用
        /*
        auto& range_sheet = workbook.create_sheet("范围操作测试");

        // 批量样式设置
        std::cout << "测试批量样式设置..." << std::endl;

        // 设置标题范围
        auto title_range = range_sheet.range("A1:D1");
        title_range.value(std::string("批量操作演示"))
                   .style(StyleTemplates::title());

        // 设置表头范围
        auto header_range = range_sheet.range("A3:D3");
        std::vector<std::vector<std::string>> header_data = {
            {"产品名称", "数量", "单价", "总价"}
        };
        header_range.values(header_data)
                    .style(StyleTemplates::header());

        // 设置数据范围
        auto data_range = range_sheet.range("A4:D7");
        std::vector<std::vector<std::string>> product_data = {
            {"苹果", "10", "5.5", "55"},
            {"香蕉", "20", "3.2", "64"},
            {"橙子", "15", "4.8", "72"},
            {"葡萄", "8", "12.0", "96"}
        };
        data_range.values(product_data)
                  .style(StyleTemplates::data());

        // 高亮高价值产品
        auto high_value_range = range_sheet.range("A7:D7");
        high_value_range.style(StyleTemplates::highlight(Color::Green));
        */
        
        // TODO: 实现 create_sheet 方法后启用以下所有测试
        /*
        // ========================================
        // 4. 链式范围操作测试
        // ========================================
        std::cout << "测试链式范围操作..." << std::endl;

        // 创建一个复杂的表格
        range_sheet.range("F1:I1")
            .value(std::string("销售报表"))
            .font("微软雅黑", 16)
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align_horizontal(Alignment::Horizontal::Center)
            .align_vertical(Alignment::Vertical::Center);

        // 表头
        range_sheet.range("F3:I3")
            .font("Calibri", 12)
            .bold()
            .background_color(Color::LightGray)
            .border(BorderType::All, BorderStyle::Medium);

        // 设置表头文本
        range_sheet["F3"].value("月份");
        range_sheet["G3"].value("销售额");
        range_sheet["H3"].value("目标");
        range_sheet["I3"].value("完成率");

        // 数据行
        range_sheet.range("F4:I7")
            .border(BorderType::All, BorderStyle::Thin)
            .align_vertical(Alignment::Vertical::Center);

        // 填充数据
        std::vector<std::vector<std::string>> sales_data = {
            {"1月", "85000", "80000", "106%"},
            {"2月", "92000", "85000", "108%"},
            {"3月", "78000", "80000", "98%"},
            {"4月", "95000", "90000", "106%"}
        };
        range_sheet.range("F4:I7").values(sales_data);

        // 高亮超额完成的月份
        range_sheet.range("F4:I4").background_color(Color::Green); // 1月
        range_sheet.range("F5:I5").background_color(Color::Green); // 2月
        range_sheet.range("F7:I7").background_color(Color::Green); // 4月

        // 标记未完成目标的月份
        range_sheet.range("F6:I6").background_color(Color::Yellow); // 3月

        // ========================================
        // 5. 迭代器测试
        // ========================================
        std::cout << "测试范围迭代器..." << std::endl;

        auto test_range = range_sheet.range("K1:M3");

        // 使用迭代器设置值
        int counter = 1;
        for (auto& cell : test_range) {
            cell.value("Cell " + std::to_string(counter++));
        }

        // 应用样式到整个范围
        test_range.style(StyleTemplates::data());

        // ========================================
        // 6. 性能测试（大范围操作）
        // ========================================
        std::cout << "测试大范围操作性能..." << std::endl;

        auto& perf_sheet = workbook.create_sheet("性能测试");

        // 创建一个大范围并批量设置样式
        auto large_range = perf_sheet.range("A1:Z100");
        large_range.value(std::string("批量数据"))
                   .font("Calibri", 10)
                   .border(BorderType::All, BorderStyle::Thin)
                   .align_vertical(Alignment::Vertical::Center);

        std::cout << "大范围操作完成，共处理 " << large_range.cell_count() << " 个单元格" << std::endl;
        */
        
        // ========================================
        // 保存文件
        // ========================================
        std::string filename = "style_template_test.xlsx";
        std::cout << "保存文件: " << filename << std::endl;

        // TODO: 实现 save 方法后启用
        // workbook.save(filename);

        std::cout << "✅ 样式模板和范围操作测试完成！" << std::endl;
        std::cout << "📁 请查看生成的文件: " << filename << std::endl;

        // 输出测试统计
        std::cout << "\n📊 测试统计:" << std::endl;
        std::cout << "- 工作表数量: 1 (基础测试)" << std::endl;
        std::cout << "- 样式模板类型: 8种" << std::endl;
        std::cout << "- 范围操作测试: 6组 (待实现)" << std::endl;
        // std::cout << "- 大范围单元格数: " << large_range.cell_count() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
