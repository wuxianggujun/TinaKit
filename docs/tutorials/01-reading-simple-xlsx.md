# 教程 1：读取简单的 Excel 文件

欢迎来到 TinaKit 的第一个教程！在这个教程中，你将学会如何使用 TinaKit 读取 Excel 文件的基本操作。

## 🎯 学习目标

完成本教程后，你将能够：
- 打开和读取 Excel 文件
- 访问工作表和单元格
- 获取单元格的值和类型
- 处理基本的错误情况

## 📋 前置条件

- 已安装 TinaKit（参见 [安装指南](../getting-started/installation.md)）
- 支持 C++20 的编译器
- 基本的 C++ 知识

## 📁 准备示例文件

首先，让我们创建一个简单的 Excel 文件用于测试。你可以用 Excel 或其他工具创建一个名为 `sample.xlsx` 的文件，内容如下：

| A | B | C |
|---|---|---|
| 姓名 | 年龄 | 城市 |
| 张三 | 25 | 北京 |
| 李四 | 30 | 上海 |
| 王五 | 28 | 广州 |

## 🚀 第一个程序：打开文件

让我们从最简单的例子开始：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        // 打开 Excel 文件
        auto workbook = Excel::open("sample.xlsx");
        
        std::cout << "✅ 文件打开成功！" << std::endl;
        std::cout << "工作表数量: " << workbook.sheet_count() << std::endl;
        
    } catch (const FileNotFoundException& e) {
        std::cerr << "❌ 文件未找到: " << e.file_path() << std::endl;
        return 1;
    } catch (const TinaKitException& e) {
        std::cerr << "❌ TinaKit 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 🔍 代码解析

1. **包含头文件**：`#include <tinakit/tinakit.hpp>` 包含了所有必要的 TinaKit 功能
2. **命名空间**：`using namespace tinakit;` 简化代码书写
3. **打开文件**：`Excel::open("sample.xlsx")` 使用静态工厂方法打开文件
4. **错误处理**：使用异常处理机制捕获可能的错误

## 📊 访问工作表

现在让我们学习如何访问工作表：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::open("sample.xlsx");
        
        // 方法 1：访问活动工作表（通常是第一个）
        auto& active_sheet = workbook.active_sheet();
        std::cout << "活动工作表: " << active_sheet.title() << std::endl;
        
        // 方法 2：按索引访问（从 0 开始）
        auto& first_sheet = workbook[0];
        std::cout << "第一个工作表: " << first_sheet.title() << std::endl;
        
        // 方法 3：按名称访问
        try {
            auto& sheet_by_name = workbook["Sheet1"];
            std::cout << "按名称访问: " << sheet_by_name.title() << std::endl;
        } catch (const WorksheetNotFoundException& e) {
            std::cout << "工作表 'Sheet1' 不存在" << std::endl;
        }
        
        // 列出所有工作表
        std::cout << "\n所有工作表:" << std::endl;
        auto sheets = workbook.worksheets();
        for (size_t i = 0; i < sheets.size(); ++i) {
            std::cout << "  " << i << ": " << sheets[i].get().title() << std::endl;
        }
        
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 📱 读取单元格数据

这是最核心的功能 - 读取单元格的值：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <string>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::open("sample.xlsx");
        auto& sheet = workbook.active_sheet();
        
        // 方法 1：直接读取为字符串
        std::cout << "=== 读取为字符串 ===" << std::endl;
        std::cout << "A1: " << sheet["A1"].as<std::string>() << std::endl;
        std::cout << "B1: " << sheet["B1"].as<std::string>() << std::endl;
        std::cout << "C1: " << sheet["C1"].as<std::string>() << std::endl;
        
        // 方法 2：类型安全的读取
        std::cout << "\n=== 类型安全读取 ===" << std::endl;
        
        // 读取姓名（字符串）
        auto name = sheet["A2"].as<std::string>();
        std::cout << "姓名: " << name << std::endl;
        
        // 读取年龄（数字）
        auto age = sheet["B2"].as<int>();
        std::cout << "年龄: " << age << std::endl;
        
        // 读取城市（字符串）
        auto city = sheet["C2"].as<std::string>();
        std::cout << "城市: " << city << std::endl;
        
        // 方法 3：安全的类型转换（不抛出异常）
        std::cout << "\n=== 安全类型转换 ===" << std::endl;
        
        auto maybe_age = sheet["B3"].try_as<int>();
        if (maybe_age) {
            std::cout << "B3 的年龄: " << *maybe_age << std::endl;
        } else {
            std::cout << "B3 不是有效的整数" << std::endl;
        }
        
        // 检查单元格是否为空
        if (sheet["D1"].empty()) {
            std::cout << "D1 单元格为空" << std::endl;
        }
        
    } catch (const TypeConversionException& e) {
        std::cerr << "类型转换错误: " << e.what() << std::endl;
        std::cerr << "从 " << e.from_type() << " 转换到 " << e.to_type() << std::endl;
        return 1;
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔄 遍历数据

让我们学习如何遍历多个单元格：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::open("sample.xlsx");
        auto& sheet = workbook.active_sheet();
        
        std::cout << "=== 遍历数据行 ===" << std::endl;
        
        // 方法 1：逐个访问单元格
        for (int row = 2; row <= 4; ++row) {  // 从第2行到第4行
            auto name_addr = "A" + std::to_string(row);
            auto age_addr = "B" + std::to_string(row);
            auto city_addr = "C" + std::to_string(row);
            
            auto name = sheet[name_addr].as<std::string>();
            auto age = sheet[age_addr].as<int>();
            auto city = sheet[city_addr].as<std::string>();
            
            std::cout << "第" << row << "行: " << name 
                      << ", " << age << "岁, " << city << std::endl;
        }
        
        // 方法 2：使用范围访问
        std::cout << "\n=== 使用范围访问 ===" << std::endl;
        
        auto data_range = sheet.range("A2:C4");
        for (auto& cell : data_range) {
            if (!cell.empty()) {
                std::cout << cell.address() << ": " 
                          << cell.as<std::string>() << std::endl;
            }
        }
        
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🛡️ 错误处理最佳实践

在实际应用中，良好的错误处理非常重要：

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <filesystem>

bool read_excel_safely(const std::string& filename) {
    using namespace tinakit;
    
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists(filename)) {
            std::cerr << "文件不存在: " << filename << std::endl;
            return false;
        }
        
        auto workbook = Excel::open(filename);
        
        // 检查是否有工作表
        if (workbook.sheet_count() == 0) {
            std::cerr << "文件中没有工作表" << std::endl;
            return false;
        }
        
        auto& sheet = workbook.active_sheet();
        
        // 安全地读取数据
        auto title_cell = sheet["A1"];
        if (!title_cell.empty()) {
            std::cout << "标题: " << title_cell.as<std::string>() << std::endl;
        }
        
        // 使用 try_as 避免类型转换异常
        for (int row = 2; row <= 10; ++row) {
            auto name_addr = "A" + std::to_string(row);
            auto age_addr = "B" + std::to_string(row);
            
            auto name_cell = sheet[name_addr];
            if (name_cell.empty()) break;  // 遇到空行就停止
            
            auto name = name_cell.as<std::string>();
            auto age = sheet[age_addr].try_as<int>();
            
            std::cout << "姓名: " << name;
            if (age) {
                std::cout << ", 年龄: " << *age;
            } else {
                std::cout << ", 年龄: 无效数据";
            }
            std::cout << std::endl;
        }
        
        return true;
        
    } catch (const FileNotFoundException& e) {
        std::cerr << "文件未找到: " << e.file_path() << std::endl;
    } catch (const CorruptedFileException& e) {
        std::cerr << "文件损坏: " << e.file_path() << std::endl;
    } catch (const ParseException& e) {
        std::cerr << "解析错误: " << e.what() 
                  << " 位置: " << e.line() << ":" << e.column() << std::endl;
    } catch (const TinaKitException& e) {
        std::cerr << "TinaKit 错误: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "系统错误: " << e.what() << std::endl;
    }
    
    return false;
}

int main() {
    if (read_excel_safely("sample.xlsx")) {
        std::cout << "✅ 文件读取成功！" << std::endl;
    } else {
        std::cout << "❌ 文件读取失败！" << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔧 构建和运行

创建 `CMakeLists.txt` 文件：

```cmake
cmake_minimum_required(VERSION 3.18)
project(TinaKitTutorial01)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(TinaKit CONFIG REQUIRED)

add_executable(read_excel read_excel.cpp)
target_link_libraries(read_excel PRIVATE TinaKit::TinaKit)
```

构建和运行：

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./read_excel  # Linux/macOS
# 或
.\read_excel.exe  # Windows
```

## 📝 小结

在这个教程中，你学会了：

✅ **基本文件操作**：使用 `Excel::open()` 打开文件  
✅ **工作表访问**：按索引、名称或获取活动工作表  
✅ **单元格读取**：类型安全的 `as<T>()` 和安全的 `try_as<T>()`  
✅ **数据遍历**：逐个访问和范围访问  
✅ **错误处理**：完整的异常处理机制  

## 🚀 下一步

现在你已经掌握了读取 Excel 文件的基础知识，接下来可以：

- 学习 [教程 2：创建和写入 Excel 文件](02-writing-basic-xlsx.md)
- 查看 [API 参考文档](../api-reference/index.md)
- 探索更多 [示例代码](../../examples/)

---

有问题？查看 [常见问题](../reference/faq.md) 或在 [GitHub Issues](https://github.com/your-username/TinaKit/issues) 中提问。
