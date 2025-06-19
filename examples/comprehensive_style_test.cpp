#include <iostream>
#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/conditional_format.hpp"

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== TinaKit 综合样式功能测试 ===" << std::endl;
        
        // 创建工作簿
        auto workbook = Workbook::create();
        
        // ========================================
        // 1. 字体和颜色测试
        // ========================================
        std::cout << "创建字体和颜色测试工作表..." << std::endl;
        auto& font_sheet = workbook.create_sheet("字体和颜色");
        
        // 标题
        font_sheet["A1"]
            .value("字体和颜色测试")
            .font("微软雅黑", 16)
            .bold()
            .color(Color::Blue)
            .background_color(Color::LightGray);
        
        // 不同字体测试
        font_sheet["A3"].value("Arial 字体").font("Arial", 12);
        font_sheet["A4"].value("Times New Roman 字体").font("Times New Roman", 12);
        font_sheet["A5"].value("Calibri 字体").font("Calibri", 12);
        font_sheet["A6"].value("微软雅黑字体").font("微软雅黑", 12);
        
        // 字体样式测试
        font_sheet["C3"].value("普通文本");
        font_sheet["C4"].value("粗体文本").bold();
        font_sheet["C5"].value("斜体文本").italic();
        font_sheet["C6"].value("粗体+斜体").bold().italic();
        
        // 颜色测试
        font_sheet["E3"].value("红色文字").color(Color::Red);
        font_sheet["E4"].value("绿色文字").color(Color::Green);
        font_sheet["E5"].value("蓝色文字").color(Color::Blue);
        font_sheet["E6"].value("自定义颜色").color(Color::from_hex("#FF6600"));
        
        // 背景色测试
        font_sheet["G3"].value("黄色背景").background_color(Color::Yellow);
        font_sheet["G4"].value("浅蓝背景").background_color(Color::LightBlue);
        font_sheet["G5"].value("浅绿背景").background_color(Color::LightGreen);
        font_sheet["G6"].value("浅红背景").background_color(Color::LightRed);
        
        // ========================================
        // 2. 对齐测试
        // ========================================
        std::cout << "创建对齐测试工作表..." << std::endl;
        auto& align_sheet = workbook.create_sheet("对齐测试");
        
        // 标题
        align_sheet["A1"]
            .value("对齐功能测试")
            .font("微软雅黑", 16)
            .bold()
            .color(Color::White)
            .background_color(Color::Blue);
        
        // 水平对齐测试
        align_sheet["A3"].value("水平对齐测试").bold();
        
        Alignment left_align;
        left_align.horizontal = Alignment::Horizontal::Left;
        left_align.vertical = Alignment::Vertical::Center;
        align_sheet["A4"].value("左对齐").align(left_align);
        
        Alignment center_align;
        center_align.horizontal = Alignment::Horizontal::Center;
        center_align.vertical = Alignment::Vertical::Center;
        align_sheet["B4"].value("居中对齐").align(center_align);
        
        Alignment right_align;
        right_align.horizontal = Alignment::Horizontal::Right;
        right_align.vertical = Alignment::Vertical::Center;
        align_sheet["C4"].value("右对齐").align(right_align);
        
        // 垂直对齐测试
        align_sheet["A6"].value("垂直对齐测试").bold();
        
        Alignment top_align;
        top_align.horizontal = Alignment::Horizontal::Center;
        top_align.vertical = Alignment::Vertical::Top;
        align_sheet["A7"].value("顶部对齐").align(top_align);
        
        Alignment middle_align;
        middle_align.horizontal = Alignment::Horizontal::Center;
        middle_align.vertical = Alignment::Vertical::Center;
        align_sheet["B7"].value("中间对齐").align(middle_align);
        
        Alignment bottom_align;
        bottom_align.horizontal = Alignment::Horizontal::Center;
        bottom_align.vertical = Alignment::Vertical::Bottom;
        align_sheet["C7"].value("底部对齐").align(bottom_align);
        
        // ========================================
        // 3. 数字格式测试
        // ========================================
        std::cout << "创建数字格式测试工作表..." << std::endl;
        auto& number_sheet = workbook.create_sheet("数字格式");
        
        // 标题
        number_sheet["A1"]
            .value("数字格式测试")
            .font("微软雅黑", 16)
            .bold()
            .background_color(Color::LightBlue);
        
        // 基础数字格式
        number_sheet["A3"].value("格式类型").bold();
        number_sheet["B3"].value("原始值").bold();
        number_sheet["C3"].value("格式化结果").bold();
        
        // 整数格式
        number_sheet["A4"].value("整数");
        number_sheet["B4"].value(12345);
        number_sheet["C4"].value(12345).number_format("0");
        
        // 小数格式
        number_sheet["A5"].value("两位小数");
        number_sheet["B5"].value(12345.678);
        number_sheet["C5"].value(12345.678).number_format("0.00");
        
        // 千分位格式
        number_sheet["A6"].value("千分位");
        number_sheet["B6"].value(1234567);
        number_sheet["C6"].value(1234567).number_format("#,##0");
        
        // 百分比格式
        number_sheet["A7"].value("百分比");
        number_sheet["B7"].value(0.1234);
        number_sheet["C7"].value(0.1234).number_format("0.00%");
        
        // 货币格式
        number_sheet["A8"].value("货币");
        number_sheet["B8"].value(1234.56);
        number_sheet["C8"].value(1234.56).number_format("¥#,##0.00");
        
        // ========================================
        // 4. 边框测试
        // ========================================
        std::cout << "创建边框测试工作表..." << std::endl;
        auto& border_sheet = workbook.create_sheet("边框测试");
        
        // 标题
        border_sheet["A1"]
            .value("边框测试")
            .font("微软雅黑", 16)
            .bold()
            .background_color(Color::LightGreen);
        
        // 不同边框样式测试
        border_sheet["A3"].value("细边框").border(BorderType::All, BorderStyle::Thin);
        border_sheet["B3"].value("粗边框").border(BorderType::All, BorderStyle::Thick);
        border_sheet["C3"].value("虚线边框").border(BorderType::All, BorderStyle::Dashed);
        border_sheet["D3"].value("点线边框").border(BorderType::All, BorderStyle::Dotted);
        
        // 单边框测试
        border_sheet["A5"].value("上边框").border(BorderType::Top, BorderStyle::Thin);
        border_sheet["B5"].value("下边框").border(BorderType::Bottom, BorderStyle::Thin);
        border_sheet["C5"].value("左边框").border(BorderType::Left, BorderStyle::Thin);
        border_sheet["D5"].value("右边框").border(BorderType::Right, BorderStyle::Thin);

        // 彩色边框测试
        border_sheet["A7"].value("红色边框").border(BorderType::All, BorderStyle::Medium, Color::Red);
        border_sheet["B7"].value("蓝色边框").border(BorderType::All, BorderStyle::Medium, Color::Blue);
        border_sheet["C7"].value("绿色边框").border(BorderType::All, BorderStyle::Medium, Color::Green);
        border_sheet["D7"].value("紫色边框").border(BorderType::All, BorderStyle::Medium, Color(128, 0, 128));

        // 文本换行和缩进测试
        border_sheet["A9"].value("这是一个很长的文本，用来测试文本换行功能，看看是否能正确换行显示").wrap_text(true);
        border_sheet["B9"].value("缩进级别1").indent(1);
        border_sheet["C9"].value("缩进级别2").indent(2);
        border_sheet["D9"].value("缩进级别3").indent(3);

        // 设置行高测试
        border_sheet.row(9).height(40.0);  // 设置第9行高度为40磅
        border_sheet.row(10).height(25.0); // 设置第10行高度为25磅

        // 设置列宽测试
        border_sheet.set_column_width("A", 25.0);  // A列宽度25字符
        border_sheet.set_column_width("B", 15.0);  // B列宽度15字符
        border_sheet.set_column_width("C", 18.0);  // C列宽度18字符
        border_sheet.set_column_width("D", 20.0);  // D列宽度20字符
        
        // ========================================
        // 5. 高级格式测试
        // ========================================
        std::cout << "创建高级格式测试工作表..." << std::endl;
        auto& advanced_sheet = workbook.create_sheet("高级格式");

        // 标题
        advanced_sheet["A1"]
            .value("高级格式功能测试")
            .font("微软雅黑", 16)
            .bold()
            .color(Color::White)
            .background_color(Color(75, 0, 130))  // 靛蓝色
            .align(center_align);

        // 文本换行测试
        advanced_sheet["A3"].value("文本换行测试").bold();
        advanced_sheet["A4"].value("这是一个很长的文本，用来测试自动换行功能。当文本超过单元格宽度时，应该自动换行到下一行显示。").wrap_text(true);
        advanced_sheet["A5"].value("不换行的长文本：这个文本不会换行，会超出单元格边界显示。");

        // 缩进测试（配合左对齐使用）
        Alignment left_align_indent;
        left_align_indent.horizontal = Alignment::Horizontal::Left;
        left_align_indent.vertical = Alignment::Vertical::Center;

        advanced_sheet["C3"].value("缩进测试（左对齐）").bold().align(left_align_indent);
        advanced_sheet["C4"].value("级别0：无缩进文本").align(left_align_indent).indent(0);
        advanced_sheet["C5"].value("级别1：一级缩进文本").align(left_align_indent).indent(1);
        advanced_sheet["C6"].value("级别2：二级缩进文本").align(left_align_indent).indent(2);
        advanced_sheet["C7"].value("级别3：三级缩进文本").align(left_align_indent).indent(3);
        advanced_sheet["C8"].value("级别4：四级缩进文本").align(left_align_indent).indent(4);
        advanced_sheet["C9"].value("级别5：五级缩进文本").align(left_align_indent).indent(5);

        // 彩色边框测试
        advanced_sheet["E3"].value("彩色边框").bold();
        advanced_sheet["E4"].value("红色").border(BorderType::All, BorderStyle::Thick, Color::Red);
        advanced_sheet["E5"].value("绿色").border(BorderType::All, BorderStyle::Thick, Color::Green);
        advanced_sheet["E6"].value("蓝色").border(BorderType::All, BorderStyle::Thick, Color::Blue);
        advanced_sheet["E7"].value("橙色").border(BorderType::All, BorderStyle::Thick, Color(255, 165, 0));

        // 组合测试
        advanced_sheet["A9"]
            .value("组合效果：换行+缩进+彩色边框")
            .wrap_text(true)
            .indent(1)
            .border(BorderType::All, BorderStyle::Medium, Color(255, 20, 147))  // 深粉色
            .background_color(Color(255, 240, 245));  // 淡粉色背景

        // 专门的缩进对比演示 - 测试调用顺序的灵活性！
        advanced_sheet["A11"].value("缩进效果对比（测试调用顺序）").bold().background_color(Color::LightBlue);
        advanced_sheet["A12"].value("先对齐后缩进：0空格").align(left_align_indent).indent(0).border(BorderType::All, BorderStyle::Thin);
        advanced_sheet["A13"].value("先缩进后对齐：5空格").indent(5).align(left_align_indent).border(BorderType::All, BorderStyle::Thin);
        advanced_sheet["A14"].value("先对齐后缩进：10空格").align(left_align_indent).indent(10).border(BorderType::All, BorderStyle::Thin);
        advanced_sheet["A15"].value("先缩进后对齐：15空格").indent(15).align(left_align_indent).border(BorderType::All, BorderStyle::Thin);
        advanced_sheet["A16"].value("对比：手动5空格").value("     |手动5空格对比").align(left_align_indent).border(BorderType::All, BorderStyle::Thin);
        advanced_sheet["A17"].value("对比：手动10空格").value("          |手动10空格对比").align(left_align_indent).border(BorderType::All, BorderStyle::Thin);

        // 实际应用场景：目录结构（使用合理的空格数）
        advanced_sheet["C11"].value("实际应用：目录结构").bold().background_color(Color::LightGreen);
        advanced_sheet["C12"].value("第一章").align(left_align_indent).indent(0);
        advanced_sheet["C13"].value("1.1 节").align(left_align_indent).indent(4);  // 4空格缩进
        advanced_sheet["C14"].value("1.1.1 小节").align(left_align_indent).indent(8);  // 8空格缩进
        advanced_sheet["C15"].value("1.1.2 小节").align(left_align_indent).indent(8);  // 8空格缩进
        advanced_sheet["C16"].value("1.2 节").align(left_align_indent).indent(4);  // 4空格缩进
        advanced_sheet["C17"].value("第二章").align(left_align_indent).indent(0);

        // 设置行高和列宽
        advanced_sheet.row(4).height(50.0);  // 换行测试行
        advanced_sheet.row(9).height(60.0);  // 组合测试行
        advanced_sheet.set_column_width("A", 35.0);  // 增加A列宽度以显示缩进效果
        advanced_sheet.set_column_width("C", 25.0);  // 增加C列宽度以显示缩进效果
        advanced_sheet.set_column_width("E", 15.0);

        // 合并单元格测试
        advanced_sheet["G3"].value("合并单元格测试").bold().background_color(Color::LightRed);
        advanced_sheet["G4"].value("这是一个合并的单元格，跨越多列").align(center_align).background_color(Color::Yellow);
        advanced_sheet.merge_cells("G4:I4");  // 合并G4到I4

        advanced_sheet["G6"].value("垂直合并").align(center_align).background_color(Color::LightGreen);
        advanced_sheet.merge_cells("G6:G8");  // 垂直合并G6到G8

        advanced_sheet["H6"].value("区域合并").align(center_align).background_color(Color::LightBlue);
        advanced_sheet.merge_cells("H6:I8");  // 区域合并H6到I8

        // ========================================
        // 6. 条件格式测试
        // ========================================
        std::cout << "创建条件格式测试工作表..." << std::endl;
        auto& conditional_sheet = workbook.create_sheet("条件格式");

        // 标题
        conditional_sheet["A1"]
            .value("条件格式功能测试")
            .font("微软雅黑", 16)
            .bold()
            .color(Color::White)
            .background_color(Color(128, 0, 128))  // 紫色
            .align(center_align);

        // 数值条件格式测试数据
        conditional_sheet["A3"].value("数值条件格式测试").bold();
        conditional_sheet["A4"].value("分数");
        conditional_sheet["B4"].value("等级");

        // 填入测试数据
        std::vector<int> scores = {95, 87, 76, 65, 58, 92, 81, 73, 69, 84};
        for (size_t i = 0; i < scores.size(); ++i) {
            conditional_sheet.cell(5 + i, 1).value(scores[i]);
        }

        // 应用条件格式：分数大于90显示绿色背景
        std::cout << "添加条件格式：分数>90显示绿色..." << std::endl;
        conditional_sheet.conditional_format("A5:A14")
            .when_greater_than(90)
            .background_color(Color::Green)
            .apply();

        // 检查条件格式是否添加成功
        const auto& formats = conditional_sheet.get_conditional_formats();
        std::cout << "当前条件格式数量: " << formats.size() << std::endl;

        // 应用条件格式：分数小于60显示红色背景
        conditional_sheet.conditional_format("A5:A14")
            .when_less_than(60)
            .background_color(Color::Red)
            .apply();

        // 应用条件格式：分数在70-89之间显示黄色背景
        conditional_sheet.conditional_format("A5:A14")
            .when_between(70, 89)
            .background_color(Color::Yellow)
            .apply();

        // 文本条件格式测试
        conditional_sheet["D3"].value("文本条件格式测试").bold();
        conditional_sheet["D4"].value("状态");

        std::vector<std::string> statuses = {"优秀", "良好", "一般", "差", "优秀", "良好", "差", "一般", "优秀", "良好"};
        for (size_t i = 0; i < statuses.size(); ++i) {
            conditional_sheet.cell(5 + i, 4).value(statuses[i]);
        }

        // 应用条件格式：包含"优秀"的文本显示绿色字体
        conditional_sheet.conditional_format("D5:D14")
            .when_contains("优秀")
            .font_color(Color::Green)
            .bold()
            .apply();

        // 应用条件格式：包含"差"的文本显示红色字体
        conditional_sheet.conditional_format("D5:D14")
            .when_contains("差")
            .font_color(Color::Red)
            .bold()
            .apply();

        // ========================================
        // 6. 综合样式测试
        // ========================================
        std::cout << "创建综合样式测试工作表..." << std::endl;
        auto& combo_sheet = workbook.create_sheet("综合样式");
        
        // 标题
        combo_sheet["A1"]
            .value("综合样式演示")
            .font("微软雅黑", 18)
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align(center_align)
            .border(BorderType::All, BorderStyle::Thick);
        
        // 复杂样式组合
        combo_sheet["A3"]
            .value("销售报表")
            .font("Arial", 14)
            .bold()
            .italic()
            .color(Color::Red)
            .background_color(Color::Yellow)
            .align(center_align)
            .border(BorderType::All, BorderStyle::Thin);
        
        combo_sheet["A5"]
            .value(98765.43)
            .font("Times New Roman", 12)
            .color(Color::Green)
            .number_format("¥#,##0.00")
            .align(right_align)
            .border(BorderType::Bottom, BorderStyle::Double);
        
        // 保存文件
        const std::string filename = "comprehensive_style_test.xlsx";
        workbook.save(filename);
        
        std::cout << "\n✅ 综合样式测试完成！" << std::endl;
        std::cout << "📁 文件已保存为: " << filename << std::endl;
        std::cout << "📋 包含以下测试工作表:" << std::endl;
        std::cout << "   • 字体和颜色 - 字体、颜色、背景色测试" << std::endl;
        std::cout << "   • 对齐测试 - 水平和垂直对齐测试" << std::endl;
        std::cout << "   • 数字格式 - 各种数字格式测试" << std::endl;
        std::cout << "   • 边框测试 - 边框样式、彩色边框、行高列宽测试" << std::endl;
        std::cout << "   • 高级格式 - 文本换行、缩进、彩色边框、合并单元格测试" << std::endl;
        std::cout << "   • 条件格式 - 数值条件、文本条件格式测试" << std::endl;
        std::cout << "   • 综合样式 - 复杂样式组合测试" << std::endl;
        std::cout << "\n🎨 新增功能:" << std::endl;
        std::cout << "   ✓ 彩色边框 - border(type, style, color)" << std::endl;
        std::cout << "   ✓ 文本换行 - wrap_text(true)" << std::endl;
        std::cout << "   ✓ 文本缩进 - indent(level) 【支持任意调用顺序】" << std::endl;
        std::cout << "   ✓ 行高设置 - row(n).height(value)" << std::endl;
        std::cout << "   ✓ 列宽设置 - set_column_width(column, width)" << std::endl;
        std::cout << "   ✓ 合并单元格 - merge_cells(range) / unmerge_cells(range)" << std::endl;
        std::cout << "   ✓ 条件格式 - conditional_format(range).when_xxx().apply()" << std::endl;
        std::cout << "\n💡 缩进说明:" << std::endl;
        std::cout << "   • 缩进是文本在单元格内的左边距偏移" << std::endl;
        std::cout << "   • 需要配合左对齐使用才能看到效果" << std::endl;
        std::cout << "   • 查看'高级格式'工作表的缩进对比演示" << std::endl;
        std::cout << "   • 层级结构演示展示了缩进的实际应用场景" << std::endl;
        std::cout << "\n请用Excel/WPS打开查看所有样式效果！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
