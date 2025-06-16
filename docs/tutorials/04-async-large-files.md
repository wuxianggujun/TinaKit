# 教程 4：异步处理大文件

在处理大型 Excel 文件时，传统的同步方法可能会导致程序阻塞，影响用户体验。TinaKit 提供了强大的异步处理能力，基于 C++20 协程，让你能够优雅地处理大文件而不阻塞主线程。

## 🎯 学习目标

完成本教程后，你将能够：
- 使用协程异步打开和处理大文件
- 实现非阻塞的数据处理流程
- 监控处理进度和处理错误
- 优化内存使用，避免内存溢出
- 实现流式数据处理

## 📋 前置条件

- 完成前三个教程
- 了解 C++20 协程基础（co_await, co_return）
- 理解异步编程概念

## 🚀 基础异步操作

### 异步打开文件

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <chrono>

using namespace tinakit;

// 异步打开文件的基本示例
Task<void> basic_async_example() {
    try {
        std::cout << "开始异步打开文件..." << std::endl;
        auto start_time = std::chrono::steady_clock::now();
        
        // 异步打开大文件
        auto workbook = co_await Excel::open_async("large_dataset.xlsx");
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
        
        std::cout << "文件打开完成，耗时: " << duration << "ms" << std::endl;
        std::cout << "工作表数量: " << workbook.sheet_count() << std::endl;
        
        // 获取文件信息
        std::cout << "文件大小: " << workbook.file_size() << " 字节" << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "异步操作失败: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== 异步文件处理示例 ===" << std::endl;
    
    // 启动异步任务
    auto task = basic_async_example();
    
    // 在等待期间可以做其他工作
    std::cout << "文件正在后台加载，可以执行其他任务..." << std::endl;
    
    // 等待任务完成
    task.wait();
    
    std::cout << "✅ 异步处理完成！" << std::endl;
    return 0;
}
```

### 带进度监控的异步处理

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <atomic>
#include <thread>

using namespace tinakit;

// 进度监控器
class ProgressMonitor {
private:
    std::atomic<double> progress_{0.0};
    std::atomic<bool> completed_{false};
    std::string current_status_;
    
public:
    void update_progress(double progress) {
        progress_ = progress;
        std::cout << "\r进度: " << std::fixed << std::setprecision(1) 
                  << (progress * 100) << "% " << current_status_ << std::flush;
    }
    
    void update_status(const std::string& status) {
        current_status_ = status;
    }
    
    void complete() {
        completed_ = true;
        std::cout << "\r进度: 100.0% 完成!" << std::endl;
    }
    
    double get_progress() const { return progress_; }
    bool is_completed() const { return completed_; }
};

Task<void> process_large_file_with_progress(const std::string& filename) {
    try {
        ProgressMonitor monitor;
        
        // 设置进度回调
        monitor.update_status("正在打开文件...");
        auto workbook = co_await Excel::open_async(filename);
        
        // 设置进度监控
        workbook.on_progress([&monitor](double progress) {
            monitor.update_progress(progress);
        });
        
        monitor.update_status("正在分析文件结构...");
        auto& sheet = workbook.active_sheet();
        
        // 获取数据范围
        auto last_row = sheet.last_row();
        auto last_col = sheet.last_column();
        
        std::cout << "\n文件信息:" << std::endl;
        std::cout << "  数据范围: A1:" << 
                     column_number_to_name(last_col) << last_row << std::endl;
        std::cout << "  预估单元格数: " << (last_row * last_col) << std::endl;
        
        monitor.update_status("正在处理数据...");
        
        // 分块处理数据
        const size_t chunk_size = 1000;  // 每次处理1000行
        size_t processed_rows = 0;
        
        for (size_t start_row = 1; start_row <= last_row; start_row += chunk_size) {
            size_t end_row = std::min(start_row + chunk_size - 1, last_row);
            
            // 异步处理数据块
            co_await process_data_chunk_async(sheet, start_row, end_row);
            
            processed_rows += (end_row - start_row + 1);
            double progress = static_cast<double>(processed_rows) / last_row;
            monitor.update_progress(progress);
            
            // 让出控制权，避免阻塞
            co_await std::suspend_always{};
        }
        
        monitor.complete();
        
    } catch (const TinaKitException& e) {
        std::cerr << "\n异步处理失败: " << e.what() << std::endl;
    }
}

// 异步处理数据块
Task<void> process_data_chunk_async(Worksheet& sheet, size_t start_row, size_t end_row) {
    // 模拟数据处理
    for (size_t row = start_row; row <= end_row; ++row) {
        // 处理当前行的数据
        auto row_data = sheet.row(row);
        
        // 这里可以进行各种数据处理操作
        // 例如：数据验证、转换、计算等
        
        // 每处理100行就让出一次控制权
        if (row % 100 == 0) {
            co_await std::suspend_always{};
        }
    }
    
    co_return;
}

int main() {
    std::cout << "=== 带进度监控的异步处理 ===" << std::endl;
    
    auto task = process_large_file_with_progress("large_dataset.xlsx");
    task.wait();
    
    return 0;
}
```

## 🔄 流式数据处理

### 内存优化的流式处理

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <memory>
#include <queue>

using namespace tinakit;

// 数据处理器接口
class DataProcessor {
public:
    virtual ~DataProcessor() = default;
    virtual Task<void> process_row(const Row& row) = 0;
    virtual Task<void> finalize() = 0;
};

// 统计处理器
class StatisticsProcessor : public DataProcessor {
private:
    size_t row_count_ = 0;
    double sum_ = 0.0;
    double min_value_ = std::numeric_limits<double>::max();
    double max_value_ = std::numeric_limits<double>::lowest();
    
public:
    Task<void> process_row(const Row& row) override {
        row_count_++;
        
        // 假设我们要统计第二列的数值
        auto value = row[2].try_as<double>();
        if (value) {
            sum_ += *value;
            min_value_ = std::min(min_value_, *value);
            max_value_ = std::max(max_value_, *value);
        }
        
        // 每处理1000行输出一次进度
        if (row_count_ % 1000 == 0) {
            std::cout << "已处理 " << row_count_ << " 行" << std::endl;
        }
        
        co_return;
    }
    
    Task<void> finalize() override {
        std::cout << "\n=== 统计结果 ===" << std::endl;
        std::cout << "总行数: " << row_count_ << std::endl;
        if (row_count_ > 0) {
            std::cout << "数值总和: " << sum_ << std::endl;
            std::cout << "平均值: " << (sum_ / row_count_) << std::endl;
            std::cout << "最小值: " << min_value_ << std::endl;
            std::cout << "最大值: " << max_value_ << std::endl;
        }
        co_return;
    }
};

// 数据验证处理器
class ValidationProcessor : public DataProcessor {
private:
    size_t valid_rows_ = 0;
    size_t invalid_rows_ = 0;
    std::vector<std::string> error_messages_;
    
public:
    Task<void> process_row(const Row& row) override {
        bool is_valid = true;
        std::string error_msg;
        
        // 验证第一列（姓名）不为空
        if (row[1].empty()) {
            is_valid = false;
            error_msg += "姓名为空; ";
        }
        
        // 验证第二列（年龄）是有效数字
        auto age = row[2].try_as<int>();
        if (!age || *age < 0 || *age > 150) {
            is_valid = false;
            error_msg += "年龄无效; ";
        }
        
        // 验证第三列（邮箱）格式
        auto email = row[3].as<std::string>();
        if (!email.contains("@")) {
            is_valid = false;
            error_msg += "邮箱格式错误; ";
        }
        
        if (is_valid) {
            valid_rows_++;
        } else {
            invalid_rows_++;
            error_messages_.push_back("第" + std::to_string(row.index()) + 
                                    "行: " + error_msg);
        }
        
        co_return;
    }
    
    Task<void> finalize() override {
        std::cout << "\n=== 验证结果 ===" << std::endl;
        std::cout << "有效行数: " << valid_rows_ << std::endl;
        std::cout << "无效行数: " << invalid_rows_ << std::endl;
        std::cout << "有效率: " << std::fixed << std::setprecision(2) 
                  << (static_cast<double>(valid_rows_) / (valid_rows_ + invalid_rows_) * 100) 
                  << "%" << std::endl;
        
        if (!error_messages_.empty() && error_messages_.size() <= 10) {
            std::cout << "\n错误详情:" << std::endl;
            for (const auto& msg : error_messages_) {
                std::cout << "  " << msg << std::endl;
            }
        } else if (error_messages_.size() > 10) {
            std::cout << "\n错误过多，仅显示前10个:" << std::endl;
            for (size_t i = 0; i < 10; ++i) {
                std::cout << "  " << error_messages_[i] << std::endl;
            }
        }
        
        co_return;
    }
};

// 流式处理引擎
class StreamProcessor {
private:
    std::vector<std::unique_ptr<DataProcessor>> processors_;
    
public:
    void add_processor(std::unique_ptr<DataProcessor> processor) {
        processors_.push_back(std::move(processor));
    }
    
    Task<void> process_file_stream(const std::string& filename) {
        try {
            std::cout << "开始流式处理文件: " << filename << std::endl;
            
            auto workbook = co_await Excel::open_async(filename);
            auto& sheet = workbook.active_sheet();
            
            // 流式处理每一行
            co_await sheet.process_rows_async([this](const Row& row) -> Task<void> {
                // 将行数据传递给所有处理器
                for (auto& processor : processors_) {
                    co_await processor->process_row(row);
                }
                co_return;
            });
            
            // 完成处理
            for (auto& processor : processors_) {
                co_await processor->finalize();
            }
            
            std::cout << "✅ 流式处理完成！" << std::endl;
            
        } catch (const TinaKitException& e) {
            std::cerr << "流式处理失败: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::cout << "=== 流式数据处理 ===" << std::endl;
    
    // 创建处理器
    StreamProcessor processor;
    processor.add_processor(std::make_unique<StatisticsProcessor>());
    processor.add_processor(std::make_unique<ValidationProcessor>());
    
    // 开始流式处理
    auto task = processor.process_file_stream("large_dataset.xlsx");
    task.wait();
    
    return 0;
}
```

## 🔀 并发处理

### 多线程异步处理

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <mutex>

using namespace tinakit;

// 线程安全的结果收集器
class ThreadSafeCollector {
private:
    std::mutex mutex_;
    std::vector<std::string> results_;
    size_t processed_count_ = 0;
    
public:
    void add_result(const std::string& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        results_.push_back(result);
        processed_count_++;
        
        if (processed_count_ % 100 == 0) {
            std::cout << "已处理 " << processed_count_ << " 个结果" << std::endl;
        }
    }
    
    std::vector<std::string> get_results() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return results_;
    }
    
    size_t get_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return processed_count_;
    }
};

// 并发处理工作单元
Task<void> process_worksheet_concurrent(Worksheet& sheet, 
                                       size_t start_row, 
                                       size_t end_row,
                                       ThreadSafeCollector& collector) {
    try {
        for (size_t row = start_row; row <= end_row; ++row) {
            auto row_data = sheet.row(row);
            
            // 模拟复杂的数据处理
            if (!row_data.empty()) {
                auto name = row_data[1].try_as<std::string>().value_or("");
                auto value = row_data[2].try_as<double>().value_or(0.0);
                
                // 进行一些计算
                auto result = std::format("Row {}: {} = {:.2f}", 
                                        row, name, value * 1.1);
                
                collector.add_result(result);
            }
            
            // 定期让出控制权
            if (row % 50 == 0) {
                co_await std::suspend_always{};
            }
        }
    } catch (const TinaKitException& e) {
        std::cerr << "处理范围 " << start_row << "-" << end_row 
                  << " 时出错: " << e.what() << std::endl;
    }
    
    co_return;
}

Task<void> concurrent_file_processing(const std::string& filename) {
    try {
        std::cout << "开始并发处理文件: " << filename << std::endl;
        
        auto workbook = co_await Excel::open_async(filename);
        auto& sheet = workbook.active_sheet();
        
        auto total_rows = sheet.last_row();
        const size_t chunk_size = 1000;
        const size_t max_threads = std::thread::hardware_concurrency();
        
        std::cout << "总行数: " << total_rows << std::endl;
        std::cout << "使用线程数: " << max_threads << std::endl;
        
        ThreadSafeCollector collector;
        std::vector<Task<void>> tasks;
        
        // 创建并发任务
        for (size_t start = 1; start <= total_rows; start += chunk_size) {
            size_t end = std::min(start + chunk_size - 1, total_rows);
            
            auto task = process_worksheet_concurrent(sheet, start, end, collector);
            tasks.push_back(std::move(task));
            
            // 限制并发任务数量
            if (tasks.size() >= max_threads) {
                // 等待一些任务完成
                for (auto& task : tasks) {
                    task.wait();
                }
                tasks.clear();
            }
        }
        
        // 等待剩余任务完成
        for (auto& task : tasks) {
            task.wait();
        }
        
        std::cout << "✅ 并发处理完成！" << std::endl;
        std::cout << "总处理结果数: " << collector.get_count() << std::endl;
        
        // 保存结果
        auto result_workbook = Excel::create();
        auto& result_sheet = result_workbook.active_sheet();
        result_sheet.title("处理结果");
        
        auto results = collector.get_results();
        for (size_t i = 0; i < results.size() && i < 1000; ++i) {  // 只保存前1000个结果
            result_sheet["A" + std::to_string(i + 1)].value(results[i]);
        }
        
        result_workbook.save("concurrent_processing_results.xlsx");
        std::cout << "结果已保存到 concurrent_processing_results.xlsx" << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "并发处理失败: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== 并发异步处理 ===" << std::endl;
    
    auto task = concurrent_file_processing("large_dataset.xlsx");
    task.wait();
    
    return 0;
}
```

## 💾 异步保存大文件

### 分块异步保存

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <random>

using namespace tinakit;

// 生成大量测试数据
Task<void> generate_large_dataset() {
    try {
        std::cout << "开始生成大型数据集..." << std::endl;
        
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        sheet.title("大型数据集");
        
        // 设置标题行
        sheet["A1"].value("ID");
        sheet["B1"].value("姓名");
        sheet["C1"].value("年龄");
        sheet["D1"].value("薪资");
        sheet["E1"].value("部门");
        
        // 随机数生成器
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> age_dist(22, 65);
        std::uniform_real_distribution<> salary_dist(5000.0, 50000.0);
        
        std::vector<std::string> names = {
            "张三", "李四", "王五", "赵六", "钱七", "孙八", "周九", "吴十"
        };
        std::vector<std::string> departments = {
            "技术部", "销售部", "市场部", "人事部", "财务部"
        };
        
        const size_t total_rows = 100000;  // 生成10万行数据
        const size_t batch_size = 1000;    // 每批处理1000行
        
        for (size_t batch = 0; batch < total_rows; batch += batch_size) {
            size_t end_row = std::min(batch + batch_size, total_rows);
            
            // 生成一批数据
            for (size_t i = batch; i < end_row; ++i) {
                size_t row = i + 2;  // 从第2行开始（跳过标题）
                
                sheet["A" + std::to_string(row)].value(static_cast<int>(i + 1));
                sheet["B" + std::to_string(row)].value(
                    names[gen() % names.size()] + std::to_string(i));
                sheet["C" + std::to_string(row)].value(age_dist(gen));
                sheet["D" + std::to_string(row)].value(salary_dist(gen));
                sheet["E" + std::to_string(row)].value(
                    departments[gen() % departments.size()]);
            }
            
            // 显示进度
            double progress = static_cast<double>(end_row) / total_rows;
            std::cout << "\r生成进度: " << std::fixed << std::setprecision(1) 
                      << (progress * 100) << "%" << std::flush;
            
            // 让出控制权
            co_await std::suspend_always{};
        }
        
        std::cout << "\n数据生成完成，开始异步保存..." << std::endl;
        
        // 设置保存进度回调
        workbook.on_progress([](double progress) {
            std::cout << "\r保存进度: " << std::fixed << std::setprecision(1) 
                      << (progress * 100) << "%" << std::flush;
        });
        
        // 异步保存大文件
        co_await workbook.save_async("large_generated_dataset.xlsx");
        
        std::cout << "\n✅ 大型数据集生成并保存完成！" << std::endl;
        std::cout << "文件: large_generated_dataset.xlsx" << std::endl;
        std::cout << "数据行数: " << total_rows << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "\n生成数据集失败: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== 异步生成和保存大文件 ===" << std::endl;
    
    auto task = generate_large_dataset();
    task.wait();
    
    return 0;
}
```

## 🛡️ 错误处理和恢复

### 异步错误处理

```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>
#include <chrono>

using namespace tinakit;

// 带重试机制的异步处理
Task<bool> process_with_retry(const std::string& filename, int max_retries = 3) {
    for (int attempt = 1; attempt <= max_retries; ++attempt) {
        try {
            std::cout << "尝试 " << attempt << "/" << max_retries 
                      << " 处理文件: " << filename << std::endl;
            
            auto workbook = co_await Excel::open_async(filename);
            
            // 设置错误回调
            workbook.on_error([](const std::exception& error) {
                std::cerr << "处理过程中出现错误: " << error.what() << std::endl;
            });
            
            auto& sheet = workbook.active_sheet();
            
            // 模拟可能失败的处理
            co_await sheet.process_rows_async([](const Row& row) -> Task<void> {
                // 模拟随机错误
                static int counter = 0;
                if (++counter % 1000 == 0) {
                    // 每1000行可能出现一次错误
                    if (std::rand() % 10 == 0) {
                        throw std::runtime_error("模拟的处理错误");
                    }
                }
                co_return;
            });
            
            std::cout << "✅ 文件处理成功！" << std::endl;
            co_return true;
            
        } catch (const FileNotFoundException& e) {
            std::cerr << "文件未找到，无法重试: " << e.what() << std::endl;
            co_return false;
        } catch (const TinaKitException& e) {
            std::cerr << "尝试 " << attempt << " 失败: " << e.what() << std::endl;
            
            if (attempt < max_retries) {
                std::cout << "等待 2 秒后重试..." << std::endl;
                co_await delay(std::chrono::seconds(2));
            }
        }
    }
    
    std::cerr << "❌ 所有重试都失败了" << std::endl;
    co_return false;
}

// 超时处理
Task<bool> process_with_timeout(const std::string& filename, 
                               std::chrono::seconds timeout) {
    try {
        std::cout << "开始处理文件，超时时间: " << timeout.count() << " 秒" << std::endl;
        
        auto start_time = std::chrono::steady_clock::now();
        auto workbook = co_await Excel::open_async(filename);
        
        // 检查是否超时
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > timeout) {
            std::cerr << "操作超时" << std::endl;
            co_return false;
        }
        
        // 继续处理...
        std::cout << "✅ 在超时前完成处理" << std::endl;
        co_return true;
        
    } catch (const TinaKitException& e) {
        std::cerr << "处理失败: " << e.what() << std::endl;
        co_return false;
    }
}

int main() {
    std::cout << "=== 异步错误处理和恢复 ===" << std::endl;
    
    // 测试重试机制
    auto retry_task = process_with_retry("problematic_file.xlsx");
    bool retry_success = retry_task.wait();
    
    if (retry_success) {
        std::cout << "重试处理成功" << std::endl;
    } else {
        std::cout << "重试处理失败" << std::endl;
    }
    
    // 测试超时处理
    auto timeout_task = process_with_timeout("large_file.xlsx", 
                                           std::chrono::seconds(30));
    bool timeout_success = timeout_task.wait();
    
    if (timeout_success) {
        std::cout << "超时处理成功" << std::endl;
    } else {
        std::cout << "超时处理失败" << std::endl;
    }
    
    return 0;
}
```

## 📝 小结

在这个教程中，你学会了：

✅ **异步文件操作**：使用协程进行非阻塞的文件处理  
✅ **进度监控**：实时跟踪处理进度和状态  
✅ **流式处理**：内存优化的大文件处理策略  
✅ **并发处理**：多线程异步处理提升性能  
✅ **错误处理**：重试机制和超时处理  
✅ **异步保存**：大文件的分块保存策略  

## 🚀 下一步

现在你已经掌握了异步处理大文件的技能，接下来可以：

- 学习 [教程 5：格式化和样式设置](05-advanced-formatting.md)
- 探索 [性能优化技巧](../performance/optimization-guide.md)
- 查看 [内存管理指南](../performance/memory-management.md)

---

有问题？在 [GitHub Issues](https://github.com/your-username/TinaKit/issues) 中报告异步处理相关的问题。
