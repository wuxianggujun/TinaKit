# Hello World 示例

欢迎来到 TinaKit 的世界！本指南将通过几个简单的示例帮助你快速上手。

## 🎯 第一个程序

让我们从最简单的示例开始 - 读取一个 Excel 文件：

### 示例 1: 读取 Excel 文件

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        // 打开 Excel 文件
        auto workbook = Workbook::open("sample.xlsx");
        
        // 获取活动工作表
        auto worksheet = workbook.active_sheet();
        
        // 读取单元格 A1 的值
        auto value = worksheet.cell("A1").value<std::string>();
        
        std::cout << "A1 单元格的值: " << value << std::endl;
        
    } catch (const tinakit::TinaKitException& e) {
        std::cerr << "TinaKit 错误: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "系统错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 示例 2: 创建新的 Excel 文件

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;
    
    try {
        // 创建新的工作簿
        auto workbook = Workbook::create();
        
        // 获取默认工作表
        auto worksheet = workbook.active_sheet();
        
        // 设置工作表名称
        worksheet.title("我的第一个工作表");
        
        // 写入一些数据
        worksheet.cell("A1").value("姓名");
        worksheet.cell("B1").value("年龄");
        worksheet.cell("C1").value("城市");
        
        worksheet.cell("A2").value("张三");
        worksheet.cell("B2").value(25);
        worksheet.cell("C2").value("北京");
        
        worksheet.cell("A3").value("李四");
        worksheet.cell("B3").value(30);
        worksheet.cell("C3").value("上海");
        
        // 保存文件
        workbook.save("hello_world.xlsx");
        
        std::cout << "✅ 文件 'hello_world.xlsx' 创建成功！" << std::endl;
        
    } catch (const tinakit::TinaKitException& e) {
        std::cerr << "TinaKit 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🚀 现代 C++20 特性示例

TinaKit 充分利用了 C++20 的现代特性。以下是一些高级示例：

### 示例 3: 使用 Ranges 处理数据

```cpp
#include <tinakit/tinakit.hpp>
#include <ranges>
#include <iostream>
#include <vector>

int main() {
    using namespace tinakit;
    using namespace std::ranges;
    
    try {
        auto workbook = Workbook::open("data.xlsx");
        auto worksheet = workbook.active_sheet();
        
        // 使用 ranges 处理数据
        auto ages = worksheet.range("B2:B10")
            | views::transform([](const Cell& cell) { 
                return cell.value<int>(); 
            })
            | views::filter([](int age) { 
                return age >= 18; 
            })
            | to<std::vector>();
        
        std::cout << "成年人年龄: ";
        for (auto age : ages) {
            std::cout << age << " ";
        }
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 示例 4: 异步文件处理

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <coroutine>

// 异步读取大文件
tinakit::Task<void> process_large_file(const std::string& filename) {
    using namespace tinakit;
    
    try {
        // 异步打开文件
        auto workbook = co_await Workbook::open_async(filename);
        
        std::cout << "文件打开成功，开始处理..." << std::endl;
        
        // 逐个处理工作表
        for (auto& sheet : workbook.worksheets()) {
            std::cout << "处理工作表: " << sheet.title() << std::endl;
            
            // 异步处理每一行
            co_await sheet.process_rows_async([](const Row& row) {
                // 处理行数据
                std::cout << "处理第 " << row.index() << " 行" << std::endl;
            });
        }
        
        std::cout << "✅ 文件处理完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "处理失败: " << e.what() << std::endl;
    }
}

int main() {
    // 运行异步任务
    auto task = process_large_file("large_file.xlsx");
    task.wait();
    
    return 0;
}
```

### 示例 5: 使用 Concepts 的类型安全

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <concepts>

// 定义一个概念来约束可写入 Excel 的类型
template<typename T>
concept ExcelWritable = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string>;
} || std::integral<T> || std::floating_point<T>;

// 泛型函数，只接受可写入 Excel 的类型
template<ExcelWritable T>
void write_to_cell(tinakit::Cell& cell, const T& value) {
    if constexpr (std::integral<T> || std::floating_point<T>) {
        cell.value(value);
    } else {
        cell.value(value.to_string());
    }
}

// 自定义类型
struct Person {
    std::string name;
    int age;
    
    std::string to_string() const {
        return name + " (" + std::to_string(age) + ")";
    }
};

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Workbook::create();
        auto worksheet = workbook.active_sheet();
        
        // 使用类型安全的写入函数
        write_to_cell(worksheet.cell("A1"), 42);           // int
        write_to_cell(worksheet.cell("A2"), 3.14);         // double
        write_to_cell(worksheet.cell("A3"), Person{"张三", 25}); // 自定义类型
        
        workbook.save("type_safe.xlsx");
        
        std::cout << "✅ 类型安全写入完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔧 构建和运行

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.18)
project(TinaKitHelloWorld)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 TinaKit
find_package(TinaKit CONFIG REQUIRED)

# 创建可执行文件
add_executable(hello_world hello_world.cpp)
add_executable(create_excel create_excel.cpp)
add_executable(ranges_example ranges_example.cpp)
add_executable(async_example async_example.cpp)
add_executable(concepts_example concepts_example.cpp)

# 链接 TinaKit
target_link_libraries(hello_world PRIVATE TinaKit::TinaKit)
target_link_libraries(create_excel PRIVATE TinaKit::TinaKit)
target_link_libraries(ranges_example PRIVATE TinaKit::TinaKit)
target_link_libraries(async_example PRIVATE TinaKit::TinaKit)
target_link_libraries(concepts_example PRIVATE TinaKit::TinaKit)
```

### 构建命令

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release

# 运行示例
./hello_world
./create_excel
./ranges_example
./async_example
./concepts_example
```

## 📝 关键概念

### 1. RAII 资源管理

TinaKit 使用 RAII 原则自动管理资源：

```cpp
{
    auto workbook = Workbook::open("file.xlsx");
    // 文件会在 workbook 析构时自动关闭
} // workbook 在这里自动析构，资源被释放
```

### 2. 异常安全

所有 TinaKit 操作都提供强异常安全保证：

```cpp
try {
    auto workbook = Workbook::open("file.xlsx");
    workbook.save("output.xlsx");  // 如果失败，原文件不会被修改
} catch (const tinakit::IOException& e) {
    // 处理 I/O 错误
} catch (const tinakit::ParseException& e) {
    // 处理解析错误
}
```

### 3. 类型安全

使用强类型系统避免运行时错误：

```cpp
// 编译时类型检查
auto value = cell.value<int>();  // 如果单元格不包含数字，会抛出异常
auto text = cell.value<std::string>();  // 总是成功，会转换为字符串
```

## 🎯 下一步

现在你已经掌握了 TinaKit 的基础用法！接下来可以：

1. 阅读 [详细教程](../tutorials/01-reading-simple-xlsx.md)
2. 查看 [API 参考](../api-reference/index.md)
3. 了解 [架构设计](../architecture/overview.md)
4. 探索 [高级功能](../guides/advanced-usage.md)

---

祝你使用 TinaKit 愉快！🚀
