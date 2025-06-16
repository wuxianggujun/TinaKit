# 教程 3：现代 C++ 数据处理

TinaKit 充分拥抱现代 C++20 特性，让数据处理变得更加优雅和高效。在这个教程中，你将学会如何使用 ranges、concepts、算法等现代 C++ 特性来处理 Excel 数据。

## 🎯 学习目标

完成本教程后，你将能够：
- 使用 C++20 ranges 进行数据处理
- 应用 concepts 进行类型约束
- 利用算法库处理 Excel 数据
- 编写函数式风格的数据处理代码
- 使用现代 C++ 特性提升代码质量

## 📋 前置条件

- 完成前两个教程
- 了解 C++20 基础特性（ranges、concepts、算法）
- 支持 C++20 的编译器

## 🔄 使用 Ranges 处理数据

### 基础 Ranges 操作

```cpp
#include <tinakit/tinakit.hpp>
#include <ranges>
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    using namespace tinakit;
    using namespace std::ranges;
    
    try {
        auto workbook = Excel::open("employees.xlsx");
        auto& sheet = workbook["员工信息"];
        
        std::cout << "=== 使用 Ranges 处理员工数据 ===" << std::endl;
        
        // 获取所有数据行（跳过标题行）
        auto data_rows = sheet.rows(2, 100);  // 从第2行到第100行
        
        // 方法 1：过滤高薪员工
        auto high_salary_employees = data_rows
            | views::filter([](const Row& row) { 
                return !row.empty() && row["C"].try_as<double>().value_or(0) > 15000; 
            })
            | views::transform([](const Row& row) {
                return std::format("{}: ¥{:.2f}", 
                    row["A"].as<std::string>(),
                    row["C"].as<double>()
                );
            })
            | to<std::vector>();
        
        std::cout << "高薪员工 (>15000):" << std::endl;
        for (const auto& employee : high_salary_employees) {
            std::cout << "  " << employee << std::endl;
        }
        
        // 方法 2：按部门分组统计
        auto tech_dept_count = data_rows
            | views::filter([](const Row& row) {
                return !row.empty() && 
                       row["C"].as<std::string>().contains("技术");
            })
            | views::transform([](const Row& row) { return 1; })
            | views::common
            | std::ranges::fold_left(0, std::plus{});
        
        std::cout << "\n技术部门员工数量: " << tech_dept_count << std::endl;
        
        // 方法 3：计算平均薪资
        auto salaries = data_rows
            | views::filter([](const Row& row) { return !row.empty(); })
            | views::transform([](const Row& row) {
                return row["D"].try_as<double>().value_or(0.0);
            })
            | views::filter([](double salary) { return salary > 0; })
            | to<std::vector>();
        
        if (!salaries.empty()) {
            double avg_salary = std::ranges::fold_left(salaries, 0.0, std::plus{}) / salaries.size();
            std::cout << "平均薪资: ¥" << std::format("{:.2f}", avg_salary) << std::endl;
        }
        
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 高级 Ranges 操作

```cpp
#include <tinakit/tinakit.hpp>
#include <ranges>
#include <iostream>
#include <map>
#include <set>

int main() {
    using namespace tinakit;
    using namespace std::ranges;
    
    try {
        auto workbook = Excel::open("sales_data.xlsx");
        auto& sheet = workbook["销售记录"];
        
        std::cout << "=== 高级数据分析 ===" << std::endl;
        
        // 获取所有销售记录
        auto sales_rows = sheet.rows(2, 1000)
            | views::filter([](const Row& row) { return !row.empty(); });
        
        // 1. 按销售员分组统计销售额
        std::map<std::string, double> sales_by_person;
        
        for (const auto& row : sales_rows) {
            auto salesperson = row["A"].as<std::string>();
            auto amount = row["E"].try_as<double>().value_or(0.0);
            sales_by_person[salesperson] += amount;
        }
        
        // 找出销售冠军
        auto top_salesperson = max_element(sales_by_person, 
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        
        if (top_salesperson != sales_by_person.end()) {
            std::cout << "销售冠军: " << top_salesperson->first 
                      << " (¥" << std::format("{:.2f}", top_salesperson->second) << ")" << std::endl;
        }
        
        // 2. 产品类别分析
        auto product_categories = sales_rows
            | views::transform([](const Row& row) {
                return row["B"].as<std::string>();
            })
            | to<std::set>();  // 自动去重
        
        std::cout << "\n产品类别:" << std::endl;
        for (const auto& category : product_categories) {
            // 计算该类别的总销售额
            auto category_total = sales_rows
                | views::filter([&category](const Row& row) {
                    return row["B"].as<std::string>() == category;
                })
                | views::transform([](const Row& row) {
                    return row["E"].try_as<double>().value_or(0.0);
                })
                | views::common
                | std::ranges::fold_left(0.0, std::plus{});
            
            std::cout << "  " << category << ": ¥" 
                      << std::format("{:.2f}", category_total) << std::endl;
        }
        
        // 3. 销售趋势分析（按月）
        std::map<std::string, double> monthly_sales;
        
        for (const auto& row : sales_rows) {
            // 假设日期在 F 列，格式为 "2024-01-15"
            auto date_str = row["F"].as<std::string>();
            auto month = date_str.substr(0, 7);  // 提取 "2024-01"
            auto amount = row["E"].try_as<double>().value_or(0.0);
            monthly_sales[month] += amount;
        }
        
        std::cout << "\n月度销售趋势:" << std::endl;
        for (const auto& [month, total] : monthly_sales) {
            std::cout << "  " << month << ": ¥" 
                      << std::format("{:.2f}", total) << std::endl;
        }
        
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🎨 使用 Concepts 进行类型约束

### 定义自定义 Concepts

```cpp
#include <tinakit/tinakit.hpp>
#include <concepts>
#include <iostream>
#include <string>

// 定义可转换为字符串的概念
template<typename T>
concept StringConvertible = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string>;
} || std::convertible_to<T, std::string>;

// 定义数值类型概念
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// 定义可写入 Excel 的概念
template<typename T>
concept ExcelWritable = StringConvertible<T> || Numeric<T> || std::same_as<T, bool>;

// 泛型函数：安全地写入 Excel 单元格
template<ExcelWritable T>
void safe_write_cell(tinakit::Cell& cell, const T& value) {
    try {
        cell.value(value);
        std::cout << "成功写入: " << cell.address() << std::endl;
    } catch (const tinakit::TinaKitException& e) {
        std::cerr << "写入失败 " << cell.address() << ": " << e.what() << std::endl;
    }
}

// 泛型函数：安全地读取数值
template<Numeric T>
std::optional<T> safe_read_numeric(const tinakit::Cell& cell) {
    return cell.try_as<T>();
}

// 自定义数据类型
struct Employee {
    std::string name;
    int age;
    double salary;
    
    std::string to_string() const {
        return std::format("{} ({}, ¥{:.2f})", name, age, salary);
    }
};

int main() {
    using namespace tinakit;
    
    try {
        auto workbook = Excel::create();
        auto& sheet = workbook.active_sheet();
        
        std::cout << "=== 使用 Concepts 的类型安全操作 ===" << std::endl;
        
        // 测试不同类型的写入
        safe_write_cell(sheet["A1"], std::string("姓名"));
        safe_write_cell(sheet["B1"], 42);
        safe_write_cell(sheet["C1"], 3.14159);
        safe_write_cell(sheet["D1"], true);
        
        // 写入自定义类型
        Employee emp{"张三", 28, 15000.0};
        safe_write_cell(sheet["A2"], emp);
        
        // 类型安全的数值读取
        sheet["B2"].value(100);
        sheet["C2"].value(99.5);
        sheet["D2"].value("not a number");
        
        auto int_val = safe_read_numeric<int>(sheet["B2"]);
        auto double_val = safe_read_numeric<double>(sheet["C2"]);
        auto invalid_val = safe_read_numeric<int>(sheet["D2"]);
        
        std::cout << "\n数值读取结果:" << std::endl;
        std::cout << "B2 (int): " << (int_val ? std::to_string(*int_val) : "无效") << std::endl;
        std::cout << "C2 (double): " << (double_val ? std::to_string(*double_val) : "无效") << std::endl;
        std::cout << "D2 (int): " << (invalid_val ? std::to_string(*invalid_val) : "无效") << std::endl;
        
        workbook.save("concepts_example.xlsx");
        
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🧮 函数式数据处理

### 数据转换管道

```cpp
#include <tinakit/tinakit.hpp>
#include <ranges>
#include <functional>
#include <iostream>
#include <vector>
#include <numeric>

// 函数式工具函数
auto to_upper = [](const std::string& str) {
    std::string result = str;
    std::ranges::transform(result, result.begin(), ::toupper);
    return result;
};

auto is_valid_email = [](const std::string& email) {
    return email.contains("@") && email.contains(".");
};

auto extract_domain = [](const std::string& email) {
    auto at_pos = email.find('@');
    return at_pos != std::string::npos ? email.substr(at_pos + 1) : "";
};

int main() {
    using namespace tinakit;
    using namespace std::ranges;
    
    try {
        auto workbook = Excel::open("customer_data.xlsx");
        auto& sheet = workbook["客户信息"];
        
        std::cout << "=== 函数式数据处理 ===" << std::endl;
        
        // 创建数据处理管道
        auto customer_rows = sheet.rows(2, 1000)
            | views::filter([](const Row& row) { return !row.empty(); });
        
        // 1. 邮箱验证和域名统计
        auto email_domains = customer_rows
            | views::transform([](const Row& row) {
                return row["C"].as<std::string>();  // 假设邮箱在 C 列
            })
            | views::filter(is_valid_email)
            | views::transform(extract_domain)
            | views::filter([](const std::string& domain) { 
                return !domain.empty(); 
            })
            | to<std::vector>();
        
        // 统计域名分布
        std::map<std::string, int> domain_count;
        for (const auto& domain : email_domains) {
            domain_count[domain]++;
        }
        
        std::cout << "邮箱域名分布:" << std::endl;
        for (const auto& [domain, count] : domain_count) {
            std::cout << "  " << domain << ": " << count << std::endl;
        }
        
        // 2. 客户年龄分析
        auto ages = customer_rows
            | views::transform([](const Row& row) {
                return row["D"].try_as<int>().value_or(0);  // 假设年龄在 D 列
            })
            | views::filter([](int age) { return age > 0 && age < 120; })
            | to<std::vector>();
        
        if (!ages.empty()) {
            auto [min_age, max_age] = std::ranges::minmax(ages);
            auto avg_age = std::accumulate(ages.begin(), ages.end(), 0.0) / ages.size();
            
            std::cout << "\n年龄统计:" << std::endl;
            std::cout << "  最小年龄: " << min_age << std::endl;
            std::cout << "  最大年龄: " << max_age << std::endl;
            std::cout << "  平均年龄: " << std::format("{:.1f}", avg_age) << std::endl;
            
            // 年龄分组
            auto young = count_if(ages, [](int age) { return age < 30; });
            auto middle = count_if(ages, [](int age) { return age >= 30 && age < 50; });
            auto senior = count_if(ages, [](int age) { return age >= 50; });
            
            std::cout << "  年龄分布:" << std::endl;
            std::cout << "    < 30岁: " << young << std::endl;
            std::cout << "    30-49岁: " << middle << std::endl;
            std::cout << "    >= 50岁: " << senior << std::endl;
        }
        
        // 3. 创建处理结果报表
        auto result_workbook = Excel::create();
        auto& result_sheet = result_workbook.active_sheet();
        result_sheet.title("数据分析结果");
        
        // 写入域名统计
        result_sheet["A1"].value("邮箱域名统计").bold();
        result_sheet["A2"].value("域名");
        result_sheet["B2"].value("数量");
        
        int row = 3;
        for (const auto& [domain, count] : domain_count) {
            result_sheet["A" + std::to_string(row)].value(domain);
            result_sheet["B" + std::to_string(row)].value(count);
            row++;
        }
        
        // 写入年龄统计
        result_sheet["D1"].value("年龄统计").bold();
        if (!ages.empty()) {
            auto [min_age, max_age] = std::ranges::minmax(ages);
            auto avg_age = std::accumulate(ages.begin(), ages.end(), 0.0) / ages.size();
            
            result_sheet["D2"].value("最小年龄");
            result_sheet["E2"].value(min_age);
            result_sheet["D3"].value("最大年龄");
            result_sheet["E3"].value(max_age);
            result_sheet["D4"].value("平均年龄");
            result_sheet["E4"].value(avg_age);
        }
        
        result_workbook.save("analysis_results.xlsx");
        std::cout << "\n✅ 分析结果已保存到 analysis_results.xlsx" << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔄 数据转换和清洗

### 数据清洗管道

```cpp
#include <tinakit/tinakit.hpp>
#include <ranges>
#include <regex>
#include <iostream>

// 数据清洗函数
class DataCleaner {
public:
    // 清理电话号码
    static std::string clean_phone(const std::string& phone) {
        std::regex phone_regex(R"([^\d])");
        auto cleaned = std::regex_replace(phone, phone_regex, "");
        
        // 标准化为 11 位手机号码格式
        if (cleaned.length() == 11 && cleaned[0] == '1') {
            return cleaned.substr(0, 3) + "-" + 
                   cleaned.substr(3, 4) + "-" + 
                   cleaned.substr(7, 4);
        }
        return cleaned;
    }
    
    // 清理姓名
    static std::string clean_name(const std::string& name) {
        std::string result = name;
        
        // 移除前后空格
        result.erase(0, result.find_first_not_of(" \t\n\r"));
        result.erase(result.find_last_not_of(" \t\n\r") + 1);
        
        // 标准化大小写（首字母大写）
        if (!result.empty()) {
            result[0] = std::toupper(result[0]);
            for (size_t i = 1; i < result.length(); ++i) {
                result[i] = std::tolower(result[i]);
            }
        }
        
        return result;
    }
    
    // 验证邮箱格式
    static bool is_valid_email(const std::string& email) {
        std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(email, email_regex);
    }
    
    // 标准化邮箱
    static std::string normalize_email(const std::string& email) {
        std::string result = email;
        std::ranges::transform(result, result.begin(), ::tolower);
        return result;
    }
};

int main() {
    using namespace tinakit;
    using namespace std::ranges;
    
    try {
        auto workbook = Excel::open("raw_customer_data.xlsx");
        auto& source_sheet = workbook["原始数据"];
        
        // 创建清洗后的工作簿
        auto clean_workbook = Excel::create();
        auto& clean_sheet = clean_workbook.active_sheet();
        clean_sheet.title("清洗后数据");
        
        std::cout << "=== 数据清洗处理 ===" << std::endl;
        
        // 设置清洗后的标题行
        clean_sheet["A1"].value("姓名").bold();
        clean_sheet["B1"].value("电话").bold();
        clean_sheet["C1"].value("邮箱").bold();
        clean_sheet["D1"].value("状态").bold();
        
        // 获取原始数据
        auto data_rows = source_sheet.rows(2, 1000)
            | views::filter([](const Row& row) { return !row.empty(); });
        
        int clean_row = 2;
        int processed_count = 0;
        int valid_count = 0;
        
        for (const auto& row : data_rows) {
            processed_count++;
            
            // 提取原始数据
            auto raw_name = row["A"].as<std::string>();
            auto raw_phone = row["B"].as<std::string>();
            auto raw_email = row["C"].as<std::string>();
            
            // 数据清洗
            auto clean_name = DataCleaner::clean_name(raw_name);
            auto clean_phone = DataCleaner::clean_phone(raw_phone);
            auto clean_email = DataCleaner::normalize_email(raw_email);
            
            // 数据验证
            bool is_valid = true;
            std::string status = "有效";
            
            if (clean_name.empty()) {
                status = "姓名为空";
                is_valid = false;
            } else if (clean_phone.length() < 10) {
                status = "电话号码无效";
                is_valid = false;
            } else if (!DataCleaner::is_valid_email(clean_email)) {
                status = "邮箱格式无效";
                is_valid = false;
            }
            
            // 写入清洗后的数据
            clean_sheet["A" + std::to_string(clean_row)].value(clean_name);
            clean_sheet["B" + std::to_string(clean_row)].value(clean_phone);
            clean_sheet["C" + std::to_string(clean_row)].value(clean_email);
            clean_sheet["D" + std::to_string(clean_row)].value(status);
            
            // 根据状态设置颜色
            if (is_valid) {
                clean_sheet["D" + std::to_string(clean_row)].color(Color::Green);
                valid_count++;
            } else {
                clean_sheet["D" + std::to_string(clean_row)].color(Color::Red);
            }
            
            clean_row++;
        }
        
        // 添加统计信息
        clean_sheet["F1"].value("数据统计").bold();
        clean_sheet["F2"].value("总处理数量");
        clean_sheet["G2"].value(processed_count);
        clean_sheet["F3"].value("有效数量");
        clean_sheet["G3"].value(valid_count);
        clean_sheet["F4"].value("有效率");
        clean_sheet["G4"].value(static_cast<double>(valid_count) / processed_count)
            .number_format("0.00%");
        
        // 设置格式
        clean_sheet.range("A1:D1").background_color(Color::LightBlue);
        clean_sheet.range("F1:G4").border(BorderType::All, BorderStyle::Thin);
        
        clean_workbook.save("cleaned_customer_data.xlsx");
        
        std::cout << "处理完成:" << std::endl;
        std::cout << "  总数量: " << processed_count << std::endl;
        std::cout << "  有效数量: " << valid_count << std::endl;
        std::cout << "  有效率: " << std::format("{:.1f}%", 
                     static_cast<double>(valid_count) / processed_count * 100) << std::endl;
        std::cout << "✅ 清洗后数据已保存到 cleaned_customer_data.xlsx" << std::endl;
        
    } catch (const TinaKitException& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 📝 小结

在这个教程中，你学会了：

✅ **Ranges 应用**：使用现代 C++ ranges 进行优雅的数据处理  
✅ **Concepts 约束**：通过 concepts 实现类型安全的泛型编程  
✅ **函数式编程**：编写声明式的数据处理管道  
✅ **数据清洗**：实现完整的数据清洗和验证流程  
✅ **现代 C++ 特性**：充分利用 C++20 的强大功能  

## 🚀 下一步

现在你已经掌握了现代 C++ 数据处理技术，接下来可以：

- 学习 [教程 4：异步处理大文件](04-async-large-files.md)
- 探索 [高级数据分析技术](../guides/advanced-data-analysis.md)
- 查看 [性能优化指南](../performance/optimization-guide.md)

---

有问题？在 [GitHub Discussions](https://github.com/your-username/TinaKit/discussions) 中与社区讨论现代 C++ 最佳实践。
