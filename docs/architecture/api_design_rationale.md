# TinaKit API 设计理念与决策

本文档详细说明了 TinaKit API 设计背后的思考过程和决策理由，基于实际的代码设计而非理论模板。

## 🎯 设计驱动原则

### 1. 用户体验优先 (UX-First Design)

我们的设计过程始于 `examples/dream_code.cpp`，这个文件展示了我们希望用户能够编写的最理想的代码。

**设计决策**：
```cpp
// 理想：直观的文件操作
auto workbook = Excel::open("data.xlsx");
auto sheet = workbook["Sheet1"];
auto value = sheet["A1"].as<std::string>();
```

**理由**：
- **直观性**：`workbook["Sheet1"]` 比 `workbook.get_worksheet("Sheet1")` 更自然
- **一致性**：Excel、Word 都使用相似的访问模式
- **简洁性**：减少冗余的方法调用

### 2. 类型安全与错误处理

**设计决策**：
```cpp
// 类型安全的值获取
template<typename T> T as() const;                    // 可能抛出异常
template<typename T> std::optional<T> try_as() const; // 不抛出异常

// 使用示例
auto value = cell.try_as<int>();
if (value) {
    std::cout << "整数值: " << *value << std::endl;
} else {
    std::cout << "不是有效的整数" << std::endl;
}
```

**理由**：
- **选择权**：用户可以选择异常版本或非异常版本
- **性能**：`try_as` 避免异常开销，适合性能敏感场景
- **清晰性**：方法名明确表达了行为差异

## 🏗️ 架构设计决策

### 1. PIMPL 模式的选择

**设计决策**：
```cpp
class Workbook {
private:
    std::unique_ptr<WorkbookImpl> impl_;  // 隐藏实现
};
```

**理由**：
- **ABI 稳定性**：实现变更不影响公共接口
- **编译时间**：减少头文件依赖，加快编译
- **封装性**：完全隐藏 OpenXML 解析细节

### 2. 静态工厂 vs 构造函数

**设计决策**：
```cpp
// 选择：静态工厂
namespace Excel {
    Workbook open(const std::filesystem::path& path);
    Workbook create();
}

// 而非：直接构造函数
// Workbook workbook("file.xlsx");  // 不采用
```

**理由**：
- **语义清晰**：`Excel::open()` vs `Excel::create()` 意图明确
- **错误处理**：工厂方法可以返回不同的错误类型
- **扩展性**：未来可以添加更多创建方式而不破坏接口

### 3. 链式调用的实现

**设计决策**：
```cpp
Cell& bold(bool bold = true) {
    // 设置粗体
    return *this;  // 返回自身引用
}

// 支持链式调用
sheet["A1"].value("标题").bold().color(Color::Red);
```

**理由**：
- **流畅性**：符合现代 C++ 的流畅接口设计
- **可读性**：样式设置代码更接近自然语言
- **效率**：避免临时对象创建

## 🚀 现代 C++20 特性应用

### 1. Concepts 的使用

**设计决策**：
```cpp
template<typename T>
concept CellValue = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string>;
} || std::integral<T> || std::floating_point<T>;

template<CellValue T>
void set_cell_value(Cell& cell, const T& value);
```

**理由**：
- **编译时检查**：在模板实例化时就能发现类型错误
- **更好的错误信息**：编译器能提供更清晰的错误提示
- **自文档化**：概念名称就是接口文档

### 2. Coroutines 的异步设计

**设计决策**：
```cpp
Task<Workbook> open_async(const std::filesystem::path& path);

// 使用示例
auto workbook = co_await Excel::open_async("large_file.xlsx");
```

**理由**：
- **非阻塞**：大文件处理不会阻塞 UI 线程
- **自然语法**：`co_await` 比回调函数更直观
- **组合性**：协程可以轻松组合复杂的异步操作

### 3. Ranges 的集成

**设计决策**：
```cpp
class Row {
public:
    iterator begin();
    iterator end();
    // ... 完整的迭代器支持
};

// 支持 std::ranges
template<>
inline constexpr bool std::ranges::enable_borrowed_range<tinakit::Row> = true;
```

**理由**：
- **标准兼容**：与 STL 算法无缝集成
- **函数式编程**：支持声明式的数据处理
- **性能**：延迟求值，避免不必要的计算

## 🎨 用户体验设计

### 1. 错误处理的层次化

**设计决策**：
```cpp
// 异常层次结构
class TinaKitException : public std::exception {};
class FileNotFoundException : public TinaKitException {};
class ParseException : public TinaKitException {};
class TypeConversionException : public TinaKitException {};
```

**理由**：
- **精确捕获**：用户可以选择捕获特定类型的错误
- **上下文信息**：每个异常类型提供相关的上下文数据
- **向后兼容**：基类捕获保证不会遗漏新的异常类型

### 2. 进度回调的设计

**设计决策**：
```cpp
using ProgressCallback = std::function<void(double progress)>;

void on_progress(ProgressCallback callback);

// 使用示例
workbook.on_progress([](double progress) {
    std::cout << std::format("进度: {:.1f}%\n", progress * 100);
});
```

**理由**：
- **灵活性**：支持 lambda、函数指针、函数对象
- **简单性**：单一回调函数，避免复杂的观察者模式
- **性能**：`std::function` 的开销在 I/O 操作中可以忽略

## 📊 性能考虑

### 1. 内存管理策略

**设计决策**：
```cpp
// 使用 unique_ptr 管理资源
class Workbook {
private:
    std::unique_ptr<WorkbookImpl> impl_;
};

// 移动语义支持
Workbook(Workbook&& other) noexcept;
Workbook& operator=(Workbook&& other) noexcept;
```

**理由**：
- **零开销**：RAII 确保资源自动释放
- **移动优化**：避免不必要的拷贝操作
- **异常安全**：强异常安全保证

### 2. 延迟加载设计

**设计决策**：
```cpp
// 工作表按需加载
Worksheet& operator[](const std::string& name) {
    if (!is_loaded(name)) {
        load_worksheet(name);  // 延迟加载
    }
    return get_worksheet(name);
}
```

**理由**：
- **内存效率**：只加载实际使用的工作表
- **启动速度**：减少初始化时间
- **可扩展性**：支持超大文件的处理

## 🔮 未来扩展性

### 1. 插件系统预留

**设计决策**：
```cpp
namespace TinaKit {
    template<typename FormatHandler>
    void register_format(std::string_view extension);
}

// 未来使用
TinaKit::register_format<CustomXmlFormat>(".cxml");
```

**理由**：
- **开放性**：第三方可以扩展支持的格式
- **模块化**：核心库保持精简，功能通过插件扩展
- **向后兼容**：新格式不影响现有代码

### 2. API 版本化策略

**设计决策**：
```cpp
namespace tinakit::v1 {
    // 当前 API
}

// 未来版本
namespace tinakit::v2 {
    // 新 API，可能有破坏性变更
}

// 默认使用最新版本
namespace tinakit = tinakit::v1;
```

**理由**：
- **稳定性**：现有代码不会因为库更新而破坏
- **演进性**：允许 API 的重大改进
- **选择性**：用户可以选择使用的 API 版本

## 📝 设计验证

我们的设计通过以下方式验证：

1. **Dream Code 验证**：所有 API 都能支持理想代码的编写
2. **类型安全验证**：编译时能捕获常见的使用错误
3. **性能验证**：关键路径的性能测试
4. **可用性验证**：真实用户场景的测试

这种设计驱动的方法确保了 TinaKit 不仅功能强大，而且真正易于使用。
