# 教程 5：高级格式化和样式设置

Excel 的魅力不仅在于数据处理，更在于美观的格式化展示。TinaKit 提供了强大而直观的样式设置功能，让你能够创建专业级的 Excel 报表。本教程将深入探讨各种格式化技巧。

## 🎯 学习目标

完成本教程后，你将能够：
- 掌握字体、颜色、对齐等基础样式设置
- 使用链式调用创建复杂的样式组合
- 设置边框、背景和数字格式
- 应用条件格式化
- 创建专业的报表模板
- 优化样式性能

## 📋 前置条件

- 完成前四个教程
- 了解 Excel 的基本格式化概念
- 熟悉颜色和字体的基本知识

## 🎨 基础样式设置

### 字体和文本样式

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("字体样式示例");

        std::cout << "=== 字体和文本样式 ===" << std::endl;

        // 1. 基础字体设置
        sheet["A1"]
            .value("标题文本")
            .font("微软雅黑", 18)
            .bold()
            .color(Color::DarkBlue);

        // 2. 不同字体族
        sheet["A3"].value("Arial 字体").font("Arial", 12);
        sheet["A4"].value("Times New Roman").font("Times New Roman", 12);
        sheet["A5"].value("Courier New").font("Courier New", 12);
        sheet["A6"].value("宋体").font("宋体", 12);

        // 3. 字体样式组合
        sheet["B3"]
            .value("粗体文本")
            .font("Arial", 12)
            .bold();

        sheet["B4"]
            .value("斜体文本")
            .font("Arial", 12)
            .italic();

        sheet["B5"]
            .value("粗斜体")
            .font("Arial", 12)
            .bold()
            .italic();

        // 4. 不同字体大小
        for (int size = 8; size <= 20; size += 2) {
            int row = 8 + (size - 8) / 2;
            sheet["A" + std::to_string(row)]
                .value("字体大小 " + std::to_string(size))
                .font("Arial", size);
        }

        // 5. 文本颜色
        sheet["C3"].value("红色文本").color(Color::Red);
        sheet["C4"].value("绿色文本").color(Color::Green);
        sheet["C5"].value("蓝色文本").color(Color::Blue);
        sheet["C6"].value("自定义颜色").color(Color(255, 165, 0)); // 橙色

        workbook.save("font_styles.xlsx");
        std::cout << "✅ 字体样式示例已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### 对齐和文本方向

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("对齐样式示例");

        std::cout << "=== 对齐和文本方向 ===" << std::endl;

        // 设置列宽以便观察对齐效果
        sheet.column("A").width(20);
        sheet.column("B").width(20);
        sheet.column("C").width(20);

        // 1. 水平对齐
        sheet["A1"].value("对齐方式").bold().align(Alignment::Center);
        sheet["B1"].value("示例文本").bold().align(Alignment::Center);
        sheet["C1"].value("说明").bold().align(Alignment::Center);

        sheet["A3"].value("左对齐").bold();
        sheet["B3"].value("Left Aligned Text").align(Alignment::Left);
        sheet["C3"].value("默认对齐方式");

        sheet["A4"].value("居中对齐").bold();
        sheet["B4"].value("Center Aligned").align(Alignment::Center);
        sheet["C4"].value("标题常用");

        sheet["A5"].value("右对齐").bold();
        sheet["B5"].value("Right Aligned").align(Alignment::Right);
        sheet["C5"].value("数字常用");

        sheet["A6"].value("两端对齐").bold();
        sheet["B6"].value("Justified Text").align(Alignment::Justify);
        sheet["C6"].value("长文本使用");

        // 2. 垂直对齐（默认已经是垂直居中，像WPS一样）
        sheet.row(8).height(40);  // 增加行高以观察垂直对齐

        sheet["A8"].value("顶部对齐").align(Alignment::Top);
        sheet["B8"].value("中间对齐（默认）").align(Alignment::Middle);
        sheet["C8"].value("底部对齐").align(Alignment::Bottom);

        // 3. 组合对齐
        sheet["A10"]
            .value("居中标题")
            .font("微软雅黑", 14)
            .bold()
            .align(Alignment::Center)
            .background_color(Color::LightBlue);

        workbook.save("alignment_styles.xlsx");
        std::cout << "✅ 对齐样式示例已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 🌈 颜色和背景

### 颜色系统详解

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("颜色系统示例");

        std::cout << "=== 颜色和背景 ===" << std::endl;

        // 1. 预定义颜色
        sheet["A1"].value("预定义颜色").bold();

        std::vector<std::pair<std::string, Color>> predefined_colors = {
            {"黑色", Color::Black},
            {"白色", Color::White},
            {"红色", Color::Red},
            {"绿色", Color::Green},
            {"蓝色", Color::Blue},
            {"黄色", Color::Yellow},
            {"青色", Color::Cyan},
            {"洋红", Color::Magenta},
            {"浅灰", Color::LightGray},
            {"深灰", Color::DarkGray}
        };

        for (size_t i = 0; i < predefined_colors.size(); ++i) {
            int row = static_cast<int>(i + 2);
            sheet["A" + std::to_string(row)]
                .value(predefined_colors[i].first)
                .background_color(predefined_colors[i].second)
                .color(i < 5 ? Color::White : Color::Black);  // 深色背景用白字
        }

        // 2. RGB 自定义颜色
        sheet["C1"].value("RGB 自定义颜色").bold();

        std::vector<std::tuple<std::string, int, int, int>> custom_colors = {
            {"橙色", 255, 165, 0},
            {"紫色", 128, 0, 128},
            {"粉色", 255, 192, 203},
            {"棕色", 165, 42, 42},
            {"海军蓝", 0, 0, 128},
            {"橄榄绿", 128, 128, 0},
            {"深红", 139, 0, 0},
            {"金色", 255, 215, 0}
        };

        for (size_t i = 0; i < custom_colors.size(); ++i) {
            int row = static_cast<int>(i + 2);
            auto [name, r, g, b] = custom_colors[i];

            sheet["C" + std::to_string(row)]
                .value(name + " RGB(" + std::to_string(r) + "," +
                       std::to_string(g) + "," + std::to_string(b) + ")")
                .background_color(Color(r, g, b))
                .color(Color::White);
        }

        // 3. 十六进制颜色
        sheet["E1"].value("十六进制颜色").bold();

        std::vector<std::pair<std::string, std::string>> hex_colors = {
            {"深蓝", "#003366"},
            {"深绿", "#006633"},
            {"深红", "#660033"},
            {"紫罗兰", "#663399"},
            {"青绿", "#339966"},
            {"橙红", "#FF6633"}
        };

        for (size_t i = 0; i < hex_colors.size(); ++i) {
            int row = static_cast<int>(i + 2);
            sheet["E" + std::to_string(row)]
                .value(hex_colors[i].first + " " + hex_colors[i].second)
                .background_color(Color(hex_colors[i].second))
                .color(Color::White);
        }

        // 4. 渐变效果模拟
        sheet["A12"].value("渐变效果模拟").bold();

        for (int i = 0; i < 10; ++i) {
            int intensity = 255 - (i * 25);  // 从255递减到5
            sheet["A" + std::to_string(13 + i)]
                .value("渐变 " + std::to_string(i + 1))
                .background_color(Color(intensity, intensity, 255))
                .color(i > 5 ? Color::White : Color::Black);
        }

        workbook.save("color_system.xlsx");
        std::cout << "✅ 颜色系统示例已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 📐 边框和线条

### 边框样式详解

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("边框样式示例");

        std::cout << "=== 边框和线条 ===" << std::endl;

        // 设置列宽
        for (char col = 'A'; col <= 'F'; ++col) {
            sheet.column(std::string(1, col)).width(15);
        }

        // 1. 基础边框类型
        sheet["A1"].value("边框类型").bold().align(Alignment::Center);

        sheet["A3"]
            .value("无边框")
            .border(BorderType::None, BorderStyle::None);

        sheet["B3"]
            .value("全边框")
            .border(BorderType::All, BorderStyle::Thin)
            .align(Alignment::Center);

        sheet["C3"]
            .value("顶部边框")
            .border(BorderType::Top, BorderStyle::Thin)
            .align(Alignment::Center);

        sheet["D3"]
            .value("底部边框")
            .border(BorderType::Bottom, BorderStyle::Thin)
            .align(Alignment::Center);

        sheet["E3"]
            .value("左边框")
            .border(BorderType::Left, BorderStyle::Thin)
            .align(Alignment::Center);

        sheet["F3"]
            .value("右边框")
            .border(BorderType::Right, BorderStyle::Thin)
            .align(Alignment::Center);

        // 2. 边框样式
        sheet["A5"].value("边框样式").bold().align(Alignment::Center);

        std::vector<std::pair<std::string, BorderStyle>> border_styles = {
            {"细线", BorderStyle::Thin},
            {"中等线", BorderStyle::Medium},
            {"粗线", BorderStyle::Thick},
            {"双线", BorderStyle::Double},
            {"点线", BorderStyle::Dotted},
            {"虚线", BorderStyle::Dashed}
        };

        for (size_t i = 0; i < border_styles.size(); ++i) {
            char col = 'A' + static_cast<char>(i);
            sheet[std::string(1, col) + "7"]
                .value(border_styles[i].first)
                .border(BorderType::All, border_styles[i].second)
                .align(Alignment::Center);
        }

        // 3. 复杂边框组合
        sheet["A9"].value("复杂边框").bold();

        // 创建表格样式
        auto table_range = sheet.range("A11:D15");

        // 设置表格数据
        sheet["A11"].value("产品").bold();
        sheet["B11"].value("价格").bold();
        sheet["C11"].value("库存").bold();
        sheet["D11"].value("状态").bold();

        sheet["A12"].value("笔记本");
        sheet["B12"].value("¥5999");
        sheet["C12"].value("50");
        sheet["D12"].value("正常");

        sheet["A13"].value("手机");
        sheet["B13"].value("¥3999");
        sheet["C13"].value("100");
        sheet["D13"].value("充足");

        sheet["A14"].value("平板");
        sheet["B14"].value("¥2999");
        sheet["C14"].value("25");
        sheet["D14"].value("偏少");

        sheet["A15"].value("总计");
        sheet["B15"].value("¥12997").bold();
        sheet["C15"].value("175").bold();
        sheet["D15"].value("").bold();

        // 设置表格边框
        table_range.border(BorderType::All, BorderStyle::Thin);

        // 标题行特殊边框
        sheet.range("A11:D11")
            .border(BorderType::Bottom, BorderStyle::Thick)
            .background_color(Color::LightBlue);

        // 总计行特殊边框
        sheet.range("A15:D15")
            .border(BorderType::Top, BorderStyle::Thick)
            .background_color(Color::LightGray);

        workbook.save("border_styles.xlsx");
        std::cout << "✅ 边框样式示例已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 🔢 数字格式化

### 数字、货币和日期格式

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <chrono>

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("数字格式化示例");

        std::cout << "=== 数字格式化 ===" << std::endl;

        // 设置标题
        sheet["A1"].value("格式类型").bold();
        sheet["B1"].value("原始值").bold();
        sheet["C1"].value("格式化结果").bold();
        sheet["D1"].value("格式代码").bold();
        sheet["E1"].value("说明").bold();

        int row = 3;

        // 1. 基础数字格式
        sheet["A" + std::to_string(row)].value("整数");
        sheet["B" + std::to_string(row)].value(1234);
        sheet["C" + std::to_string(row)].value(1234).number_format("0");
        sheet["D" + std::to_string(row)].value("0");
        row++;

        sheet["A" + std::to_string(row)].value("千分位");
        sheet["B" + std::to_string(row)].value(1234567);
        sheet["C" + std::to_string(row)].value(1234567).number_format("#,##0");
        sheet["D" + std::to_string(row)].value("#,##0");
        row++;

        sheet["A" + std::to_string(row)].value("小数");
        sheet["B" + std::to_string(row)].value(1234.567);
        sheet["C" + std::to_string(row)].value(1234.567).number_format("0.00");
        sheet["D" + std::to_string(row)].value("0.00");
        row++;

        sheet["A" + std::to_string(row)].value("千分位小数");
        sheet["B" + std::to_string(row)].value(1234567.89);
        sheet["C" + std::to_string(row)].value(1234567.89).number_format("#,##0.00");
        sheet["D" + std::to_string(row)].value("#,##0.00");
        row++;

        // 2. 货币格式
        row++;
        sheet["A" + std::to_string(row)].value("人民币").bold();
        sheet["B" + std::to_string(row)].value(1234.56);
        sheet["C" + std::to_string(row)].value(1234.56).number_format("¥#,##0.00");
        sheet["D" + std::to_string(row)].value("¥#,##0.00");
        row++;

        sheet["A" + std::to_string(row)].value("美元");
        sheet["B" + std::to_string(row)].value(1234.56);
        sheet["C" + std::to_string(row)].value(1234.56).number_format("$#,##0.00");
        sheet["D" + std::to_string(row)].value("$#,##0.00");
        row++;

        sheet["A" + std::to_string(row)].value("欧元");
        sheet["B" + std::to_string(row)].value(1234.56);
        sheet["C" + std::to_string(row)].value(1234.56).number_format("€#,##0.00");
        sheet["D" + std::to_string(row)].value("€#,##0.00");
        row++;

        // 3. 百分比格式（注意：Excel中百分比格式会自动将小数乘以100）
        row++;
        sheet["A" + std::to_string(row)].value("百分比").bold();
        sheet["B" + std::to_string(row)].value(0.1234);  // 原始值：0.1234
        sheet["C" + std::to_string(row)].value(0.1234).number_format("0.00%");  // 显示为：12.34%
        sheet["D" + std::to_string(row)].value("0.00%");
        sheet["E" + std::to_string(row)].value("自动×100");
        row++;

        sheet["A" + std::to_string(row)].value("整数百分比");
        sheet["B" + std::to_string(row)].value(0.1234);  // 原始值：0.1234
        sheet["C" + std::to_string(row)].value(0.1234).number_format("0%");     // 显示为：12%
        sheet["D" + std::to_string(row)].value("0%");
        sheet["E" + std::to_string(row)].value("自动×100");
        row++;

        // 4. 日期时间格式
        auto now = std::chrono::system_clock::now();

        row++;
        sheet["A" + std::to_string(row)].value("日期格式").bold();
        sheet["B" + std::to_string(row)].value(now);
        sheet["C" + std::to_string(row)].value(now).number_format("yyyy-mm-dd");
        sheet["D" + std::to_string(row)].value("yyyy-mm-dd");
        row++;

        sheet["A" + std::to_string(row)].value("时间格式");
        sheet["B" + std::to_string(row)].value(now);
        sheet["C" + std::to_string(row)].value(now).number_format("hh:mm:ss");
        sheet["D" + std::to_string(row)].value("hh:mm:ss");
        row++;

        sheet["A" + std::to_string(row)].value("日期时间");
        sheet["B" + std::to_string(row)].value(now);
        sheet["C" + std::to_string(row)].value(now).number_format("yyyy-mm-dd hh:mm:ss");
        sheet["D" + std::to_string(row)].value("yyyy-mm-dd hh:mm:ss");
        row++;

        // 5. 自定义格式
        row++;
        sheet["A" + std::to_string(row)].value("自定义格式").bold();
        sheet["B" + std::to_string(row)].value(1234567);
        sheet["C" + std::to_string(row)].value(1234567).number_format("[>1000000]#,##0,\"万\";#,##0");
        sheet["D" + std::to_string(row)].value("[>1000000]#,##0,\"万\";#,##0");
        row++;

        // 设置表格样式
        sheet.range("A1:D1").background_color(Color::LightBlue).bold();
        sheet.range("A1:D" + std::to_string(row - 1)).border(BorderType::All, BorderStyle::Thin);

        // 调整列宽
        sheet.column("A").width(15);
        sheet.column("B").width(15);
        sheet.column("C").width(20);
        sheet.column("D").width(25);

        workbook.save("number_formats.xlsx");
        std::cout << "✅ 数字格式化示例已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 🎯 条件格式化

### 动态样式设置

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <vector>
#include <random>

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("条件格式化示例");

        std::cout << "=== 条件格式化 ===" << std::endl;

        // 生成示例数据
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> score_dist(60.0, 100.0);
        std::uniform_real_distribution<> sales_dist(5000.0, 50000.0);

        // 设置标题
        sheet["A1"].value("学生姓名").bold();
        sheet["B1"].value("数学成绩").bold();
        sheet["C1"].value("英语成绩").bold();
        sheet["D1"].value("平均分").bold();
        sheet["E1"].value("等级").bold();

        sheet["G1"].value("销售员").bold();
        sheet["H1"].value("销售额").bold();
        sheet["I1"].value("完成率").bold();
        sheet["J1"].value("奖金").bold();

        std::vector<std::string> students = {
            "张三", "李四", "王五", "赵六", "钱七", "孙八", "周九", "吴十"
        };

        std::vector<std::string> salespeople = {
            "销售A", "销售B", "销售C", "销售D", "销售E", "销售F"
        };

        // 生成学生成绩数据
        for (size_t i = 0; i < students.size(); ++i) {
            int row = static_cast<int>(i + 2);

            double math_score = score_dist(gen);
            double english_score = score_dist(gen);
            double average = (math_score + english_score) / 2.0;

            sheet["A" + std::to_string(row)].value(students[i]);
            sheet["B" + std::to_string(row)].value(math_score).number_format("0.0");
            sheet["C" + std::to_string(row)].value(english_score).number_format("0.0");
            sheet["D" + std::to_string(row)].value(average).number_format("0.0");

            // 条件格式化 - 成绩等级
            std::string grade;
            Color grade_color;
            if (average >= 90) {
                grade = "优秀";
                grade_color = Color::Green;
            } else if (average >= 80) {
                grade = "良好";
                grade_color = Color::Blue;
            } else if (average >= 70) {
                grade = "中等";
                grade_color = Color(255, 165, 0); // 橙色
            } else {
                grade = "需努力";
                grade_color = Color::Red;
            }

            sheet["E" + std::to_string(row)]
                .value(grade)
                .color(grade_color)
                .bold();

            // 成绩背景色条件格式化
            if (math_score >= 90) {
                sheet["B" + std::to_string(row)].background_color(Color::LightGreen);
            } else if (math_score < 70) {
                sheet["B" + std::to_string(row)].background_color(Color(255, 200, 200)); // 浅红
            }

            if (english_score >= 90) {
                sheet["C" + std::to_string(row)].background_color(Color::LightGreen);
            } else if (english_score < 70) {
                sheet["C" + std::to_string(row)].background_color(Color(255, 200, 200)); // 浅红
            }
        }

        // 生成销售数据
        for (size_t i = 0; i < salespeople.size(); ++i) {
            int row = static_cast<int>(i + 2);

            double sales = sales_dist(gen);
            double target = 20000.0;  // 目标销售额
            double completion_rate = sales / target;
            double bonus = 0.0;

            // 计算奖金
            if (completion_rate >= 1.5) {
                bonus = sales * 0.15;
            } else if (completion_rate >= 1.2) {
                bonus = sales * 0.10;
            } else if (completion_rate >= 1.0) {
                bonus = sales * 0.05;
            }

            sheet["G" + std::to_string(row)].value(salespeople[i]);
            sheet["H" + std::to_string(row)].value(sales).number_format("¥#,##0");
            sheet["I" + std::to_string(row)].value(completion_rate).number_format("0.0%");
            sheet["J" + std::to_string(row)].value(bonus).number_format("¥#,##0");

            // 条件格式化 - 完成率
            Color completion_color;
            if (completion_rate >= 1.5) {
                completion_color = Color::Green;
            } else if (completion_rate >= 1.2) {
                completion_color = Color::Blue;
            } else if (completion_rate >= 1.0) {
                completion_color = Color(255, 165, 0); // 橙色
            } else {
                completion_color = Color::Red;
            }

            sheet["I" + std::to_string(row)]
                .color(completion_color)
                .bold();

            // 销售额数据条（模拟）
            double bar_width = (sales / 50000.0) * 100;  // 按最大值50000计算宽度
            if (bar_width > 80) {
                sheet["H" + std::to_string(row)].background_color(Color::Green);
            } else if (bar_width > 60) {
                sheet["H" + std::to_string(row)].background_color(Color::LightGreen);
            } else if (bar_width > 40) {
                sheet["H" + std::to_string(row)].background_color(Color::Yellow);
            } else {
                sheet["H" + std::to_string(row)].background_color(Color(255, 200, 200));
            }
        }

        // 使用 TinaKit 的条件格式化 API
        sheet.range("B2:C9")
            .conditional_format()
            .when_greater_than(90)
            .background_color(Color::LightGreen);

        sheet.range("B2:C9")
            .conditional_format()
            .when_less_than(70)
            .background_color(Color(255, 200, 200));

        sheet.range("H2:H7")
            .conditional_format()
            .when_greater_than(30000)
            .background_color(Color::Green);

        // 设置表格边框
        sheet.range("A1:E9").border(BorderType::All, BorderStyle::Thin);
        sheet.range("G1:J7").border(BorderType::All, BorderStyle::Thin);

        // 标题行样式
        sheet.range("A1:E1").background_color(Color::LightBlue);
        sheet.range("G1:J1").background_color(Color::LightBlue);

        workbook.save("conditional_formatting.xlsx");
        std::cout << "✅ 条件格式化示例已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 📊 专业报表模板

### 创建企业级报表

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

class ReportTemplate {
private:
    Workbook& workbook_;
    Worksheet& sheet_;

public:
    ReportTemplate(Workbook& wb, Worksheet& ws) : workbook_(wb), sheet_(ws) {}

    void create_header(const std::string& title, const std::string& subtitle = "") {
        // 公司标题
        sheet_["A1"]
            .value(title)
            .font("微软雅黑", 20)
            .bold()
            .color(Color(0, 51, 102))  // 深蓝色
            .align(Alignment::Center);

        if (!subtitle.empty()) {
            sheet_["A2"]
                .value(subtitle)
                .font("微软雅黑", 14)
                .italic()
                .color(Color(102, 102, 102))  // 灰色
                .align(Alignment::Center);
        }

        // 日期
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y年%m月%d日");

        sheet_["A3"]
            .value("报告日期: " + ss.str())
            .font("微软雅黑", 10)
            .color(Color(102, 102, 102))
            .align(Alignment::Center);

        // 分隔线
        sheet_.range("A4:J4")
            .border(BorderType::Bottom, BorderStyle::Thick)
            .background_color(Color(0, 51, 102));
    }

    void create_summary_section(int start_row) {
        // 摘要标题
        sheet_["A" + std::to_string(start_row)]
            .value("执行摘要")
            .font("微软雅黑", 16)
            .bold()
            .color(Color(0, 51, 102));

        // 摘要背景
        sheet_.range("A" + std::to_string(start_row) + ":J" + std::to_string(start_row))
            .background_color(Color(230, 240, 250));

        // 关键指标
        int row = start_row + 2;

        // KPI 卡片样式
        create_kpi_card("B" + std::to_string(row), "总收入", "¥2,450,000", Color::Green);
        create_kpi_card("D" + std::to_string(row), "总支出", "¥1,680,000", Color::Red);
        create_kpi_card("F" + std::to_string(row), "净利润", "¥770,000", Color::Blue);
        create_kpi_card("H" + std::to_string(row), "利润率", "31.4%", Color(255, 165, 0));
    }

    void create_kpi_card(const std::string& start_cell,
                        const std::string& label,
                        const std::string& value,
                        const Color& color) {
        // 解析起始单元格
        char col = start_cell[0];
        int row = std::stoi(start_cell.substr(1));

        // 标签
        sheet_[start_cell]
            .value(label)
            .font("微软雅黑", 10)
            .color(Color(102, 102, 102))
            .align(Alignment::Center);

        // 值
        sheet_[std::string(1, col) + std::to_string(row + 1)]
            .value(value)
            .font("微软雅黑", 14)
            .bold()
            .color(color)
            .align(Alignment::Center);

        // 卡片边框
        std::string range = start_cell + ":" + std::string(1, col + 1) + std::to_string(row + 1);
        sheet_.range(range)
            .border(BorderType::All, BorderStyle::Thin)
            .background_color(Color(248, 248, 248));
    }

    void create_data_table(int start_row, const std::string& title) {
        // 表格标题
        sheet_["A" + std::to_string(start_row)]
            .value(title)
            .font("微软雅黑", 14)
            .bold()
            .color(Color(0, 51, 102));

        int header_row = start_row + 2;

        // 表格标题行
        std::vector<std::string> headers = {
            "项目", "Q1", "Q2", "Q3", "Q4", "总计", "增长率"
        };

        for (size_t i = 0; i < headers.size(); ++i) {
            char col = 'A' + static_cast<char>(i);
            sheet_[std::string(1, col) + std::to_string(header_row)]
                .value(headers[i])
                .font("微软雅黑", 11)
                .bold()
                .color(Color::White)
                .background_color(Color(0, 51, 102))
                .align(Alignment::Center);
        }

        // 示例数据
        std::vector<std::vector<std::string>> data = {
            {"销售收入", "580,000", "620,000", "650,000", "600,000", "2,450,000", "12.5%"},
            {"营销费用", "120,000", "130,000", "135,000", "125,000", "510,000", "8.3%"},
            {"运营费用", "280,000", "290,000", "295,000", "285,000", "1,150,000", "5.2%"},
            {"净利润", "180,000", "200,000", "220,000", "190,000", "790,000", "18.7%"}
        };

        for (size_t i = 0; i < data.size(); ++i) {
            int row = header_row + 1 + static_cast<int>(i);

            for (size_t j = 0; j < data[i].size(); ++j) {
                char col = 'A' + static_cast<char>(j);
                auto cell = sheet_[std::string(1, col) + std::to_string(row)];

                cell.value(data[i][j]);

                if (j == 0) {
                    // 项目名称列
                    cell.font("微软雅黑", 10).bold();
                } else if (j == data[i].size() - 1) {
                    // 增长率列
                    cell.font("微软雅黑", 10)
                        .color(data[i][j].find("-") != std::string::npos ? Color::Red : Color::Green)
                        .bold();
                } else {
                    // 数据列
                    cell.font("微软雅黑", 10).number_format("#,##0");
                }

                // 交替行背景色
                if (i % 2 == 1) {
                    cell.background_color(Color(248, 248, 248));
                }
            }
        }

        // 表格边框
        std::string table_range = "A" + std::to_string(header_row) + ":G" +
                                 std::to_string(header_row + static_cast<int>(data.size()));
        sheet_.range(table_range).border(BorderType::All, BorderStyle::Thin);
    }

    void create_footer(int start_row) {
        // 分隔线
        sheet_.range("A" + std::to_string(start_row) + ":J" + std::to_string(start_row))
            .border(BorderType::Top, BorderStyle::Medium)
            .background_color(Color(230, 230, 230));

        // 页脚信息
        sheet_["A" + std::to_string(start_row + 1)]
            .value("本报告由 TinaKit 自动生成")
            .font("微软雅黑", 9)
            .italic()
            .color(Color(102, 102, 102))
            .align(Alignment::Center);

        sheet_["A" + std::to_string(start_row + 2)]
            .value("机密文件 - 仅供内部使用")
            .font("微软雅黑", 8)
            .color(Color::Red)
            .align(Alignment::Center);
    }

    void apply_global_styles() {
        // 设置默认列宽
        for (char col = 'A'; col <= 'J'; ++col) {
            sheet_.column(std::string(1, col)).width(12);
        }

        // 特殊列宽调整
        sheet_.column("A").width(15);  // 项目名称列
        sheet_.column("G").width(10);  // 增长率列
    }
};

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("财务报表");

        std::cout << "=== 创建专业报表模板 ===" << std::endl;

        ReportTemplate report(workbook, sheet);

        // 应用全局样式
        report.apply_global_styles();

        // 创建报表各部分
        report.create_header("ABC 公司财务报表", "2024年度季度分析");
        report.create_summary_section(6);
        report.create_data_table(12, "季度财务数据");
        report.create_footer(20);

        workbook.save("professional_report.xlsx");
        std::cout << "✅ 专业报表模板已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 🚀 性能优化技巧

### 批量样式设置

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <chrono>

int main() {
    using namespace tinakit;

    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("性能优化示例");

        std::cout << "=== 样式设置性能优化 ===" << std::endl;

        auto start_time = std::chrono::high_resolution_clock::now();

        // 方法 1：逐个设置样式（较慢）
        std::cout << "方法 1：逐个设置样式..." << std::endl;
        auto method1_start = std::chrono::high_resolution_clock::now();

        for (int row = 1; row <= 100; ++row) {
            for (int col = 1; col <= 10; ++col) {
                std::string cell_addr = column_number_to_name(col) + std::to_string(row);
                sheet[cell_addr]
                    .value("数据" + std::to_string(row) + "-" + std::to_string(col))
                    .font("Arial", 10)
                    .border(BorderType::All, BorderStyle::Thin);
            }
        }

        auto method1_end = std::chrono::high_resolution_clock::now();
        auto method1_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            method1_end - method1_start).count();

        // 方法 2：批量样式设置（更快）
        std::cout << "方法 2：批量样式设置..." << std::endl;
        auto method2_start = std::chrono::high_resolution_clock::now();

        // 创建样式对象
        CellStyle data_style;
        data_style.font("Arial", 10)
                  .border(BorderType::All, BorderStyle::Thin);

        // 批量应用样式
        sheet.range("L1:U100").style(data_style);

        // 批量设置数据
        for (int row = 1; row <= 100; ++row) {
            for (int col = 12; col <= 21; ++col) {  // L-U 列
                std::string cell_addr = column_number_to_name(col) + std::to_string(row);
                sheet[cell_addr].value("数据" + std::to_string(row) + "-" + std::to_string(col - 11));
            }
        }

        auto method2_end = std::chrono::high_resolution_clock::now();
        auto method2_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            method2_end - method2_start).count();

        // 方法 3：样式模板（最快）
        std::cout << "方法 3：样式模板..." << std::endl;
        auto method3_start = std::chrono::high_resolution_clock::now();

        // 预定义样式模板
        auto header_style = CellStyle()
            .font("微软雅黑", 12)
            .bold()
            .background_color(Color::LightBlue)
            .align(Alignment::Center);

        auto data_style_alt = CellStyle()
            .font("Arial", 10)
            .border(BorderType::All, BorderStyle::Thin);

        auto highlight_style = CellStyle()
            .font("Arial", 10)
            .bold()
            .background_color(Color::Yellow)
            .border(BorderType::All, BorderStyle::Thin);

        // 批量应用不同样式
        sheet.range("W1:AF1").style(header_style);
        sheet.range("W2:AF100").style(data_style_alt);

        // 条件样式应用
        for (int row = 2; row <= 100; ++row) {
            if (row % 10 == 0) {  // 每10行高亮
                sheet.range("W" + std::to_string(row) + ":AF" + std::to_string(row))
                    .style(highlight_style);
            }
        }

        auto method3_end = std::chrono::high_resolution_clock::now();
        auto method3_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            method3_end - method3_start).count();

        // 性能对比结果
        sheet["A102"].value("性能对比结果").font("微软雅黑", 14).bold();
        sheet["A104"].value("方法 1 (逐个设置)");
        sheet["B104"].value(std::to_string(method1_duration) + " ms");
        sheet["A105"].value("方法 2 (批量设置)");
        sheet["B105"].value(std::to_string(method2_duration) + " ms");
        sheet["A106"].value("方法 3 (样式模板)");
        sheet["B106"].value(std::to_string(method3_duration) + " ms");

        // 性能提升计算
        double improvement2 = static_cast<double>(method1_duration) / method2_duration;
        double improvement3 = static_cast<double>(method1_duration) / method3_duration;

        sheet["A108"].value("性能提升");
        sheet["A109"].value("方法 2 vs 方法 1");
        sheet["B109"].value(std::format("{:.1f}x 更快", improvement2));
        sheet["A110"].value("方法 3 vs 方法 1");
        sheet["B110"].value(std::format("{:.1f}x 更快", improvement3));

        std::cout << "性能对比结果:" << std::endl;
        std::cout << "  方法 1: " << method1_duration << " ms" << std::endl;
        std::cout << "  方法 2: " << method2_duration << " ms ("
                  << std::format("{:.1f}x", improvement2) << " 更快)" << std::endl;
        std::cout << "  方法 3: " << method3_duration << " ms ("
                  << std::format("{:.1f}x", improvement3) << " 更快)" << std::endl;

        workbook.save("performance_optimization.xlsx");
        std::cout << "✅ 性能优化示例已保存" << std::endl;

    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## 📝 小结

在这个教程中，你学会了：

✅ **基础样式设置**：字体、颜色、对齐等基本格式化技巧
✅ **颜色系统**：预定义颜色、RGB 自定义、十六进制颜色的使用
✅ **边框和线条**：各种边框类型和样式的应用
✅ **数字格式化**：货币、百分比、日期时间等专业格式
✅ **条件格式化**：基于数据值的动态样式设置
✅ **专业报表**：企业级报表模板的创建技巧
✅ **性能优化**：批量样式设置和样式模板的高效使用

## 🎨 最佳实践

1. **样式复用**：创建样式模板，避免重复设置
2. **批量操作**：使用范围操作而非逐个单元格设置
3. **条件格式**：让数据自动呈现视觉效果
4. **颜色搭配**：使用协调的颜色方案
5. **性能考虑**：大量数据时优先使用批量方法

## 🚀 下一步

现在你已经掌握了高级格式化技能，接下来可以：

- 探索 [Word 文档处理](../guides/word-processing.md)
- 学习 [图表和图像处理](../guides/charts-and-images.md)
- 查看 [完整 API 参考](../api-reference/index.md)
- 参与 [开源贡献](../../CONTRIBUTING.md)

---

恭喜！你已经完成了 TinaKit 的所有核心教程。现在你具备了创建专业 Excel 应用的全部技能！🎉
```
```