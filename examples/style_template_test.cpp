#include <iostream>
#include "tinakit/tinakit.hpp"
// 注意：style_template 和 worksheet_range 功能尚未实现

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== TinaKit 基础样式测试 ===" << std::endl;

        // 创建工作簿
        auto workbook = tinakit::excel::Workbook::create();
        auto sheet = workbook.active_sheet();
        sheet.set_name("基础样式测试");

        // ========================================
        // 1. 基础样式测试（使用已实现的功能）
        // ========================================
        std::cout << "测试基础样式功能..." << std::endl;

        // 标题样式
        sheet["A1"].value("TinaKit 样式演示")
                   .font("微软雅黑", 18)
                   .bold(true)
                   .color(Color::White)
                   .background_color(Color::Blue);

        // 表头样式
        sheet["A3"].value("姓名").font("Calibri", 12).bold(true).background_color(Color::LightGray);
        sheet["B3"].value("年龄").font("Calibri", 12).bold(true).background_color(Color::LightGray);
        sheet["C3"].value("部门").font("Calibri", 12).bold(true).background_color(Color::LightGray);
        sheet["D3"].value("薪资").font("Calibri", 12).bold(true).background_color(Color::LightGray);

        // 数据行
        sheet["A4"].value("张三").font("Calibri", 11);
        sheet["B4"].value(28).font("Calibri", 11);
        sheet["C4"].value("技术部").font("Calibri", 11);
        sheet["D4"].value(8000).font("Calibri", 11).number_format("#,##0");

        sheet["A5"].value("李四").font("Calibri", 11);
        sheet["B5"].value(32).font("Calibri", 11);
        sheet["C5"].value("销售部").font("Calibri", 11);
        sheet["D5"].value(12000).font("Calibri", 11).number_format("#,##0").background_color(Color::Yellow); // 高薪高亮

        // ========================================
        // 2. 颜色和格式测试
        // ========================================
        std::cout << "测试不同颜色和格式..." << std::endl;

        sheet["A7"].value("颜色演示").bold(true);
        sheet["A8"].value("红色文本").color(Color::Red);
        sheet["A9"].value("绿色背景").background_color(Color::Green);
        sheet["A10"].value("蓝色文本").color(Color::Blue);
        sheet["A11"].value("黄色高亮").background_color(Color::Yellow);
        sheet["A12"].value("灰色背景").background_color(Color::LightGray);

        // 数字格式测试
        sheet["A14"].value("数字格式演示").bold(true);
        sheet["A15"].value(1234.56).number_format("#,##0.00");
        sheet["A16"].value(0.85).number_format("0.00%");
        sheet["A17"].value(1234567.89).number_format("$#,##0.00");

        // ========================================
        // 保存文件
        // ========================================
        std::string filename = "basic_style_test.xlsx";
        std::cout << "保存文件: " << filename << std::endl;

        workbook.save(filename);

        std::cout << "✅ 基础样式测试完成！" << std::endl;
        std::cout << "📁 请查看生成的文件: " << filename << std::endl;

        // 输出测试统计
        std::cout << "\n📊 测试统计:" << std::endl;
        std::cout << "- 工作表数量: 1" << std::endl;
        std::cout << "- 测试的样式功能: 字体、颜色、背景、数字格式" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
