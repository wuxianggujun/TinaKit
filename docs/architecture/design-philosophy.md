# TinaKit 设计哲学

TinaKit 的设计哲学体现了现代 C++ 的最佳实践和软件工程的核心原则。本文档详细阐述了指导 TinaKit 开发的核心理念。

## 🎯 核心原则

### 1. 现代 C++ 优先 (Modern C++ First)

**理念**: 充分拥抱 C++20 的新特性，编写更安全、更表达性强的代码。

#### Concepts - 类型约束

```cpp
// 定义清晰的接口约束
template<typename T>
concept CellValue = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string>;
} || std::integral<T> || std::floating_point<T> || std::same_as<T, std::string>;

// 使用 concept 约束模板参数
template<CellValue T>
void set_cell_value(Cell& cell, const T& value) {
    cell.value(value);
}
```

**优势**:
- 编译时类型检查
- 更清晰的错误信息
- 自文档化的接口

#### Ranges - 函数式数据处理

```cpp
// 使用 ranges 进行数据转换
auto process_worksheet_data(const Worksheet& sheet) {
    return sheet.rows()
        | std::views::drop(1)  // 跳过标题行
        | std::views::filter([](const Row& row) { 
            return !row.empty(); 
        })
        | std::views::transform([](const Row& row) {
            return extract_data(row);
        })
        | std::ranges::to<std::vector>();
}
```

**优势**:
- 声明式编程风格
- 延迟求值
- 组合性强

#### Coroutines - 异步处理

```cpp
// 异步文件处理
Task<ProcessResult> process_large_file_async(const std::filesystem::path& path) {
    auto file_stream = co_await open_file_async(path);
    
    while (auto chunk = co_await read_chunk_async(file_stream)) {
        auto result = co_await process_chunk_async(chunk);
        co_yield result;  // 返回中间结果
    }
    
    co_return ProcessResult::success();
}
```

**优势**:
- 非阻塞 I/O
- 自然的异步代码表达
- 高效的资源利用

### 2. RAII - 资源获取即初始化

**理念**: 通过对象的生命周期自动管理资源，避免内存泄漏和资源泄漏。

#### 自动资源管理

```cpp
class Workbook {
private:
    std::unique_ptr<WorkbookImpl> impl_;
    
public:
    // 构造时获取资源
    explicit Workbook(const std::filesystem::path& path) 
        : impl_(std::make_unique<WorkbookImpl>(path)) {
        // 资源在构造函数中获取
    }
    
    // 析构时自动释放资源
    ~Workbook() = default;  // unique_ptr 自动清理
    
    // 移动语义支持
    Workbook(Workbook&&) = default;
    Workbook& operator=(Workbook&&) = default;
    
    // 禁止拷贝
    Workbook(const Workbook&) = delete;
    Workbook& operator=(const Workbook&) = delete;
};
```

#### 异常安全保证

```cpp
class WorksheetTransaction {
private:
    Worksheet& worksheet_;
    std::vector<CellChange> changes_;
    bool committed_ = false;
    
public:
    explicit WorksheetTransaction(Worksheet& ws) : worksheet_(ws) {}
    
    ~WorksheetTransaction() {
        if (!committed_) {
            rollback();  // 异常时自动回滚
        }
    }
    
    void commit() {
        worksheet_.apply_changes(changes_);
        committed_ = true;
    }
    
private:
    void rollback() {
        // 回滚所有更改
        for (auto it = changes_.rbegin(); it != changes_.rend(); ++it) {
            worksheet_.revert_change(*it);
        }
    }
};
```

### 3. 类型安全 (Type Safety)

**理念**: 在编译时捕获尽可能多的错误，减少运行时错误。

#### 强类型系统

```cpp
// 使用强类型避免参数错误
class CellAddress {
private:
    std::string address_;
    
public:
    explicit CellAddress(std::string addr) : address_(std::move(addr)) {
        if (!is_valid_address(address_)) {
            throw InvalidCellAddressException(address_);
        }
    }
    
    const std::string& value() const { return address_; }
};

class RowIndex {
private:
    std::size_t index_;
    
public:
    explicit RowIndex(std::size_t idx) : index_(idx) {
        if (idx == 0) {
            throw InvalidRowIndexException("Row index must be >= 1");
        }
    }
    
    std::size_t value() const { return index_; }
};

// API 使用强类型
Cell get_cell(const CellAddress& address);
Row get_row(RowIndex index);
```

#### 编译时常量

```cpp
namespace tinakit::constants {
    inline constexpr std::size_t max_rows = 1048576;
    inline constexpr std::size_t max_columns = 16384;
    inline constexpr std::string_view default_sheet_name = "Sheet1";
}

// 编译时验证
template<std::size_t N>
constexpr bool is_valid_row_count() {
    return N <= constants::max_rows;
}

template<std::size_t N>
requires is_valid_row_count<N>()
class FixedSizeWorksheet {
    // 编译时保证行数不超过限制
};
```

### 4. 可扩展性设计 (Extensibility Design)

**理念**: 设计灵活的架构，支持功能扩展而不修改核心代码。

#### 插件架构

```cpp
// 插件接口
class FormatPlugin {
public:
    virtual ~FormatPlugin() = default;
    virtual std::string_view name() const = 0;
    virtual std::vector<std::string> supported_extensions() const = 0;
    virtual std::unique_ptr<DocumentReader> create_reader() const = 0;
    virtual std::unique_ptr<DocumentWriter> create_writer() const = 0;
};

// 插件注册机制
class PluginRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<FormatPlugin>> plugins_;
    
public:
    void register_plugin(std::unique_ptr<FormatPlugin> plugin) {
        auto name = std::string(plugin->name());
        plugins_[name] = std::move(plugin);
    }
    
    template<typename PluginT, typename... Args>
    void register_plugin(Args&&... args) {
        register_plugin(std::make_unique<PluginT>(std::forward<Args>(args)...));
    }
    
    const FormatPlugin* get_plugin(std::string_view name) const {
        auto it = plugins_.find(std::string(name));
        return it != plugins_.end() ? it->second.get() : nullptr;
    }
};
```

#### 回调机制

```cpp
// 事件系统
template<typename... Args>
class Event {
private:
    std::vector<std::function<void(Args...)>> handlers_;
    
public:
    void subscribe(std::function<void(Args...)> handler) {
        handlers_.push_back(std::move(handler));
    }
    
    void emit(Args... args) {
        for (auto& handler : handlers_) {
            handler(args...);
        }
    }
};

class DocumentProcessor {
public:
    Event<double> progress_changed;
    Event<std::string_view> status_changed;
    Event<const std::exception&> error_occurred;
    
    void process_document(const Document& doc) {
        status_changed.emit("开始处理文档");
        
        try {
            for (std::size_t i = 0; i < doc.page_count(); ++i) {
                process_page(doc.page(i));
                
                double progress = static_cast<double>(i + 1) / doc.page_count();
                progress_changed.emit(progress);
            }
            
            status_changed.emit("文档处理完成");
        } catch (const std::exception& e) {
            error_occurred.emit(e);
        }
    }
};
```

### 5. 性能优先 (Performance First)

**理念**: 在保持代码清晰的前提下，追求最佳性能。

#### 零成本抽象

```cpp
// 使用模板和内联函数实现零成本抽象
template<typename T>
class CellValue {
private:
    T value_;
    
public:
    constexpr explicit CellValue(T value) : value_(std::move(value)) {}
    
    constexpr const T& get() const noexcept { return value_; }
    constexpr T& get() noexcept { return value_; }
    
    // 隐式转换，零运行时成本
    constexpr operator const T&() const noexcept { return value_; }
};

// 编译器会优化掉包装类型
inline void process_cell(const CellValue<int>& value) {
    // 直接操作 int，没有额外开销
    std::cout << value.get() << std::endl;
}
```

#### 内存池和对象重用

```cpp
template<typename T>
class ObjectPool {
private:
    std::stack<std::unique_ptr<T>> available_;
    std::mutex mutex_;
    
public:
    template<typename... Args>
    std::unique_ptr<T> acquire(Args&&... args) {
        std::lock_guard lock(mutex_);
        
        if (!available_.empty()) {
            auto obj = std::move(available_.top());
            available_.pop();
            obj->reset(std::forward<Args>(args)...);
            return obj;
        }
        
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    
    void release(std::unique_ptr<T> obj) {
        std::lock_guard lock(mutex_);
        available_.push(std::move(obj));
    }
};

// 全局对象池
inline ObjectPool<XmlParser>& get_parser_pool() {
    static ObjectPool<XmlParser> pool;
    return pool;
}
```

### 6. 错误处理哲学

**理念**: 明确区分可恢复错误和不可恢复错误，提供清晰的错误信息。

#### 异常层次结构

```cpp
namespace tinakit {
    // 基础异常类
    class TinaKitException : public std::exception {
    private:
        std::string message_;
        std::string context_;
        
    public:
        TinaKitException(std::string message, std::string context = "")
            : message_(std::move(message)), context_(std::move(context)) {}
        
        const char* what() const noexcept override {
            return message_.c_str();
        }
        
        const std::string& context() const noexcept {
            return context_;
        }
    };
    
    // 具体异常类型
    class ParseException : public TinaKitException {
    public:
        ParseException(std::string message, std::size_t line = 0, std::size_t column = 0)
            : TinaKitException(std::move(message), 
                              "Line: " + std::to_string(line) + 
                              ", Column: " + std::to_string(column)) {}
    };
    
    class IOException : public TinaKitException {
    public:
        IOException(std::string message, std::filesystem::path path)
            : TinaKitException(std::move(message), "Path: " + path.string()) {}
    };
}
```

#### Expected/Result 类型

```cpp
template<typename T, typename E = std::exception_ptr>
class Expected {
private:
    std::variant<T, E> value_;
    
public:
    Expected(T value) : value_(std::move(value)) {}
    Expected(E error) : value_(std::move(error)) {}
    
    bool has_value() const noexcept {
        return std::holds_alternative<T>(value_);
    }
    
    const T& value() const {
        if (!has_value()) {
            std::rethrow_exception(std::get<E>(value_));
        }
        return std::get<T>(value_);
    }
    
    const E& error() const noexcept {
        return std::get<E>(value_);
    }
};

// 使用 Expected 进行错误处理
Expected<Workbook> try_open_workbook(const std::filesystem::path& path) noexcept {
    try {
        return Workbook::open(path);
    } catch (...) {
        return std::current_exception();
    }
}
```

## 🎨 设计模式应用

### 1. 策略模式 + 现代 C++

```cpp
// 使用 std::function 和 lambda 实现策略模式
class CellFormatter {
public:
    using FormatStrategy = std::function<std::string(const CellValue&)>;
    
private:
    FormatStrategy strategy_;
    
public:
    explicit CellFormatter(FormatStrategy strategy) 
        : strategy_(std::move(strategy)) {}
    
    std::string format(const CellValue& value) const {
        return strategy_(value);
    }
};

// 使用示例
auto number_formatter = CellFormatter([](const CellValue& value) {
    return std::format("{:.2f}", value.as<double>());
});

auto date_formatter = CellFormatter([](const CellValue& value) {
    auto date = value.as<std::chrono::system_clock::time_point>();
    return std::format("{:%Y-%m-%d}", date);
});
```

### 2. 观察者模式 + 现代 C++

```cpp
// 使用信号槽机制
template<typename... Args>
class Signal {
private:
    mutable std::vector<std::function<void(Args...)>> slots_;
    mutable std::shared_mutex mutex_;
    
public:
    void connect(std::function<void(Args...)> slot) const {
        std::unique_lock lock(mutex_);
        slots_.push_back(std::move(slot));
    }
    
    void emit(Args... args) const {
        std::shared_lock lock(mutex_);
        for (const auto& slot : slots_) {
            slot(args...);
        }
    }
};

class Document {
public:
    mutable Signal<const std::string&> content_changed;
    mutable Signal<double> save_progress;
    
    void set_content(std::string content) {
        content_ = std::move(content);
        content_changed.emit(content_);
    }
};
```

## 🔄 持续改进

TinaKit 的设计哲学是一个持续演进的过程。我们会根据：

- **用户反馈**: 实际使用中的痛点和需求
- **技术发展**: C++ 标准的更新和最佳实践的演进
- **性能分析**: 基准测试和性能分析的结果
- **社区贡献**: 开源社区的智慧和经验

来不断完善和优化我们的设计理念。

---

这些设计哲学确保了 TinaKit 不仅是一个功能强大的库，更是一个展示现代 C++ 最佳实践的典范。
