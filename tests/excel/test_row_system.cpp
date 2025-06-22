/**
 * @file test_row_system.cpp
 * @brief Row类系统测试
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "test_framework.hpp"
#include "tinakit/tinakit.hpp"
#include <algorithm>
#include <ranges>

using namespace tinakit;
using namespace tinakit::excel;
using namespace tinakit::test;

// ========================================
// Row 基础功能测试
// ========================================

TEST_CASE(RowSystem, BasicRowCreation) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);
    ASSERT_TRUE(row.valid());
    ASSERT_EQ(1u, row.index());
}

TEST_CASE(RowSystem, DefaultRowHandle) {
    Row default_row;
    ASSERT_FALSE(default_row.valid());
    ASSERT_EQ(0u, default_row.index());
}

TEST_CASE(RowSystem, RowCopyAndAssignment) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row1 = sheet.row(1);
    auto row2 = row1; // 拷贝构造
    auto row3 = sheet.row(2);
    row3 = row1; // 拷贝赋值

    ASSERT_TRUE(row1.valid());
    ASSERT_TRUE(row2.valid());
    ASSERT_TRUE(row3.valid());
    ASSERT_EQ(row1.index(), row2.index());
    ASSERT_EQ(row1.index(), row3.index());
}

// ========================================
// Row 属性测试
// ========================================

TEST_CASE(RowSystem, RowHeightOperations) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 测试默认行高
    double default_height = row.height();
    ASSERT_EQ(15.0, default_height);

    // 测试设置行高
    row.set_height(25.0);
    ASSERT_EQ(25.0, row.height());

    // 测试链式调用
    row.height(30.0);
    ASSERT_EQ(30.0, row.height());
}

TEST_CASE(RowSystem, RowEmptyCheck) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto empty_row = sheet.row(1);
    ASSERT_TRUE(empty_row.empty());

    // 添加数据后不应为空
    empty_row[1].value("test");
    ASSERT_FALSE(empty_row.empty());
}

TEST_CASE(RowSystem, RowSizeCalculation) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);
    ASSERT_EQ(0u, row.size());

    // 添加数据
    row[1].value("A");
    row[3].value("C");
    row[5].value("E");

    // 大小应该是最后一个非空单元格的列索引
    ASSERT_EQ(5u, row.size());
}

// ========================================
// Row 单元格访问测试
// ========================================

TEST_CASE(RowSystem, CellAccessByIndex) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 按索引访问
    auto cell1 = row[1];
    auto cell2 = row[2];

    // 验证单元格位置
    ASSERT_EQ(1u, cell1.row());
    ASSERT_EQ(1u, cell1.column());
    ASSERT_EQ(1u, cell2.row());
    ASSERT_EQ(2u, cell2.column());
}

TEST_CASE(RowSystem, CellAccessByColumnName) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 按列名访问
    auto cellA = row["A"];
    auto cellB = row["B"];
    auto cellZ = row["Z"];
    auto cellAA = row["AA"];

    // 验证列索引
    ASSERT_EQ(1u, cellA.column());
    ASSERT_EQ(2u, cellB.column());
    ASSERT_EQ(26u, cellZ.column());
    ASSERT_EQ(27u, cellAA.column());
}

TEST_CASE(RowSystem, CellValueOperations) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 设置不同类型的值
    row[1].value("文本");
    row[2].value(42);
    row[3].value(3.14);
    row[4].value(true);

    // 验证值
    ASSERT_EQ("文本", row[1].as<std::string>());
    ASSERT_EQ(42, row[2].as<int>());
    ASSERT_EQ(3.14, row[3].as<double>());
    ASSERT_EQ(true, row[4].as<bool>());
}

// ========================================
// Row 迭代器测试
// ========================================

TEST_CASE(RowSystem, BasicIteration) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 填充数据
    row[1].value("A");
    row[2].value("B");
    row[3].value("C");

    // 测试迭代器
    std::vector<std::string> values;
    for (auto cell : row) {
        if (!cell.empty()) {
            values.push_back(cell.as<std::string>());
        }
    }

    ASSERT_EQ(3u, values.size());
    ASSERT_EQ("A", values[0]);
    ASSERT_EQ("B", values[1]);
    ASSERT_EQ("C", values[2]);
}

TEST_CASE(RowSystem, IteratorEquality) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);
    row[1].value("test");

    auto it1 = row.begin();
    auto it2 = row.begin();
    auto it3 = row.end();

    ASSERT_TRUE(it1 == it2);
    ASSERT_FALSE(it1 == it3);
    ASSERT_TRUE(it1 != it3);
}

TEST_CASE(RowSystem, IteratorIncrement) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);
    row[1].value("A");
    row[2].value("B");

    auto it = row.begin();
    auto cell1 = *it;
    ++it;
    auto cell2 = *it;

    ASSERT_EQ(1u, cell1.column());
    ASSERT_EQ(2u, cell2.column());
}

// ========================================
// Row STL算法支持测试
// ========================================

TEST_CASE(RowSystem, STLAlgorithmSupport) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 填充数据
    row[1].value("Apple");
    row[2].value("");  // 空值
    row[3].value("Banana");
    row[4].value("Cherry");

    // 使用STL算法计算非空单元格数量
    auto non_empty_count = std::count_if(row.begin(), row.end(),
        [](const Cell& cell) { return !cell.empty(); });

    ASSERT_EQ(3, non_empty_count);
}

// ========================================
// Row Ranges支持测试
// ========================================

TEST_CASE(RowSystem, RangesSupport) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 填充数字数据
    row[1].value(10);
    row[2].value(20);
    row[3].value(5);
    row[4].value(30);

    // 使用ranges过滤大于15的值
    auto large_values = row
        | std::views::filter([](const Cell& cell) {
            return !cell.empty() && cell.as<int>() > 15;
        })
        | std::views::transform([](const Cell& cell) {
            return cell.as<int>();
        });

    std::vector<int> result(large_values.begin(), large_values.end());

    ASSERT_EQ(2u, result.size());
    ASSERT_EQ(20, result[0]);
    ASSERT_EQ(30, result[1]);
}

TEST_CASE(RowSystem, AsTemplateMethod) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    row[1].value("Hello");
    row[2].value(42);
    row[3].value(3.14);

    // 测试as模板方法
    ASSERT_EQ("Hello", row.as<std::string>(1));
    ASSERT_EQ(42, row.as<int>(2));
    ASSERT_EQ(3.14, row.as<double>(3));
}

// ========================================
// Row 批量操作测试
// ========================================

TEST_CASE(RowSystem, BatchSetValues) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 测试批量设置值
    std::vector<Cell::CellValue> values = {
        std::string("姓名"),
        std::string("年龄"),
        42,
        3.14,
        true
    };

    row.set_values(values);

    // 验证设置的值
    ASSERT_EQ("姓名", row[1].as<std::string>());
    ASSERT_EQ("年龄", row[2].as<std::string>());
    ASSERT_EQ(42, row[3].as<int>());
    ASSERT_EQ(3.14, row[4].as<double>());
    ASSERT_EQ(true, row[5].as<bool>());
    ASSERT_EQ(5u, row.size());
}

TEST_CASE(RowSystem, BatchSetValuesWithStartColumn) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 从第3列开始设置值
    std::vector<Cell::CellValue> values = {
        std::string("C列"),
        std::string("D列"),
        std::string("E列")
    };

    row.set_values(values, 3);

    // 验证值设置在正确的位置
    ASSERT_TRUE(row[1].empty());
    ASSERT_TRUE(row[2].empty());
    ASSERT_EQ("C列", row[3].as<std::string>());
    ASSERT_EQ("D列", row[4].as<std::string>());
    ASSERT_EQ("E列", row[5].as<std::string>());
    ASSERT_EQ(5u, row.size());
}

TEST_CASE(RowSystem, BatchGetValues) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 先设置一些值
    row[1].value("测试1");
    row[2].value(100);
    row[3].value(2.5);
    row[4].value(false);

    // 获取所有值
    auto values = row.get_values();

    ASSERT_EQ(4u, values.size());

    // 验证获取的值
    ASSERT_TRUE(std::holds_alternative<std::string>(values[0]));
    ASSERT_EQ("测试1", std::get<std::string>(values[0]));

    ASSERT_TRUE(std::holds_alternative<int>(values[1]));
    ASSERT_EQ(100, std::get<int>(values[1]));

    ASSERT_TRUE(std::holds_alternative<double>(values[2]));
    ASSERT_EQ(2.5, std::get<double>(values[2]));

    ASSERT_TRUE(std::holds_alternative<bool>(values[3]));
    ASSERT_EQ(false, std::get<bool>(values[3]));
}

TEST_CASE(RowSystem, BatchGetValuesWithRange) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 设置测试数据
    for (int i = 1; i <= 10; ++i) {
        row[i].value("值" + std::to_string(i));
    }

    // 获取第3-6列的值
    auto values = row.get_values(3, 4);

    ASSERT_EQ(4u, values.size());
    ASSERT_EQ("值3", std::get<std::string>(values[0]));
    ASSERT_EQ("值4", std::get<std::string>(values[1]));
    ASSERT_EQ("值5", std::get<std::string>(values[2]));
    ASSERT_EQ("值6", std::get<std::string>(values[3]));
}

TEST_CASE(RowSystem, RowClear) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 先填充数据
    row[1].value("数据1");
    row[2].value("数据2");
    row[3].value("数据3");

    ASSERT_FALSE(row.empty());
    ASSERT_EQ(3u, row.size());

    // 清空行
    row.clear();

    // 验证行已清空
    ASSERT_TRUE(row.empty());
    ASSERT_EQ(0u, row.size());
    ASSERT_TRUE(row[1].empty());
    ASSERT_TRUE(row[2].empty());
    ASSERT_TRUE(row[3].empty());
}

TEST_CASE(RowSystem, BatchOperationsWithMonostate) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    auto row = sheet.row(1);

    // 测试包含空值的批量设置
    std::vector<Cell::CellValue> values = {
        std::string("有值"),
        std::monostate{},  // 空值
        std::string("也有值"),
        std::monostate{}   // 空值
    };

    row.set_values(values);

    // 验证结果
    ASSERT_EQ("有值", row[1].as<std::string>());
    ASSERT_TRUE(row[2].empty());
    ASSERT_EQ("也有值", row[3].as<std::string>());
    ASSERT_TRUE(row[4].empty());
    ASSERT_EQ(3u, row.size()); // 最后一个非空单元格在第3列
}

// ========================================
// Row 批量操作错误处理测试
// ========================================

TEST_CASE(RowSystem, BatchOperationsInvalidHandle) {
    Row invalid_row;

    std::vector<Cell::CellValue> values = {std::string("test")};

    ASSERT_THROWS(invalid_row.set_values(values), std::runtime_error);
    ASSERT_THROWS(invalid_row.get_values(), std::runtime_error);
    ASSERT_THROWS(invalid_row.clear(), std::runtime_error);
}

// ========================================
// Row 错误处理测试
// ========================================

TEST_CASE(RowSystem, InvalidHandleOperations) {
    Row invalid_row;

    ASSERT_THROWS(invalid_row.set_height(20.0), std::runtime_error);
    ASSERT_THROWS(invalid_row[1], std::runtime_error);
}

TEST_CASE(RowSystem, InvalidColumnName) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto row = sheet.row(1);

    // 测试无效列名会抛出异常
    ASSERT_THROWS(row[""], InvalidCellAddressException);
    ASSERT_THROWS(row["123"], InvalidCellAddressException);
}

// ========================================
// RowRange 测试
// ========================================

TEST_CASE(RowSystem, RowRangeBasicOperations) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 填充一些测试数据
    sheet.row(1)[1].value("Row1");
    sheet.row(2)[1].value("Row2");
    sheet.row(3)[1].value("Row3");

    // 测试行范围
    auto row_range = sheet.rows(1, 3);
    ASSERT_EQ(3u, row_range.size());
    ASSERT_FALSE(row_range.empty());

    // 测试空范围
    auto empty_range = sheet.rows(5, 4); // start > end
    ASSERT_TRUE(empty_range.empty());
    ASSERT_EQ(0u, empty_range.size());
}

TEST_CASE(RowSystem, RowRangeIteration) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 填充测试数据
    for (int i = 1; i <= 5; ++i) {
        sheet.row(i)[1].value("Row" + std::to_string(i));
    }

    // 测试迭代器
    auto row_range = sheet.rows(2, 4);
    std::vector<std::string> values;

    for (auto row : row_range) {
        values.push_back(row[1].as<std::string>());
    }

    ASSERT_EQ(3u, values.size());
    ASSERT_EQ("Row2", values[0]);
    ASSERT_EQ("Row3", values[1]);
    ASSERT_EQ("Row4", values[2]);
}

TEST_CASE(RowSystem, RowRangeSTLAlgorithms) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 填充数字数据
    for (int i = 1; i <= 10; ++i) {
        sheet.row(i)[1].value(i * 10);
    }

    auto row_range = sheet.rows(1, 10);

    // 使用STL算法计算大于50的行数
    auto count = std::count_if(row_range.begin(), row_range.end(),
        [](const Row& row) {
            return row[1].as<int>() > 50;
        });

    ASSERT_EQ(5, count); // 60, 70, 80, 90, 100
}

TEST_CASE(RowSystem, RowRangeRangesSupport) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 填充测试数据
    for (int i = 1; i <= 6; ++i) {
        sheet.row(i)[1].value(i);
        sheet.row(i)[2].value(i % 2 == 0 ? "偶数" : "奇数");
    }

    auto row_range = sheet.rows(1, 6);

    // 使用ranges过滤偶数行
    auto even_rows = row_range
        | std::views::filter([](const Row& row) {
            return row[1].as<int>() % 2 == 0;
        })
        | std::views::transform([](const Row& row) {
            return row[1].as<int>();
        });

    std::vector<int> result(even_rows.begin(), even_rows.end());

    ASSERT_EQ(3u, result.size());
    ASSERT_EQ(2, result[0]);
    ASSERT_EQ(4, result[1]);
    ASSERT_EQ(6, result[2]);
}
