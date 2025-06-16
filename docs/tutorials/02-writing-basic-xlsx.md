# 教程 2：创建和写入 Excel 文件

在这个教程中，你将学会如何使用 TinaKit 创建新的 Excel 文件并写入数据。我们将从最简单的例子开始，逐步学习更高级的功能。

## 🎯 学习目标

完成本教程后，你将能够：
- 创建新的 Excel 工作簿
- 添加和管理工作表
- 写入不同类型的数据
- 设置公式和基本格式
- 保存文件

## 📋 前置条件

- 完成 [教程 1：读取简单的 Excel 文件](01-reading-simple-xlsx.md)
- 了解基本的 Excel 概念（工作簿、工作表、单元格）

## 🚀 创建第一个 Excel 文件

让我们从最简单的例子开始：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        // 创建新的工作簿
        auto workbook = Excel::create();
        
        // 获取默认工作表
        auto& sheet = workbook.active_sheet();
        
        // 写入一些数据
        sheet["A1"].value("Hello, TinaKit!");
        sheet["A2"].value("这是我的第一个 Excel 文件");
        
        // 保存文件
        workbook.save("my_first_file.xlsx");
        
        std::cout << "✅ 文件创建成功：my_first_file.xlsx" << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 🔍 代码解析

1. **创建工作簿**：`Excel::create()` 创建一个新的空工作簿
2. **获取工作表**：`workbook.active_sheet()` 获取默认工作表
3. **写入数据**：`sheet["A1"].value("...")` 设置单元格的值
4. **保存文件**：`workbook.save("filename.xlsx")` 保存到文件

## 📊 写入不同类型的数据

TinaKit 支持多种数据类型的写入：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <chrono>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        
        // 设置标题行
        sheet["A1"].value("数据类型");
        sheet["B1"].value("示例值");
        
        // 字符串
        sheet["A2"].value("字符串");
        sheet["B2"].value("Hello World");
        
        // 整数
        sheet["A3"].value("整数");
        sheet["B3"].value(42);
        
        // 浮点数
        sheet["A4"].value("浮点数");
        sheet["B4"].value(3.14159);
        
        // 布尔值
        sheet["A5"].value("布尔值");
        sheet["B5"].value(true);
        
        // 日期时间
        sheet["A6"].value("日期");
        auto now = std::chrono::system_clock::now();
        sheet["B6"].value(now);
        
        // 大数字
        sheet["A7"].value("大数字");
        sheet["B7"].value(1234567890.123);
        
        // 空值（清空单元格）
        sheet["A8"].value("空值");
        sheet["B8"].clear();
        
        workbook.save("data_types.xlsx");
        
        std::cout << "✅ 数据类型示例文件创建成功！" << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🏗️ 使用构建器模式

TinaKit 支持流畅的构建器模式，让代码更加简洁：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        // 使用构建器模式创建多个工作表
        auto workbook = Excel::create()
            .add_sheet("销售数据")
            .add_sheet("统计报表")
            .add_sheet("图表数据");
        
        // 在销售数据表中写入数据
        auto& sales_sheet = workbook["销售数据"];
        
        // 设置标题
        sales_sheet["A1"].value("产品名称");
        sales_sheet["B1"].value("销售额");
        sales_sheet["C1"].value("增长率");
        
        // 写入数据
        sales_sheet["A2"].value("iPhone");
        sales_sheet["B2"].value(1000000.0);
        sales_sheet["C2"].value(0.15);
        
        sales_sheet["A3"].value("iPad");
        sales_sheet["B3"].value(500000.0);
        sales_sheet["C3"].value(0.08);
        
        sales_sheet["A4"].value("MacBook");
        sales_sheet["B4"].value(800000.0);
        sales_sheet["C4"].value(0.12);
        
        // 在统计报表中写入汇总数据
        auto& stats_sheet = workbook["统计报表"];
        stats_sheet["A1"].value("总销售额");
        stats_sheet["B1"].value(2300000.0);
        
        stats_sheet["A2"].value("平均增长率");
        stats_sheet["B2"].value(0.117);  // (0.15 + 0.08 + 0.12) / 3
        
        workbook.save("sales_report.xlsx");
        
        std::cout << "✅ 销售报表创建成功！" << std::endl;
        std::cout << "包含工作表数量: " << workbook.sheet_count() << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 📐 批量数据写入

对于大量数据，可以使用更高效的批量写入方法：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <vector>
#include <tuple>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("员工信息");
        
        // 设置标题行
        sheet["A1"].value("姓名");
        sheet["B1"].value("年龄");
        sheet["C1"].value("部门");
        sheet["D1"].value("薪资");
        
        // 准备批量数据
        std::vector<std::tuple<std::string, int, std::string, double>> employees = {
            {"张三", 28, "技术部", 15000.0},
            {"李四", 32, "销售部", 12000.0},
            {"王五", 25, "市场部", 11000.0},
            {"赵六", 35, "技术部", 18000.0},
            {"钱七", 29, "人事部", 10000.0}
        };
        
        // 方法 1：逐行写入
        for (size_t i = 0; i < employees.size(); ++i) {
            int row = static_cast<int>(i + 2);  // 从第2行开始
            
            sheet["A" + std::to_string(row)].value(std::get<0>(employees[i]));
            sheet["B" + std::to_string(row)].value(std::get<1>(employees[i]));
            sheet["C" + std::to_string(row)].value(std::get<2>(employees[i]));
            sheet["D" + std::to_string(row)].value(std::get<3>(employees[i]));
        }
        
        // 方法 2：使用批量写入（更高效）
        sheet.write_data("A2", employees);
        
        workbook.save("employees.xlsx");
        
        std::cout << "✅ 员工信息表创建成功！" << std::endl;
        std::cout << "写入员工数量: " << employees.size() << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🧮 设置公式

Excel 的强大功能之一是公式计算：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("财务计算");
        
        // 设置数据
        sheet["A1"].value("项目");
        sheet["B1"].value("收入");
        sheet["C1"].value("支出");
        sheet["D1"].value("利润");
        
        sheet["A2"].value("项目A");
        sheet["B2"].value(100000);
        sheet["C2"].value(60000);
        
        sheet["A3"].value("项目B");
        sheet["B3"].value(150000);
        sheet["C3"].value(80000);
        
        sheet["A4"].value("项目C");
        sheet["B4"].value(200000);
        sheet["C4"].value(120000);
        
        // 设置公式计算利润
        sheet["D2"].formula("B2-C2");
        sheet["D3"].formula("B3-C3");
        sheet["D4"].formula("B4-C4");
        
        // 设置汇总行
        sheet["A6"].value("总计");
        sheet["B6"].formula("SUM(B2:B4)");  // 总收入
        sheet["C6"].formula("SUM(C2:C4)");  // 总支出
        sheet["D6"].formula("SUM(D2:D4)");  // 总利润
        
        // 设置平均值
        sheet["A7"].value("平均");
        sheet["B7"].formula("AVERAGE(B2:B4)");
        sheet["C7"].formula("AVERAGE(C2:C4)");
        sheet["D7"].formula("AVERAGE(D2:D4)");
        
        workbook.save("financial_report.xlsx");
        
        std::cout << "✅ 财务报表创建成功！" << std::endl;
        
        // 验证公式是否正确设置
        if (sheet["D2"].is_formula()) {
            std::cout << "D2 公式: " << *sheet["D2"].formula() << std::endl;
        }
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🎨 基本格式设置

让我们为数据添加一些基本的格式：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("格式化示例");
        
        // 创建标题行并设置格式
        sheet["A1"]
            .value("产品报表")
            .font("Arial", 16)
            .bold()
            .color(Color::Blue)
            .background_color(Color::LightGray)
            .align(Alignment::Center);
        
        // 设置列标题
        sheet["A3"].value("产品名称").bold();
        sheet["B3"].value("价格").bold();
        sheet["C3"].value("库存").bold();
        sheet["D3"].value("状态").bold();
        
        // 写入数据并设置格式
        sheet["A4"].value("笔记本电脑");
        sheet["B4"].value(5999.99).number_format("¥#,##0.00");
        sheet["C4"].value(50);
        sheet["D4"].value("充足").color(Color::Green);
        
        sheet["A5"].value("智能手机");
        sheet["B5"].value(3999.99).number_format("¥#,##0.00");
        sheet["C5"].value(5);
        sheet["D5"].value("紧缺").color(Color::Red);
        
        sheet["A6"].value("平板电脑");
        sheet["B6"].value(2999.99).number_format("¥#,##0.00");
        sheet["C6"].value(25);
        sheet["D6"].value("正常").color(Color::Black);
        
        // 设置标题行范围的格式
        sheet.range("A3:D3")
            .background_color(Color::LightBlue)
            .bold();
        
        // 设置边框
        sheet.range("A3:D6")
            .border(BorderType::All, BorderStyle::Thin);
        
        workbook.save("formatted_report.xlsx");
        
        std::cout << "✅ 格式化报表创建成功！" << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 💾 保存选项

TinaKit 提供了多种保存选项：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <filesystem>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        
        sheet["A1"].value("保存示例");
        sheet["A2"].value("当前时间");
        sheet["B2"].value(std::chrono::system_clock::now());
        
        // 方法 1：保存到指定路径
        workbook.save("output/example1.xlsx");
        std::cout << "✅ 保存到 output/example1.xlsx" << std::endl;
        
        // 方法 2：另存为
        workbook.save_as("output/example1_copy.xlsx");
        std::cout << "✅ 另存为 output/example1_copy.xlsx" << std::endl;
        
        // 方法 3：检查文件是否有未保存的更改
        sheet["A3"].value("新增数据");
        
        if (workbook.has_unsaved_changes()) {
            std::cout << "⚠️  有未保存的更改" << std::endl;
            workbook.save();  // 保存到原文件
            std::cout << "✅ 更改已保存" << std::endl;
        }
        
        // 方法 4：获取文件信息
        std::cout << "文件路径: " << workbook.file_path() << std::endl;
        std::cout << "文件大小: " << workbook.file_size() << " 字节" << std::endl;
        
    } catch (const IOException& e) {
        std::cerr << "❌ I/O 错误: " << e.what() << std::endl;
        std::cerr << "文件路径: " << e.file_path() << std::endl;
        return 1;
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔧 完整示例：创建销售报表

让我们把所有学到的知识结合起来，创建一个完整的销售报表：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string format_date(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d");
    return ss.str();
}

int main() {
    using namespace tinakit;
    
    try {
        // 创建工作簿
        auto workbook = Excel::create();
        
        // 重命名默认工作表
        auto& sheet = workbook.active_sheet();
        sheet.title("月度销售报表");
        
        // 创建报表标题
        auto now = std::chrono::system_clock::now();
        sheet["A1"]
            .value("月度销售报表")
            .font("微软雅黑", 18)
            .bold()
            .color(Color::DarkBlue);
        
        sheet["A2"]
            .value("报表日期: " + format_date(now))
            .font("微软雅黑", 12)
            .italic();
        
        // 设置列标题
        int header_row = 4;
        sheet["A" + std::to_string(header_row)].value("销售员").bold();
        sheet["B" + std::to_string(header_row)].value("产品类别").bold();
        sheet["C" + std::to_string(header_row)].value("销售数量").bold();
        sheet["D" + std::to_string(header_row)].value("单价").bold();
        sheet["E" + std::to_string(header_row)].value("销售额").bold();
        sheet["F" + std::to_string(header_row)].value("提成").bold();
        
        // 设置标题行格式
        sheet.range("A4:F4")
            .background_color(Color::LightBlue)
            .bold()
            .align(Alignment::Center);
        
        // 销售数据
        std::vector<std::tuple<std::string, std::string, int, double>> sales_data = {
            {"张三", "笔记本", 10, 5999.99},
            {"张三", "手机", 15, 3999.99},
            {"李四", "平板", 8, 2999.99},
            {"李四", "手机", 12, 3999.99},
            {"王五", "笔记本", 6, 5999.99},
            {"王五", "配件", 25, 199.99}
        };
        
        // 写入数据并设置公式
        for (size_t i = 0; i < sales_data.size(); ++i) {
            int row = static_cast<int>(i + 5);  // 从第5行开始
            
            sheet["A" + std::to_string(row)].value(std::get<0>(sales_data[i]));
            sheet["B" + std::to_string(row)].value(std::get<1>(sales_data[i]));
            sheet["C" + std::to_string(row)].value(std::get<2>(sales_data[i]));
            sheet["D" + std::to_string(row)].value(std::get<3>(sales_data[i]))
                .number_format("¥#,##0.00");
            
            // 销售额 = 数量 × 单价
            sheet["E" + std::to_string(row)]
                .formula("C" + std::to_string(row) + "*D" + std::to_string(row))
                .number_format("¥#,##0.00");
            
            // 提成 = 销售额 × 5%
            sheet["F" + std::to_string(row)]
                .formula("E" + std::to_string(row) + "*0.05")
                .number_format("¥#,##0.00");
        }
        
        // 添加汇总行
        int summary_row = static_cast<int>(sales_data.size() + 6);
        sheet["A" + std::to_string(summary_row)].value("总计").bold();
        sheet["C" + std::to_string(summary_row)]
            .formula("SUM(C5:C" + std::to_string(summary_row - 1) + ")")
            .bold();
        sheet["E" + std::to_string(summary_row)]
            .formula("SUM(E5:E" + std::to_string(summary_row - 1) + ")")
            .number_format("¥#,##0.00")
            .bold()
            .background_color(Color::LightGreen);
        sheet["F" + std::to_string(summary_row)]
            .formula("SUM(F5:F" + std::to_string(summary_row - 1) + ")")
            .number_format("¥#,##0.00")
            .bold()
            .background_color(Color::LightGreen);
        
        // 设置数据区域边框
        sheet.range("A4:F" + std::to_string(summary_row))
            .border(BorderType::All, BorderStyle::Thin);
        
        // 保存文件
        workbook.save("monthly_sales_report.xlsx");
        
        std::cout << "✅ 月度销售报表创建成功！" << std::endl;
        std::cout << "文件名: monthly_sales_report.xlsx" << std::endl;
        std::cout << "数据行数: " << sales_data.size() << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 📝 小结

在这个教程中，你学会了：

✅ **创建工作簿**：使用 `Excel::create()` 和构建器模式  
✅ **数据写入**：支持多种数据类型的写入  
✅ **批量操作**：高效的批量数据写入  
✅ **公式设置**：Excel 公式的使用  
✅ **基本格式**：字体、颜色、边框等格式设置  
✅ **文件保存**：多种保存选项和文件管理  

## 🚀 下一步

现在你已经掌握了创建和写入 Excel 文件的技能，接下来可以：

- 学习 [教程 3：现代 C++ 数据处理](03-modern-cpp-data-processing.md)
- 探索 [高级格式化功能](05-advanced-formatting.md)
- 查看 [完整 API 参考](../api-reference/index.md)

---

有问题？查看 [故障排除指南](../guides/troubleshooting.md) 或在 [GitHub Discussions](https://github.com/your-username/TinaKit/discussions) 中讨论。
