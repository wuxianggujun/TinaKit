# TinaKit Examples

这个目录包含了 TinaKit 的核心使用示例，展示不同层次的API使用方法。

## 📋 核心示例

### 🎨 **样式功能测试**
- **`comprehensive_style_test.cpp`** - **高级API样式测试**（⭐ 推荐）
  - ✅ 字体样式（字体名称、大小、粗体、斜体、颜色）
  - ✅ 背景色（各种颜色）
  - ✅ 对齐（水平、垂直对齐）
  - ✅ 数字格式（整数、小数、千分位、百分比、货币）
  - ✅ 边框（各种样式、彩色边框）
  - ✅ 文本换行（`wrap_text()`）
  - ✅ 文本缩进（`indent()` - 支持任意调用顺序）
  - ✅ 行高设置（`row().height()`）
  - ✅ 列宽设置（`set_column_width()`）
  - ✅ 合并单元格（`merge_cells()` / `unmerge_cells()`）
  - ✅ 条件格式（`conditional_format().when_xxx().apply()`）

### 🔧 **底层API演示**
- **`excel_with_styles.cpp`** - **StyleManager底层API演示**
  - 展示如何直接使用 StyleManager
  - 适合需要精细控制样式的场景

### 📖 **基础功能**
- **`main.cpp`** - **基础读写功能测试**
  - Excel文件创建和读取
  - 基本数据操作

## 编译示例

```bash
# 在项目根目录下
mkdir build
cd build
cmake ..
cmake --build . --config Debug

# 示例程序将生成在 build/examples/ 目录下
```

## 示例程序列表

### 1. xlsx_archiver_demo - XLSX 文件操作完整示例

**功能：**
- 创建示例 XLSX 文件
- 读取和列出 XLSX 文件内容
- 修改 XLSX 文件（添加/删除文件）
- 保存修改后的文件

**运行：**
```bash
cd build/examples
./xlsx_archiver_demo
```

**输出文件：**
- `sample.xlsx` - 创建的示例 XLSX 文件
- `modified_sample.xlsx` - 修改后的 XLSX 文件

### 2. xlsx_reader_test - XLSX 文件读取测试

**功能：**
- 分析现有 XLSX 文件的内部结构
- 列出所有内部文件
- 读取关键 XML 文件内容
- 显示文件统计信息

**运行方式：**

1. **自动查找测试文件：**
```bash
cd build/examples
./xlsx_reader_test
```
程序会自动查找以下位置的 XLSX 文件：
- `test.xlsx`
- `sample.xlsx`
- `examples/test.xlsx`
- `../test.xlsx`

2. **指定 XLSX 文件：**
```bash
cd build/examples
./xlsx_reader_test path/to/your/file.xlsx
```

### 3. tinakit_xml_parser_example - XML 解析示例

**功能：**
- 演示 XML 解析功能
- 展示基本的 XML 操作

**运行：**
```bash
cd build/examples
./tinakit_xml_parser_example
```

## XLSX 文件结构说明

XLSX 文件实际上是一个 ZIP 压缩包，包含以下典型结构：

```
xlsx_file.xlsx
├── [Content_Types].xml          # 内容类型定义
├── _rels/
│   └── .rels                    # 主关系文件
├── xl/
│   ├── workbook.xml             # 工作簿定义
│   ├── worksheets/
│   │   ├── sheet1.xml           # 工作表1数据
│   │   └── sheet2.xml           # 工作表2数据（如果存在）
│   ├── sharedStrings.xml        # 共享字符串表（如果存在）
│   ├── styles.xml               # 样式定义（如果存在）
│   └── _rels/
│       └── workbook.xml.rels    # 工作簿关系文件
└── docProps/                    # 文档属性（可选）
    ├── app.xml
    └── core.xml
```

## 使用 TinaKit XlsxArchiver 的基本步骤

### 1. 读取现有 XLSX 文件

```cpp
#include "tinakit/tinakit.hpp"
using namespace tinakit::io;
using namespace tinakit::async;

Task<void> read_xlsx_example() {
    // 打开文件
    auto archiver = co_await XlsxArchiver::open_from_file("example.xlsx");
    
    // 列出所有文件
    auto files = co_await archiver.list_files();
    for (const auto& file : files) {
        std::cout << "文件: " << file << std::endl;
    }
    
    // 读取特定文件内容
    if (co_await archiver.has_file("xl/worksheets/sheet1.xml")) {
        auto content = co_await archiver.read_file("xl/worksheets/sheet1.xml");
        // 处理内容...
    }
}
```

### 2. 创建新的 XLSX 文件

```cpp
Task<void> create_xlsx_example() {
    // 创建内存写入器
    auto archiver = XlsxArchiver::create_in_memory_writer();
    
    // 添加必要的文件
    co_await archiver.add_file("[Content_Types].xml", content_types_data);
    co_await archiver.add_file("_rels/.rels", rels_data);
    co_await archiver.add_file("xl/workbook.xml", workbook_data);
    co_await archiver.add_file("xl/worksheets/sheet1.xml", worksheet_data);
    
    // 保存到文件
    co_await archiver.save_to_file("new_file.xlsx");
}
```

### 3. 修改现有 XLSX 文件

```cpp
Task<void> modify_xlsx_example() {
    // 打开现有文件
    auto archiver = co_await XlsxArchiver::open_from_file("existing.xlsx");
    
    // 添加新文件
    co_await archiver.add_file("custom/data.xml", custom_data);
    
    // 删除文件（如果需要）
    co_await archiver.remove_file("unwanted_file.xml");
    
    // 保存修改
    co_await archiver.save_to_file("modified.xlsx");
}
```

## 注意事项

1. **异步操作：** 所有文件操作都是异步的，需要使用 `co_await` 关键字
2. **文件格式：** 内容需要转换为 `std::vector<std::byte>` 格式
3. **错误处理：** 建议使用 try-catch 块处理可能的异常
4. **内存管理：** XlsxArchiver 会自动管理内部资源，无需手动释放

## 下一步

这些示例展示了 TinaKit XlsxArchiver 的基本用法。在实际的 Excel 处理应用中，您还需要：

1. **XML 解析：** 使用 TinaKit 的 XML 解析功能来解析工作表数据
2. **数据转换：** 将 Excel 的内部格式转换为应用程序可用的数据结构
3. **样式处理：** 处理单元格样式、格式等信息
4. **公式支持：** 解析和处理 Excel 公式

这些高级功能将在后续的开发中逐步添加到 TinaKit 库中。
