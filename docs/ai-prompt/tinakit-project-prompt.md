# TinaKit 项目 AI 提示词

## 项目概述

TinaKit 是一个现代化的 C++20 OpenXML 文件处理库，专注于 Excel (.xlsx)、Word (.docx) 等 Office 文档的读取和写入操作。该项目采用 DX-First（开发者体验优先）的设计理念，提供直观、类型安全且高性能的 API。

## 项目基本信息

- **项目名称**: TinaKit
- **编程语言**: C++20
- **主要功能**: OpenXML 文件（Excel、Word）的读写处理
- **设计理念**: 现代 C++、RAII、异步支持、可扩展架构
- **开发状态**: 设计阶段（已完成 API 设计和文档）
- **许可证**: MIT
- **目标平台**: Windows、Linux、macOS

## 核心特性

### ✅ 已设计完成的功能

1. **现代 C++20 设计**
   - 充分利用 concepts、ranges、coroutines
   - RAII 资源管理
   - 强类型安全
   - 异常安全保证

2. **直观的 API 设计**
   ```cpp
   // 理想的用户体验
   auto workbook = Excel::open("data.xlsx");
   auto sheet = workbook["Sheet1"];
   auto value = sheet["A1"].as<std::string>();
   
   // 链式调用样式设置
   sheet["A1"].value("标题").bold().color(Color::Red);
   ```

3. **异步处理支持**
   ```cpp
   // 协程异步操作
   auto workbook = co_await Excel::open_async("large_file.xlsx");
   ```

4. **类型安全的数据访问**
   ```cpp
   // 安全的类型转换
   auto value = cell.try_as<int>();  // 返回 std::optional<int>
   auto value2 = cell.as<int>();     // 可能抛出异常
   ```

5. **现代 C++ 数据处理**
   ```cpp
   // 使用 ranges 进行数据处理
   auto results = sheet.rows(2, 100)
       | std::views::filter([](const Row& row) { return !row.empty(); })
       | std::views::transform([](const Row& row) { return process(row); })
       | std::ranges::to<std::vector>();
   ```

### 🚧 当前项目状态

**已完成**:
- ✅ 完整的 API 设计（头文件）
- ✅ 详细的架构文档
- ✅ 5 个完整的教程系列
- ✅ Dream Code 示例（理想用户体验）
- ✅ 设计理念和决策文档

**未完成**:
- ❌ 实际的 C++ 实现代码
- ❌ 单元测试
- ❌ 构建系统配置
- ❌ CI/CD 流程
- ❌ 性能基准测试

## 项目架构

### 核心组件

1. **Excel 命名空间**: 提供 Excel 文件操作的统一入口
2. **Workbook 类**: Excel 工作簿的顶层抽象
3. **Worksheet 类**: 工作表操作
4. **Cell 类**: 单元格数据和样式管理
5. **Row 类**: 行级操作，支持迭代器
6. **Task 类**: 基于协程的异步操作

### 设计模式

- **PIMPL 模式**: 隐藏实现细节，提供稳定 ABI
- **静态工厂模式**: 清晰的对象创建语义
- **构建器模式**: 支持链式调用
- **回调模式**: 进度和错误通知

## 文档结构

```
docs/
├── README.md                    # 项目总览
├── getting-started/             # 快速开始指南
├── architecture/                # 架构设计文档
├── tutorials/                   # 完整教程系列
├── api-reference/               # API 参考文档
└── guides/                      # 开发者指南
```

## AI 助手能力范围

### ✅ 可以协助的任务

1. **文档编写和完善**
   - 补充 API 文档
   - 编写更多教程
   - 创建示例代码
   - 完善架构说明

2. **API 设计优化**
   - 基于现有设计提出改进建议
   - 设计新的 API 接口
   - 优化方法签名和命名

3. **代码示例创建**
   - 基于 API 设计编写示例代码
   - 创建使用场景演示
   - 编写测试用例设计

4. **项目规划**
   - 制定开发计划
   - 设计测试策略
   - 规划发布流程

5. **技术咨询**
   - C++20 最佳实践建议
   - 性能优化策略
   - 跨平台兼容性建议

### ❌ 不能直接完成的任务

1. **实际代码实现**
   - 无法编写完整的 .cpp 实现文件
   - 无法编译和测试代码
   - 无法验证代码的实际运行效果

2. **系统级操作**
   - 无法配置构建系统
   - 无法设置 CI/CD 流程
   - 无法进行实际的性能测试

3. **外部集成**
   - 无法与真实的 OpenXML 库集成
   - 无法访问文件系统进行测试
   - 无法连接到包管理器

## 设计哲学和原则

### 核心理念

1. **用户体验优先 (UX-First)**
   - 先设计理想的用户代码
   - 再设计 API 接口
   - 最后完善实现细节

2. **现代 C++ 优先**
   - 充分利用 C++20 特性
   - 类型安全和编译时检查
   - 零成本抽象

3. **可扩展性设计**
   - 插件系统支持
   - 格式注册机制
   - 回调和事件系统

### 设计决策示例

- **选择静态工厂而非构造函数**: `Excel::open()` vs `Workbook("file.xlsx")`
- **提供异常和非异常版本**: `as<T>()` vs `try_as<T>()`
- **使用 PIMPL 模式**: 隐藏实现，保证 ABI 稳定性
- **支持链式调用**: 提升代码可读性和流畅性

## 使用示例

### 基础使用

```cpp
#include <tinakit/tinakit.hpp>

int main() {
    using namespace tinakit;
    
    // 读取文件
    auto workbook = Excel::open("data.xlsx");
    auto sheet = workbook["Sheet1"];
    auto value = sheet["A1"].as<std::string>();
    
    // 写入文件
    auto new_workbook = Excel::create();
    auto new_sheet = new_workbook.add_sheet("MySheet");
    new_sheet["A1"].value("Hello, TinaKit!");
    new_workbook.save("output.xlsx");
    
    return 0;
}
```

### 高级使用

```cpp
// 异步处理大文件
Task<void> process_large_file() {
    auto workbook = co_await Excel::open_async("large_file.xlsx");
    
    // 使用 ranges 处理数据
    auto results = workbook[0].rows(2, 1000)
        | std::views::filter([](const Row& row) { return !row.empty(); })
        | std::views::transform([](const Row& row) { return analyze(row); })
        | std::ranges::to<std::vector>();
    
    // 保存结果
    co_await save_results_async(results);
}
```

## 项目目标

### 短期目标
- 完成核心 API 的实现
- 建立完整的测试套件
- 发布第一个可用版本

### 长期目标
- 成为 C++ 生态中最易用的 OpenXML 处理库
- 支持更多 Office 格式（PowerPoint 等）
- 建立活跃的开源社区

## 注意事项

1. **这是一个设计阶段的项目**: 虽然有完整的 API 设计和文档，但实际实现尚未完成
2. **重点关注设计质量**: 当前阶段更注重 API 设计的合理性和用户体验
3. **基于真实需求**: 所有设计都基于实际的使用场景和用户反馈
4. **持续演进**: API 设计可能会根据实现过程中的发现进行调整

---

这个提示词可以帮助其他 AI 系统快速理解 TinaKit 项目的现状、能力范围和设计理念，从而提供更准确和有用的协助。
