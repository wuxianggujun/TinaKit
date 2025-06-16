/**
 * TinaKit Dream Code - 理想的用户体验设计
 * 
 * 这个文件展示了我们希望 TinaKit 用户能够编写的最直观、最优雅的代码。
 * 不考虑实现细节，专注于用户体验和 API 设计。
 */

#include <tinakit/tinakit.hpp>
#include <iostream>
#include <vector>
#include <ranges>
#include <format>

using namespace tinakit;
using namespace std::ranges;

// ============================================================================
// 场景 1: 简单的 Excel 读取 - 最常见的用例
// ============================================================================
void simple_excel_reading() {
    std::cout << "=== 简单 Excel 读取 ===" << std::endl;
    
    // 理想：一行代码打开文件
    auto workbook = Excel::open("data/sales.xlsx");
    
    // 理想：直观的工作表访问
    auto sheet = workbook["销售数据"];  // 按名称访问
    // 或者
    auto sheet2 = workbook[0];  // 按索引访问
    
    // 理想：像数组一样访问单元格
    std::cout << "A1 的值: " << sheet["A1"] << std::endl;
    std::cout << "B2 的值: " << sheet["B2"].as<double>() << std::endl;
    
    // 理想：范围访问，返回可迭代的对象
    for (auto cell : sheet["A1:C10"]) {
        if (!cell.empty()) {
            std::cout << cell.address() << ": " << cell << std::endl;
        }
    }
}

// ============================================================================
// 场景 2: 现代 C++ 风格的数据处理
// ============================================================================
void modern_cpp_data_processing() {
    std::cout << "=== 现代 C++ 数据处理 ===" << std::endl;
    
    auto workbook = Excel::open("data/employees.xlsx");
    auto sheet = workbook["员工信息"];
    
    // 理想：使用 ranges 进行数据转换和过滤
    auto high_salary_employees = sheet.rows(2, 100)  // 跳过标题行
        | views::filter([](const auto& row) { 
            return row["C"].as<double>() > 50000;  // 薪资列
        })
        | views::transform([](const auto& row) {
            return std::format("{}: {}", 
                row["A"].as<std::string>(),  // 姓名
                row["C"].as<double>()        // 薪资
            );
        })
        | to<std::vector>();
    
    for (const auto& employee : high_salary_employees) {
        std::cout << employee << std::endl;
    }
}

// ============================================================================
// 场景 3: 创建和写入 Excel 文件
// ============================================================================
void create_excel_file() {
    std::cout << "=== 创建 Excel 文件 ===" << std::endl;
    
    // 理想：流畅的构建器模式
    auto workbook = Excel::create()
        .add_sheet("销售报表")
        .add_sheet("统计数据");
    
    auto sheet = workbook["销售报表"];
    
    // 理想：链式调用设置样式
    sheet["A1"].value("产品名称").bold().font_size(14);
    sheet["B1"].value("销售额").bold().font_size(14);
    sheet["C1"].value("增长率").bold().font_size(14).background_color(Color::LightBlue);
    
    // 理想：批量数据写入
    std::vector<std::tuple<std::string, double, double>> data = {
        {"iPhone", 1000000.0, 0.15},
        {"iPad", 500000.0, 0.08},
        {"MacBook", 800000.0, 0.12}
    };
    
    sheet.write_data("A2", data);
    
    // 理想：简单的公式设置
    sheet["D2"].formula("=B2*C2");
    sheet["D3"].formula("=B3*C3");
    sheet["D4"].formula("=B4*C4");
    
    // 理想：一行保存
    workbook.save("output/sales_report.xlsx");
}

// ============================================================================
// 场景 4: 异步处理大文件
// ============================================================================
Task<void> async_large_file_processing() {
    std::cout << "=== 异步大文件处理 ===" << std::endl;
    
    // 理想：异步打开大文件
    auto workbook = co_await Excel::open_async("data/large_dataset.xlsx");
    
    // 理想：进度回调
    workbook.on_progress([](double progress) {
        std::cout << std::format("处理进度: {:.1f}%\n", progress * 100);
    });
    
    auto sheet = workbook[0];
    
    // 理想：流式处理，避免内存溢出
    co_await sheet.process_rows_async([](const Row& row) -> Task<void> {
        // 异步处理每一行
        if (row["A"].as<std::string>().starts_with("重要")) {
            co_await save_to_database(row);
        }
        co_return;
    });
    
    std::cout << "大文件处理完成！" << std::endl;
}

// ============================================================================
// 场景 5: Word 文档处理
// ============================================================================
void word_document_processing() {
    std::cout << "=== Word 文档处理 ===" << std::endl;
    
    // 理想：统一的 API 风格
    auto doc = Word::open("templates/report_template.docx");
    
    // 理想：模板变量替换
    doc.replace_placeholder("{{title}}", "2024年度销售报告");
    doc.replace_placeholder("{{date}}", "2024-12-16");
    doc.replace_placeholder("{{author}}", "张三");
    
    // 理想：表格操作
    auto table = doc.tables()[0];
    table.cell(1, 1).text("Q1销售额");
    table.cell(1, 2).text("1,000,000");
    
    // 理想：插入图片
    doc.insert_image("charts/sales_chart.png", 
                     Position::after_paragraph(5));
    
    doc.save("output/annual_report.docx");
}

// ============================================================================
// 场景 6: 格式化和样式
// ============================================================================
void formatting_and_styling() {
    std::cout << "=== 格式化和样式 ===" << std::endl;
    
    auto workbook = Excel::create();
    auto sheet = workbook.add_sheet("样式示例");
    
    // 理想：链式样式设置
    sheet["A1"]
        .value("标题")
        .font("Arial", 16)
        .bold()
        .italic()
        .color(Color::Red)
        .background_color(Color::Yellow)
        .align(Alignment::Center)
        .border(Border::All, BorderStyle::Thick);
    
    // 理想：范围样式设置
    sheet["A1:C1"]
        .background_color(Color::LightGray)
        .bold();
    
    // 理想：条件格式
    sheet["B2:B10"]
        .conditional_format()
        .when_greater_than(100)
        .background_color(Color::Green);
    
    // 理想：数字格式
    sheet["C2:C10"]
        .number_format("¥#,##0.00");  // 货币格式
    
    workbook.save("output/styled_workbook.xlsx");
}

// ============================================================================
// 场景 7: 错误处理和验证
// ============================================================================
void error_handling_and_validation() {
    std::cout << "=== 错误处理和验证 ===" << std::endl;
    
    try {
        auto workbook = Excel::open("nonexistent.xlsx");
    } catch (const FileNotFoundException& e) {
        std::cout << "文件未找到: " << e.what() << std::endl;
    }
    
    try {
        auto workbook = Excel::open("corrupted.xlsx");
        auto sheet = workbook[0];
        
        // 理想：类型安全的值获取
        auto value = sheet["A1"].try_as<int>();
        if (value) {
            std::cout << "整数值: " << *value << std::endl;
        } else {
            std::cout << "A1 不是有效的整数" << std::endl;
        }
        
    } catch (const CorruptedFileException& e) {
        std::cout << "文件损坏: " << e.what() << std::endl;
    } catch (const ParseException& e) {
        std::cout << "解析错误: " << e.what() 
                  << " 位置: " << e.location() << std::endl;
    }
}

// ============================================================================
// 场景 8: 插件和扩展
// ============================================================================
void plugin_and_extension() {
    std::cout << "=== 插件和扩展 ===" << std::endl;
    
    // 理想：注册自定义格式处理器
    TinaKit::register_format<CustomXmlFormat>(".cxml");
    
    // 理想：使用自定义格式
    auto doc = TinaKit::open("data/custom.cxml");
    
    // 理想：添加自定义函数
    Excel::register_function("MYSUM", [](const std::vector<double>& args) {
        return std::accumulate(args.begin(), args.end(), 0.0);
    });
    
    auto workbook = Excel::create();
    auto sheet = workbook.add_sheet("自定义函数");
    sheet["A1"].formula("=MYSUM(1,2,3,4,5)");
}

// ============================================================================
// 主函数 - 展示所有场景
// ============================================================================
int main() {
    std::cout << "TinaKit Dream Code - 理想用户体验展示\n" << std::endl;
    
    try {
        simple_excel_reading();
        std::cout << std::endl;
        
        modern_cpp_data_processing();
        std::cout << std::endl;
        
        create_excel_file();
        std::cout << std::endl;
        
        // 异步示例需要协程运行时
        // auto async_task = async_large_file_processing();
        // async_task.wait();
        
        word_document_processing();
        std::cout << std::endl;
        
        formatting_and_styling();
        std::cout << std::endl;
        
        error_handling_and_validation();
        std::cout << std::endl;
        
        plugin_and_extension();
        std::cout << std::endl;
        
        std::cout << "✅ 所有理想场景展示完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

// ============================================================================
// 辅助函数声明（实际实现中需要）
// ============================================================================
Task<void> save_to_database(const Row& row) {
    // 模拟数据库保存
    co_return;
}
