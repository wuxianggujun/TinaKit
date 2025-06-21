# TinaKit API 参考文档

## 📖 概述

TinaKit 提供了现代化的 C++20 API 来处理 OpenXML 文件。本文档详细介绍了所有可用的类、方法和功能。

## 🏗️ 核心架构

TinaKit 采用 PIMPL（Pointer to Implementation）设计模式，提供稳定的 ABI 和清晰的接口分离。

### 命名空间结构

```cpp
namespace tinakit {
    namespace excel {
        // Excel 相关功能
    }
    namespace core {
        // 核心功能和类型
    }
    namespace internal {
        // 内部实现（用户不应直接使用）
    }
}
```

## 📚 主要类参考

### 1. Workbook 类

Excel 工作簿的主要接口。

#### 静态工厂方法

```cpp
// 创建新的工作簿
static Workbook create();

// 从文件打开工作簿
static Workbook open(const std::filesystem::path& file_path);

// 注意：异步API暂未实现
// static async::Task<Workbook> open_async(const std::filesystem::path& file_path);
```

#### 工作表管理

```cpp
// 获取活动工作表
Worksheet active_sheet() const;

// 按名称获取工作表
Worksheet get_worksheet(const std::string& name) const;

// 按索引获取工作表
Worksheet get_worksheet(std::size_t index) const;

// 创建新工作表
Worksheet create_worksheet(const std::string& name);

// 删除工作表
void remove_worksheet(const std::string& name);

// 重命名工作表（通过Worksheet对象）
// worksheet.set_name(new_name);

// 获取所有工作表名称
std::vector<std::string> worksheet_names() const;

// 获取工作表数量
std::size_t worksheet_count() const noexcept;

// 检查工作表是否存在
bool has_worksheet(const std::string& name) const;
```

#### 文件操作

```cpp
// 保存到指定路径
void save(const std::filesystem::path& file_path = {});

// 异步保存
async::Task<void> save_async(const std::filesystem::path& file_path = {});

// 获取文件路径
const std::filesystem::path& file_path() const noexcept;

// 检查是否有未保存的更改
bool has_unsaved_changes() const noexcept;

// 获取文件大小
std::size_t file_size() const;
```

#### 操作符重载

```cpp
// 按名称访问工作表
Worksheet operator[](const std::string& name) const;

// 按索引访问工作表
Worksheet operator[](std::size_t index) const;
```

### 2. Worksheet 类

工作表操作的主要接口。

#### 单元格访问

```cpp
// 获取单元格（按地址）
Cell cell(const std::string& address);
const Cell cell(const std::string& address) const;

// 获取单元格（按行列）
Cell cell(std::size_t row, std::size_t column);
const Cell cell(std::size_t row, std::size_t column) const;

// 操作符重载
Cell operator[](const std::string& address);
const Cell operator[](const std::string& address) const;
```

#### 行操作

```cpp
// 获取行
Row row(std::size_t index);
const Row& row(std::size_t index) const;

// 插入行
void insert_row(std::size_t index, std::size_t count = 1);

// 删除行
void delete_row(std::size_t index, std::size_t count = 1);

// 获取已使用的行数
std::size_t used_row_count() const;
```

#### 列操作

```cpp
// 插入列
void insert_column(std::size_t index, std::size_t count = 1);

// 删除列
void delete_column(std::size_t index, std::size_t count = 1);

// 获取已使用的列数
std::size_t used_column_count() const;
```

#### 范围操作

```cpp
// 获取范围
Range range(const std::string& range_address);

// 获取已使用的范围
Range used_range() const;

// 选择范围
void select_range(const std::string& range_address);
```

#### 查找和替换

```cpp
// 查找值
std::vector<std::string> find(const std::string& value) const;

// 替换值
std::size_t replace(const std::string& old_value, const std::string& new_value);
```

#### 工作表属性

```cpp
// 获取工作表名称
const std::string& name() const;

// 设置工作表名称
void set_name(const std::string& name);

// 检查工作表是否可见
bool visible() const;

// 设置工作表可见性
void set_visible(bool visible);

// 检查工作表是否受保护
bool protected_sheet() const;

// 保护工作表
void protect_sheet(const std::string& password = "");

// 取消保护工作表
void unprotect_sheet(const std::string& password = "");
```

#### 条件格式

```cpp
// 创建条件格式构建器
ConditionalFormatBuilder conditional_format(const std::string& range_str);

// 添加条件格式
void add_conditional_format(const ConditionalFormat& format);

// 获取所有条件格式
const std::vector<ConditionalFormat>& get_conditional_formats() const;
```

### 3. Row 类

Excel 行的抽象，支持迭代器模式和批量操作。

```cpp
class Row {
public:
    // 单元格访问
    Cell operator[](std::size_t column_index) const;
    Cell operator[](const std::string& column_name) const;

    // 行属性
    std::size_t index() const noexcept;
    double height() const noexcept;
    void set_height(double height);
    Row& height(double height);  // 链式调用版本

    // 状态查询
    bool empty() const;
    std::size_t size() const;
    bool valid() const noexcept;

    // 批量操作
    void set_values(const std::vector<Cell::CellValue>& values, std::size_t start_column = 1);
    std::vector<Cell::CellValue> get_values(std::size_t start_column = 1, std::size_t count = 0) const;
    void clear();

    // 迭代器支持
    iterator begin() const;
    iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
};
```

### 4. Cell 类

单元格操作的主要接口。

#### 值操作

```cpp
// 设置值（支持多种类型）
template<typename T>
Cell& value(const T& val);

// 获取值（类型安全）
template<typename T>
T as() const;

// 尝试获取值（不抛异常）
template<typename T>
std::optional<T> try_as() const noexcept;

// 获取字符串表示
std::string to_string() const;

// 检查是否为空
bool empty() const;

// 清空单元格
Cell& clear();
```

#### 公式操作

```cpp
// 设置公式
Cell& formula(const std::string& formula);

// 获取公式
std::optional<std::string> formula() const;

// 检查是否包含公式
bool has_formula() const;
```

#### 样式操作

```cpp
// 字体设置
Cell& font(const std::string& font_name, double size = 11.0);
Cell& bold(bool bold = true);
Cell& italic(bool italic = true);
Cell& color(const Color& color);

// 背景和填充
Cell& background_color(const Color& color);

// 对齐方式
Cell& align(const Alignment& alignment);

// 边框
Cell& border(BorderType border_type, BorderStyle style = BorderStyle::Thin);
Cell& border(BorderType border_type, BorderStyle style, const Color& color);

// 数字格式
Cell& number_format(const std::string& format_code);

// 文本换行
Cell& wrap_text(bool wrap = true);

// 缩进
Cell& indent(int indent_level);

// 应用样式模板
Cell& style(const StyleTemplate& style_template);

// 样式ID操作
Cell& style_id(std::uint32_t style_id);
std::uint32_t style_id() const;
bool has_custom_style() const;
```

#### 位置信息

```cpp
// 获取行号（1-based）
std::size_t row() const;

// 获取列号（1-based）
std::size_t column() const;

// 获取地址字符串（如 "A1"）
std::string address() const;
```

### 4. Range 类

范围操作的主要接口。

#### 构造和基本信息

```cpp
// 构造函数
Range(std::shared_ptr<internal::workbook_impl> workbook_impl,
      const std::string& sheet_name,
      const std::string& range_address);

// 获取范围地址
const std::string& address() const;

// 获取工作表名称
const std::string& sheet_name() const;

// 检查范围是否有效
bool valid() const;

// 获取范围大小
std::pair<std::size_t, std::size_t> size() const; // {rows, columns}
```

#### 批量数据操作

```cpp
// 批量设置相同值
template<typename T>
Range& set_value(const T& value);

// 批量设置不同值
template<typename T>
Range& set_values(const std::vector<std::vector<T>>& values);

// 批量获取值
template<typename T>
std::vector<std::vector<T>> get_values() const;
```

#### 批量样式操作

```cpp
// 应用样式模板
Range& set_style(const StyleTemplate& style_template);

// 应用样式ID
Range& set_style(std::uint32_t style_id);

// 清空范围
Range& clear();
```

#### 迭代器支持

```cpp
// 开始迭代器
RangeIterator begin();
const RangeIterator begin() const;

// 结束迭代器
RangeIterator end();
const RangeIterator end() const;
```

## 🎨 样式系统

### Color 类

```cpp
// 预定义颜色
static const Color Black;
static const Color White;
static const Color Red;
static const Color Green;
static const Color Blue;
static const Color Yellow;
static const Color Cyan;
static const Color Magenta;
static const Color LightGray;
static const Color DarkGray;

// 构造函数
Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255);

// 从十六进制字符串创建
static Color from_hex(const std::string& hex);

// 获取RGB值
std::uint8_t red() const;
std::uint8_t green() const;
std::uint8_t blue() const;
std::uint8_t alpha() const;

// 转换为十六进制字符串
std::string to_hex() const;
```

### Alignment 类

```cpp
// 水平对齐
enum class Horizontal {
    Left, Center, Right, Justify, Fill
};

// 垂直对齐
enum class Vertical {
    Top, Center, Bottom, Justify
};

// 构造函数
Alignment(Horizontal h = Horizontal::Left, Vertical v = Vertical::Bottom);

// 属性
Horizontal horizontal;
Vertical vertical;
bool wrap_text = false;
int indent = 0;
```

### StyleTemplate 类

```cpp
// 链式调用样式构建
StyleTemplate& font(const std::string& name, double size = 11.0);
StyleTemplate& bold(bool bold = true);
StyleTemplate& italic(bool italic = true);
StyleTemplate& color(const Color& color);
StyleTemplate& background_color(const Color& color);
StyleTemplate& align_horizontal(Alignment::Horizontal alignment);
StyleTemplate& align_vertical(Alignment::Vertical alignment);
StyleTemplate& border(BorderType type, BorderStyle style = BorderStyle::Thin);
StyleTemplate& number_format(const std::string& format);
```

## 🚀 异步操作

### Task 类

```cpp
template<typename T = void>
class Task {
public:
    // 等待任务完成
    T get();
    
    // 检查任务是否完成
    bool ready() const;
    
    // 取消任务
    void cancel();
    
    // 检查任务是否被取消
    bool cancelled() const;
};

// 同步等待异步任务
template<typename T>
T sync_wait(Task<T> task);
```

## ⚠️ 异常处理

TinaKit 使用统一的异常体系，所有异常都继承自 `TinaKitException`：

```cpp
// 基础异常类
class TinaKitException : public std::exception;

// 具体异常类型
class FileNotFoundException : public TinaKitException;
class CorruptedFileException : public TinaKitException;
class WorksheetNotFoundException : public TinaKitException;
class DuplicateWorksheetNameException : public TinaKitException;
class InvalidCellAddressException : public TinaKitException;
class TypeConversionException : public TinaKitException;
```

## 📝 使用示例

详细的使用示例请参考：
- [快速开始指南](../getting-started/hello-world.md)
- [教程系列](../tutorials/)
- [示例程序](../../examples/)

## 🔗 相关文档

- [架构设计](../architecture/overview.md)
- [性能优化](../performance_optimizations.md)
- [构建指南](../getting-started/compilation.md)
