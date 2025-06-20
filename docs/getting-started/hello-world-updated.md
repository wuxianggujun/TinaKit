# 你的第一个 TinaKit 程序

欢迎来到 TinaKit 的世界！本指南将带你创建第一个使用 TinaKit 的程序。

## 🎯 学习目标

通过本教程，你将学会：
- 创建和保存 Excel 文件
- 读取和写入单元格数据
- 应用基本的格式化
- 处理错误和异常
- 使用现代 C++20 特性

## 📋 前置条件

在开始之前，请确保你已经：
- 安装了 TinaKit 库（参见 [安装指南](installation.md)）
- 配置了 C++20 兼容的开发环境
- 熟悉基本的 C++ 语法

## 🚀 示例 1: 创建你的第一个 Excel 文件

让我们从最简单的例子开始：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <filesystem>

int main() {
    using namespace tinakit;
    
    try {
        // 创建一个新的工作簿
        auto workbook = excel::Workbook::create();
        
        // 获取默认工作表
        auto worksheet = workbook.active_sheet();
        
        // 写入一些数据
        worksheet.cell("A1").value("Hello, TinaKit!");
        worksheet.cell("B1").value(42);
        worksheet.cell("C1").value(3.14159);
        worksheet.cell("D1").value(true);
        
        // 保存文件
        workbook.save("my_first_file.xlsx");
        
        std::cout << "✅ 文件创建成功！" << std::endl;
        std::cout << "📁 文件位置: " << std::filesystem::absolute("my_first_file.xlsx") << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "❌ TinaKit 错误: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "❌ 系统错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 🔍 代码解析

1. **包含头文件**: `#include <tinakit/tinakit.hpp>` 包含了所有必要的 TinaKit 功能
2. **创建工作簿**: `excel::Workbook::create()` 创建一个新的空白工作簿
3. **获取工作表**: `active_sheet()` 获取默认的活动工作表（通常名为 "Sheet1"）
4. **写入数据**: 使用 `cell().value()` 方法写入不同类型的数据
5. **保存文件**: `save()` 方法将工作簿保存为 Excel 文件
6. **异常处理**: 使用 TinaKit 特定的异常类型进行更精确的错误处理

## 📖 示例 2: 读取现有的 Excel 文件

现在让我们学习如何读取文件：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <filesystem>

int main() {
    using namespace tinakit;
    
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists("my_first_file.xlsx")) {
            std::cerr << "❌ 文件不存在，请先运行示例1创建文件" << std::endl;
            return 1;
        }
        
        // 打开现有文件
        auto workbook = excel::Workbook::load("my_first_file.xlsx");
        auto worksheet = workbook.active_sheet();
        
        std::cout << "📊 工作簿信息:" << std::endl;
        std::cout << "  工作表数量: " << workbook.worksheet_count() << std::endl;
        std::cout << "  活动工作表: " << worksheet.name() << std::endl;
        
        // 读取数据（类型安全）
        auto text_value = worksheet.cell("A1").as<std::string>();
        auto int_value = worksheet.cell("B1").as<int>();
        auto double_value = worksheet.cell("C1").as<double>();
        auto bool_value = worksheet.cell("D1").as<bool>();
        
        // 输出结果
        std::cout << "\n📋 单元格数据:" << std::endl;
        std::cout << "  A1 (文本): " << text_value << std::endl;
        std::cout << "  B1 (整数): " << int_value << std::endl;
        std::cout << "  C1 (浮点): " << double_value << std::endl;
        std::cout << "  D1 (布尔): " << (bool_value ? "true" : "false") << std::endl;
        
        // 使用 try_as 进行安全类型转换
        auto safe_value = worksheet.cell("B1").try_as<std::string>();
        if (safe_value) {
            std::cout << "  B1 (转为字符串): " << *safe_value << std::endl;
        }
        
    } catch (const FileNotFoundException& e) {
        std::cerr << "❌ 文件未找到: " << e.what() << std::endl;
        return 1;
    } catch (const TypeConversionException& e) {
        std::cerr << "❌ 类型转换错误: " << e.what() << std::endl;
        return 1;
    } catch (const TinaKitException& e) {
        std::cerr << "❌ TinaKit 错误: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "❌ 系统错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🎨 示例 3: 添加样式和格式化

让我们学习如何美化你的 Excel 文件：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = excel::Workbook::create();
        auto worksheet = workbook.active_sheet();
        
        // 创建标题行
        worksheet.cell("A1").value("产品名称")
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
            
        worksheet.cell("B1").value("价格")
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
            
        worksheet.cell("C1").value("库存")
            .bold()
            .color(Color::White)
            .background_color(Color::Blue)
            .align(Alignment(Alignment::Horizontal::Center, Alignment::Vertical::Center));
        
        // 添加数据行
        worksheet.cell("A2").value("笔记本电脑");
        worksheet.cell("B2").value(5999.99).number_format("¥#,##0.00");
        worksheet.cell("C2").value(50);
        
        worksheet.cell("A3").value("无线鼠标");
        worksheet.cell("B3").value(199.00).number_format("¥#,##0.00");
        worksheet.cell("C3").value(200);
        
        // 为低库存商品添加警告颜色
        if (worksheet.cell("C2").as<int>() < 100) {
            worksheet.cell("C2").background_color(Color::Yellow);
        }
        
        workbook.save("styled_example.xlsx");
        std::cout << "✅ 带样式的文件创建成功！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🚀 示例 4: 使用 Range 进行批量操作

TinaKit 支持高效的批量操作：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = excel::Workbook::create();
        auto worksheet = workbook.active_sheet();
        
        // 批量设置标题
        auto header_range = worksheet.range("A1:E1");
        std::vector<std::string> headers = {"ID", "姓名", "年龄", "部门", "薪资"};
        
        // 逐个设置标题（Range批量设置values功能待实现）
        worksheet.cell("A1").value("ID");
        worksheet.cell("B1").value("姓名");
        worksheet.cell("C1").value("年龄");
        worksheet.cell("D1").value("部门");
        worksheet.cell("E1").value("薪资");
        
        // 批量应用样式到标题行
        header_range.set_style(StyleTemplate()
            .bold()
            .background_color(Color::LightGray)
            .align_horizontal(Alignment::Horizontal::Center));
        
        // 添加一些数据
        for (int i = 2; i <= 6; ++i) {
            worksheet.cell("A" + std::to_string(i)).value(i - 1);
            worksheet.cell("B" + std::to_string(i)).value("员工" + std::to_string(i - 1));
            worksheet.cell("C" + std::to_string(i)).value(25 + i);
            worksheet.cell("D" + std::to_string(i)).value("部门" + std::to_string((i % 3) + 1));
            worksheet.cell("E" + std::to_string(i)).value(5000 + i * 500);
        }
        
        // 为薪资列应用货币格式
        auto salary_range = worksheet.range("E2:E6");
        salary_range.set_style(StyleTemplate().number_format("¥#,##0"));
        
        workbook.save("batch_example.xlsx");
        std::cout << "✅ 批量操作示例创建成功！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔧 示例 5: 工作表管理

学习如何管理多个工作表：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = excel::Workbook::create();
        
        // 创建多个工作表
        auto sales_sheet = workbook.create_worksheet("销售数据");
        auto summary_sheet = workbook.create_worksheet("汇总报表");
        
        // 在销售数据表中添加数据
        sales_sheet.cell("A1").value("日期");
        sales_sheet.cell("B1").value("销售额");
        sales_sheet.cell("A2").value("2024-01-01");
        sales_sheet.cell("B2").value(10000);
        
        // 在汇总报表中添加数据
        summary_sheet.cell("A1").value("总销售额");
        summary_sheet.cell("B1").value(10000);
        
        // 列出所有工作表
        std::cout << "📊 工作簿包含以下工作表:" << std::endl;
        for (const auto& name : workbook.worksheet_names()) {
            std::cout << "  - " << name << std::endl;
        }
        
        workbook.save("multi_sheet_example.xlsx");
        std::cout << "✅ 多工作表示例创建成功！" << std::endl;
        
    } catch (const DuplicateWorksheetNameException& e) {
        std::cerr << "❌ 工作表名称重复: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🎓 下一步

恭喜！你已经学会了 TinaKit 的基础用法。接下来你可以：

1. 查看 [API 参考文档](../api-reference/index.md) 了解更多功能
2. 阅读 [教程系列](../tutorials/) 学习高级特性
3. 查看 [示例程序](../../examples/) 获取更多灵感
4. 了解 [架构设计](../architecture/overview.md) 深入理解 TinaKit

## 💡 最佳实践

- 始终使用异常处理来捕获可能的错误
- 使用类型安全的 `as<T>()` 方法读取数据
- 对于不确定的类型转换，使用 `try_as<T>()` 方法
- 利用 Range 操作进行批量处理以提高性能
- 使用 StyleTemplate 创建可重用的样式

## 🔗 相关资源

- [安装指南](installation.md)
- [编译指南](compilation.md)
- [API 参考](../api-reference/index.md)
- [GitHub 仓库](https://github.com/your-username/TinaKit)
