/**
 * @file excel_with_styles.cpp
 * @brief 使用样式创建格式化的 Excel 文档
 * @author TinaKit Team
 * @date 2025-01-08
 */

#include <tinakit/tinakit.hpp>
#include <iostream>
#include <format>
#include <vector>

using namespace tinakit;
using namespace tinakit::excel;

int main() {
    try {
        std::cout << "=== 创建带样式的 Excel 文档 ===" << std::endl;
        
        // 创建新工作簿
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        sheet.set_name("销售报表");

        // 注意：直接使用Cell的样式方法，不需要直接访问style_manager和shared_strings

        // 1. 设置标题（使用链式调用设置样式）
        sheet["A1"].value("销售报表 - 2025年1月")
                   .font("Arial", 16.0)
                   .bold(true)
                   .color(Color::White)
                   .background_color(Color::Blue);

        // 2. 设置表头
        std::vector<std::string> headers = {"产品名称", "单价", "数量", "总额", "备注"};
        Alignment center_align;
        center_align.horizontal = Alignment::Horizontal::Center;

        for (size_t i = 0; i < headers.size(); ++i) {
            sheet.cell(3, i + 1).value(headers[i])
                                .font("Calibri", 12.0)
                                .bold(true)
                                .background_color(Color::LightGray)
                                .align(center_align);
        }


        // 3. 填充数据
        struct Product {
            std::string name;
            double price;
            int quantity;
            std::string note;
        };

        std::vector<Product> products = {
            {"TinaKit Pro 许可证", 299.99, 10, "企业版"},
            {"TinaKit Standard 许可证", 99.99, 25, "标准版"},
            {"技术支持服务", 499.99, 5, "年度合同"},
            {"培训课程", 1999.99, 2, "现场培训"}
        };

        int row = 4;
        for (const auto& product : products) {
            // 产品名称
            sheet.cell(row, 1).value(product.name);

            // 单价（使用货币格式）
            sheet.cell(row, 2).value(product.price).number_format("$#,##0.00");

            // 数量
            sheet.cell(row, 3).value(product.quantity);

            // 总额（使用公式和货币格式）
            sheet.cell(row, 4)
                .formula(std::format("=B{}*C{}", row, row))
                .number_format("$#,##0.00");

            // 备注
            sheet.cell(row, 5).value(product.note);

            row++;
        }

        // 4. 总计行
        sheet.cell(row, 3).value("总计：").bold(true);
        sheet.cell(row, 4)
            .formula(std::format("=SUM(D4:D{})", row - 1))
            .number_format("$#,##0.00")
            .bold(true);

        // 5. 保存文件
        const std::string filename = "sales_report_with_styles.xlsx";
        workbook.save(filename);

        std::cout << "\n文件已保存为: " << filename << std::endl;
        
        std::cout << "\n=== 创建完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
