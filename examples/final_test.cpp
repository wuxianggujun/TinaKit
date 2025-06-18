/**
 * @file final_test.cpp
 * @brief 最终测试 - 验证所有修复
 */

#include <iostream>
#include <filesystem>
#include "tinakit/tinakit.hpp"
#include "tinakit/core/openxml_archiver.hpp"

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::async;

// 测试创建和保存
Task<void> test_create_and_save() {
    std::cout << "1. 创建和保存测试" << std::endl;
    std::cout << "==================" << std::endl;
    
    auto workbook = Workbook::create();
    auto& sheet = workbook.active_sheet();
    
    // 添加数据
    sheet["A1"].value("姓名");
    sheet["B1"].value("年龄");
    sheet["C1"].value("城市");
    
    sheet["A2"].value("张三");
    sheet["B2"].value(25);
    sheet["C2"].value("北京");
    
    sheet["A3"].value("李四");
    sheet["B3"].value(30);
    sheet["C3"].value("上海");
    
    // 保存文件
    co_await workbook.save_async("final_test.xlsx");
    std::cout << "✓ 文件创建成功: final_test.xlsx" << std::endl;
    
    // 测试保存后立即读取（之前会失败）
    std::cout << "\n2. 测试保存后立即读取" << std::endl;
    std::cout << "======================" << std::endl;
    
    try {
        auto& read_sheet = workbook[0];
        std::cout << "工作表名称: " << read_sheet.name() << std::endl;
        std::cout << "A1: " << read_sheet["A1"].to_string() << std::endl;
        std::cout << "✓ 保存后读取成功！" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ 保存后读取失败: " << e.what() << std::endl;
    }
}

// 测试重新打开
Task<void> test_reopen() {
    std::cout << "\n3. 重新打开文件测试" << std::endl;
    std::cout << "===================" << std::endl;

    try {
        // 先检查文件是否存在
        if (!std::filesystem::exists("final_test.xlsx")) {
            std::cout << "❌ 文件不存在: final_test.xlsx" << std::endl;
            co_return;
        }

        std::cout << "文件大小: " << std::filesystem::file_size("final_test.xlsx") << " 字节" << std::endl;

        // 尝试用OpenXmlArchiver直接打开文件
        std::cout << "尝试用OpenXmlArchiver打开文件..." << std::endl;
        auto archiver = co_await core::OpenXmlArchiver::open_from_file("final_test.xlsx");

        // 列出文件
        auto files = co_await archiver.list_files();
        std::cout << "ZIP文件中包含 " << files.size() << " 个文件:" << std::endl;
        for (const auto& file : files) {
            std::cout << "  - " << file << std::endl;
        }

        // 检查关键文件
        bool has_workbook_rels = co_await archiver.has_file("xl/_rels/workbook.xml.rels");
        std::cout << "xl/_rels/workbook.xml.rels 存在: " << (has_workbook_rels ? "是" : "否") << std::endl;

        if (has_workbook_rels) {
            std::cout << "尝试读取 workbook.xml.rels..." << std::endl;
            auto content = co_await archiver.read_file("xl/_rels/workbook.xml.rels");
            std::string xml_content(reinterpret_cast<const char*>(content.data()), content.size());
            std::cout << "内容:\n" << xml_content << std::endl;
        }

        std::cout << "OpenXmlArchiver 测试成功，现在尝试用Workbook打开..." << std::endl;

        auto workbook = co_await Workbook::open_async("final_test.xlsx");
        std::cout << "✓ 文件打开成功" << std::endl;

        auto& sheet = workbook[0];
        std::cout << "工作表数量: " << workbook.sheet_count() << std::endl;

        // 读取数据
        std::cout << "\n读取的数据：" << std::endl;
        for (std::size_t r = 1; r <= 3; ++r) {
            for (std::size_t c = 1; c <= 3; ++c) {
                std::cout << sheet.cell(r, c).to_string() << "\t";
            }
            std::cout << std::endl;
        }

        std::cout << "\n✓ 所有数据读取成功！" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "❌ 打开文件失败: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "TinaKit 最终测试" << std::endl;
    std::cout << "================" << std::endl << std::endl;
    
    try {
        // 执行测试
        sync_wait(test_create_and_save());
        sync_wait(test_reopen());
        
        std::cout << "\n4. 验证文件结构" << std::endl;
        std::cout << "================" << std::endl;
        
        // 解压并检查
        std::system("powershell -Command \"Copy-Item final_test.xlsx final_test.zip -Force; Expand-Archive -Path final_test.zip -DestinationPath final_test_extracted -Force\"");
        
        std::cout << "\n检查单元格样式属性..." << std::endl;
        std::system("powershell -Command \"Select-String -Path final_test_extracted\\xl\\worksheets\\sheet1.xml -Pattern 's=\\\"0\\\"' | Select-Object -First 3\"");
        
        std::cout << "\n✅ 测试完成！请用 Excel/WPS 打开 final_test.xlsx 验证内容是否正确显示" << std::endl;
        std::cout << "文件路径: " << std::filesystem::absolute("final_test.xlsx") << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ 测试失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}