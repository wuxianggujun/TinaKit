/**
 * @file conditional_format_test.cpp
 * @brief 专门测试条件格式功能
 */

#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/conditional_format.hpp"
#include "tinakit/core/color.hpp"
#include <iostream>
#include <vector>
using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== 条件格式专项测试 ===" << std::endl;
        
        // 创建新工作簿
        auto workbook = Workbook::create();
        auto& sheet = workbook.create_sheet("条件格式测试");
        
        // ========================================
        // 测试1：数值条件格式（背景色）
        // ========================================
        std::cout << "\n📊 测试1：数值条件格式（背景色）" << std::endl;
        
        sheet["A1"].value("分数").bold();
        sheet["B1"].value("预期效果").bold();
        
        // 添加测试数据
        std::vector<int> scores = {95, 87, 76, 65, 58, 92, 81, 73, 69, 84};
        std::vector<std::string> expected = {
            "绿色背景(>90)", "黄色背景(70-89)", "黄色背景(70-89)", 
            "无背景(60-69)", "红色背景(<60)", "绿色背景(>90)",
            "黄色背景(70-89)", "黄色背景(70-89)", "无背景(60-69)", "黄色背景(70-89)"
        };
        
        for (size_t i = 0; i < scores.size(); ++i) {
            sheet.cell(2 + i, 1).value(scores[i]);
            sheet.cell(2 + i, 2).value(expected[i]);
        }
        
        // 应用数值条件格式
        std::cout << "添加条件：分数>90显示绿色背景..." << std::endl;
        sheet.conditional_format("A2:A11")
            .when_greater_than(90)
            .background_color(Color::Green)
            .apply();
            
        std::cout << "添加条件：分数<60显示红色背景..." << std::endl;
        sheet.conditional_format("A2:A11")
            .when_less_than(60)
            .background_color(Color::Red)
            .apply();
            
        std::cout << "添加条件：分数70-89显示黄色背景..." << std::endl;
        sheet.conditional_format("A2:A11")
            .when_between(70, 89)
            .background_color(Color::Yellow)
            .apply();
        
        // ========================================
        // 测试2：文本条件格式（字体样式）
        // ========================================
        std::cout << "\n📝 测试2：文本条件格式（字体样式）" << std::endl;
        
        sheet["D1"].value("状态").bold();
        sheet["E1"].value("预期效果").bold();
        
        std::vector<std::string> statuses = {"优秀", "良好", "一般", "差", "优秀", "良好", "差", "一般", "优秀", "良好"};
        std::vector<std::string> text_expected = {
            "绿色粗体", "普通", "普通", "红色粗体", "绿色粗体",
            "普通", "红色粗体", "普通", "绿色粗体", "普通"
        };
        
        for (size_t i = 0; i < statuses.size(); ++i) {
            sheet.cell(2 + i, 4).value(statuses[i]);
            sheet.cell(2 + i, 5).value(text_expected[i]);
        }
        
        // 应用文本条件格式
        std::cout << "添加条件：包含'优秀'显示绿色粗体..." << std::endl;
        sheet.conditional_format("D2:D11")
            .when_contains("优秀")
            .font("Calibri", 11)  // 使用Calibri，11pt字体（与手动设置完全一致）
            .font_color(Color::Green)
            .bold()
            .apply();

        std::cout << "添加条件：包含'差'显示红色粗体..." << std::endl;
        sheet.conditional_format("D2:D11")
            .when_contains("差")
            .font("Calibri", 11)  // 使用Calibri，11pt字体（与手动设置完全一致）
            .font_color(Color::Red)
            .bold()
            .apply();
        
        // ========================================
        // 测试3：混合条件格式（背景色+字体）
        // ========================================
        std::cout << "\n🎨 测试3：混合条件格式（背景色+字体）" << std::endl;
        
        sheet["G1"].value("等级").bold();
        sheet["H1"].value("预期效果").bold();
        
        std::vector<std::string> grades = {"A+", "A", "B+", "B", "C+", "C", "D", "F"};
        std::vector<std::string> mixed_expected = {
            "绿底白字粗体", "绿底黑字粗体", "蓝底白字粗体", "蓝底黑字",
            "黄底黑字", "黄底黑字", "橙底黑字", "红底白字粗体"
        };
        
        for (size_t i = 0; i < grades.size(); ++i) {
            sheet.cell(2 + i, 7).value(grades[i]);
            sheet.cell(2 + i, 8).value(mixed_expected[i]);
        }
        
        // 应用混合条件格式
        std::cout << "添加条件：A+等级显示绿底白字粗体..." << std::endl;
        sheet.conditional_format("G2:G9")
            .when_contains("A+")
            .background_color(Color::Green)
            .font("Calibri", 11)  // 使用Calibri，11pt字体（与手动设置完全一致）
            .font_color(Color::White)
            .bold()
            .apply();

        std::cout << "添加条件：F等级显示红底白字粗体..." << std::endl;
        sheet.conditional_format("G2:G9")
            .when_contains("F")
            .background_color(Color::Red)
            .font("Calibri", 11)  // 使用Calibri，11pt字体（与手动设置完全一致）
            .font_color(Color::White)
            .bold()
            .apply();
        
        // 设置列宽
        sheet.set_column_width("A", 8);
        sheet.set_column_width("B", 20);
        sheet.set_column_width("D", 8);
        sheet.set_column_width("E", 15);
        sheet.set_column_width("G", 8);
        sheet.set_column_width("H", 20);
        
        // 保存文件
        std::string filename = "conditional_format_test_v3.xlsx";
        workbook.save(filename);
        
        std::cout << "\n✅ 条件格式测试完成！" << std::endl;
        std::cout << "📁 文件已保存为: " << filename << std::endl;
        
        std::cout << "\n🔍 请检查以下效果：" << std::endl;
        std::cout << "📊 A列数值条件格式：" << std::endl;
        std::cout << "   • A2(95), A6(92) → 绿色背景" << std::endl;
        std::cout << "   • A9(58) → 红色背景" << std::endl;
        std::cout << "   • A3(87), A7(76), A8(81), A9(73), A11(84) → 黄色背景" << std::endl;
        std::cout << "   • A4(65), A10(69) → 无背景色" << std::endl;
        
        std::cout << "\n📝 D列文本条件格式：" << std::endl;
        std::cout << "   • D2,D5,D9(优秀) → 绿色粗体字" << std::endl;
        std::cout << "   • D4,D7(差) → 红色粗体字" << std::endl;
        std::cout << "   • 其他 → 普通字体" << std::endl;
        
        std::cout << "\n🎨 G列混合条件格式：" << std::endl;
        std::cout << "   • G2(A+) → 绿底白字粗体" << std::endl;
        std::cout << "   • G9(F) → 红底白字粗体" << std::endl;
        
        std::cout << "\n💡 如果看不到效果，请检查：" << std::endl;
        std::cout << "   1. Excel版本是否支持条件格式" << std::endl;
        std::cout << "   2. 尝试重新打开文件" << std::endl;
        std::cout << "   3. 在Excel中查看'开始'→'条件格式'→'管理规则'" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
