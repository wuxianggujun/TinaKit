# TinaKit

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.18+-green.svg)](https://cmake.org/)

> 🚀 现代化的 OpenXML 文件处理 C++ 库

TinaKit 是一个专为现代 C++ 设计的高性能 OpenXML 文件处理库，专注于 Excel (.xlsx)、Word (.docx) 等 Office 文档的读取和写入操作。

## ✨ 核心特性

- **🎯 现代 C++20**：充分利用 concepts、ranges、coroutines 等现代特性
- **🔧 模块化设计**：清晰的架构，易于扩展和维护
- **🚀 高性能**：优化的内存管理和流式处理
- **🔌 插件系统**：支持自定义格式和功能扩展
- **⚡ 异步支持**：大文件处理的非阻塞操作
- **🛡️ 类型安全**：RAII 原则和强类型系统
- **📦 跨平台**：支持 Windows、Linux、macOS

## 🚀 快速开始

### 安装

```bash
# 使用 vcpkg
vcpkg install tinakit

# 或使用 CMake FetchContent
# 在你的 CMakeLists.txt 中添加：
include(FetchContent)
FetchContent_Declare(
    TinaKit
    GIT_REPOSITORY https://github.com/your-username/TinaKit.git
    GIT_TAG main
)
FetchContent_MakeAvailable(TinaKit)
```

### Hello World 示例

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    using namespace tinakit;

    // 读取 Excel 文件
    auto workbook = excel::Workbook::load("example.xlsx");
    auto worksheet = workbook.active_sheet();

    // 读取单元格值
    auto value = worksheet.cell("A1").value<std::string>();
    std::cout << "A1 的值: " << value << std::endl;

    // 写入新值
    worksheet.cell("B1").value("Hello, TinaKit!");

    // 保存文件
    workbook.save("output.xlsx");

    return 0;
}
```

## 📚 文档

- [📖 完整文档](docs/README.md)
- [🏗️ 架构设计](docs/architecture/overview.md)
- [🚀 快速开始](docs/getting-started/installation.md)
- [📝 API 参考](docs/api-reference/index.md)
- [🎓 教程](docs/tutorials/)

## 🏗️ 项目状态

TinaKit 目前处于积极开发阶段。我们正在构建一个强大而灵活的 OpenXML 处理框架。

### 支持的格式

- ✅ Excel (.xlsx) - 基础读写支持
- 🚧 Word (.docx) - 开发中
- 📋 PowerPoint (.pptx) - 计划中

### 路线图

- [x] 核心架构设计
- [x] Excel 基础读取功能
- [ ] Excel 完整写入功能
- [ ] Word 文档支持
- [ ] 图表和图像处理
- [ ] 高级格式化功能

## 🤝 贡献

我们欢迎所有形式的贡献！请查看 [贡献指南](CONTRIBUTING.md) 了解详情。

### 开发环境要求

- C++20 兼容编译器 (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.18+
- vcpkg 或 Conan (推荐)

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

- [OpenXML SDK](https://github.com/OfficeDev/Open-XML-SDK) - 提供了宝贵的参考
- [pugixml](https://pugixml.org/) - XML 解析支持
- [zlib](https://zlib.net/) - 压缩支持

---

⭐ 如果这个项目对你有帮助，请给我们一个 star！
