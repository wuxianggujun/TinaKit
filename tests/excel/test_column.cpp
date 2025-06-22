/**
 * @file test_column.cpp
 * @brief Column类测试
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "test_framework.hpp"
#include "tinakit/excel/workbook.hpp"
#include "tinakit/excel/worksheet.hpp"
#include "tinakit/excel/column.hpp"
#include "tinakit/excel/cell.hpp"

using namespace tinakit::excel;

using namespace tinakit::test;

// ========================================
// Column 基础功能测试
// ========================================

TEST_CASE(ColumnSystem, BasicColumnCreation) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        
        // 测试通过索引获取列
        auto col_a = sheet.column(1);  // A列
        auto col_b = sheet.column(2);  // B列
        
        ASSERT_TRUE(col_a.valid());
        ASSERT_TRUE(col_b.valid());
        ASSERT_EQ(col_a.index(), 1);
        ASSERT_EQ(col_b.index(), 2);
    }
    
TEST_CASE(ColumnSystem, ColumnByName) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();

    // 测试通过名称获取列
    auto col_a = sheet.column("A");
    auto col_b = sheet.column("B");
    auto col_aa = sheet.column("AA");

    ASSERT_TRUE(col_a.valid());
    ASSERT_TRUE(col_b.valid());
    ASSERT_TRUE(col_aa.valid());
    ASSERT_EQ(col_a.index(), 1);
    ASSERT_EQ(col_b.index(), 2);
    ASSERT_EQ(col_aa.index(), 27);  // AA = 27
}

TEST_CASE(ColumnSystem, DefaultColumnHandle) {
    Column default_col;
    ASSERT_FALSE(default_col.valid());
    ASSERT_EQ(default_col.index(), 0);
    ASSERT_TRUE(default_col.empty());
    ASSERT_EQ(default_col.size(), 0);
}

TEST_CASE(ColumnSystem, ColumnCopyAndAssignment) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        auto col1 = sheet.column(1);
        
        // 拷贝构造
        Column col2 = col1;
        ASSERT_TRUE(col2.valid());
        ASSERT_EQ(col2.index(), 1);
        
        // 赋值操作
        Column col3;
        col3 = col1;
        ASSERT_TRUE(col3.valid());
        ASSERT_EQ(col3.index(), 1);
}

TEST_CASE(ColumnSystem, ColumnWidthOperations) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        auto col = sheet.column(1);
        
        // 测试默认列宽
        double default_width = col.width();
        ASSERT_GT(default_width, 0.0);
        
        // 设置列宽
        col.set_width(15.0);
        ASSERT_EQ(col.width(), 15.0);
        
        // 链式调用
        col.width(20.0);
        ASSERT_EQ(col.width(), 20.0);
}

TEST_CASE(ColumnSystem, ColumnHiddenOperations) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 测试默认隐藏状态
    ASSERT_FALSE(col.hidden());

    // 设置隐藏
    col.set_hidden(true);
    ASSERT_TRUE(col.hidden());

    // 链式调用
    col.hidden(false);
    ASSERT_FALSE(col.hidden());
}

TEST_CASE(ColumnSystem, ColumnEmptyCheck) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        auto col = sheet.column(1);
        
        // 空列
        ASSERT_TRUE(col.empty());
        ASSERT_EQ(col.size(), 0);
        
        // 添加数据
        col[1].value("Test");
        ASSERT_FALSE(col.empty());
        ASSERT_EQ(col.size(), 1);
        
        // 添加更多数据
        col[3].value("More data");
        ASSERT_EQ(col.size(), 3);
}

TEST_CASE(ColumnSystem, ColumnSizeCalculation) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 在不同行添加数据
    col[1].value("Row 1");
    col[5].value("Row 5");
    col[3].value("Row 3");

    // 大小应该是最大行号
    ASSERT_EQ(col.size(), 5);
}

TEST_CASE(ColumnSystem, CellAccessByIndex) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 通过索引访问单元格
    auto cell1 = col[1];
    auto cell2 = col.cell(2);

    ASSERT_EQ(cell1.row(), 1);
    ASSERT_EQ(cell1.column(), 1);
    ASSERT_EQ(cell2.row(), 2);
    ASSERT_EQ(cell2.column(), 1);
}

TEST_CASE(ColumnSystem, CellValueOperations) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        auto col = sheet.column(1);
        
        // 设置和获取值
        col[1].value("Text");
        col[2].value(42);
        col[3].value(3.14);
        col[4].value(true);
        
        ASSERT_EQ(col[1].as<std::string>(), "Text");
        ASSERT_EQ(col[2].as<int>(), 42);
        ASSERT_EQ(col[3].as<double>(), 3.14);
        ASSERT_EQ(col[4].as<bool>(), true);
}

TEST_CASE(ColumnSystem, BasicIteration) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        auto col = sheet.column(1);
        
        // 添加一些数据
        col[1].value("A");
        col[2].value("B");
        col[3].value("C");
        
        // 迭代测试
        std::vector<std::string> values;
        for (auto cell : col) {
            if (!cell.empty()) {
                values.push_back(cell.as<std::string>());
            }
        }
        
        ASSERT_EQ(values.size(), 3);
        ASSERT_EQ(values[0], "A");
        ASSERT_EQ(values[1], "B");
        ASSERT_EQ(values[2], "C");
}

TEST_CASE(ColumnSystem, IteratorEquality) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    col[1].value("Test");

    auto it1 = col.begin();
    auto it2 = col.begin();
    auto end_it = col.end();

    ASSERT_TRUE(it1 == it2);
    ASSERT_FALSE(it1 == end_it);
    ASSERT_TRUE(it1 != end_it);
}

TEST_CASE(ColumnSystem, IteratorIncrement) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    col[1].value("First");
    col[2].value("Second");

    auto it = col.begin();
    ASSERT_EQ((*it).row(), 1);

    ++it;
    ASSERT_EQ((*it).row(), 2);

    auto it2 = it++;
    ASSERT_EQ((*it2).row(), 2);
    ASSERT_EQ((*it).row(), 3);
}

TEST_CASE(ColumnSystem, STLAlgorithmSupport) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        auto col = sheet.column(1);
        
        // 添加数据
        col[1].value(10);
        col[2].value(20);
        col[3].value(30);
        
        // 使用STL算法
        int sum = 0;
        std::for_each(col.begin(), col.end(), [&sum](const Cell& cell) {
            if (!cell.empty()) {
                sum += cell.as<int>();
            }
        });
        
    ASSERT_EQ(sum, 60);
}

TEST_CASE(ColumnSystem, BatchSetValues) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 批量设置值
    std::vector<Cell::CellValue> values = {
        std::string("A"),
        42,
        3.14,
        true
    };

    col.set_values(values, 1);

    ASSERT_EQ(col[1].as<std::string>(), "A");
    ASSERT_EQ(col[2].as<int>(), 42);
    ASSERT_EQ(col[3].as<double>(), 3.14);
    ASSERT_EQ(col[4].as<bool>(), true);
}

TEST_CASE(ColumnSystem, BatchSetValuesWithStartRow) {
        auto workbook = Workbook::create();
        auto sheet = workbook.active_sheet();
        auto col = sheet.column(1);
        
        // 从第3行开始批量设置值
        std::vector<Cell::CellValue> values = {
            std::string("Start"), 
            std::string("Middle"), 
            std::string("End")
        };
        
        col.set_values(values, 3);
        
        ASSERT_TRUE(col[1].empty());
        ASSERT_TRUE(col[2].empty());
        ASSERT_EQ(col[3].as<std::string>(), "Start");
        ASSERT_EQ(col[4].as<std::string>(), "Middle");
    ASSERT_EQ(col[5].as<std::string>(), "End");
}

TEST_CASE(ColumnSystem, BatchGetValues) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 设置一些值
    col[1].value("A");
    col[2].value("B");
    col[3].value("C");

    // 批量获取值
    auto values = col.get_values(1, 3);

    ASSERT_EQ(values.size(), 3);
    ASSERT_EQ(std::get<std::string>(values[0]), "A");
    ASSERT_EQ(std::get<std::string>(values[1]), "B");
    ASSERT_EQ(std::get<std::string>(values[2]), "C");
}

TEST_CASE(ColumnSystem, BatchGetValuesWithRange) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 设置一些值
    col[2].value("Start");
    col[3].value("Middle");
    col[4].value("End");

    // 从第2行开始获取2个值
    auto values = col.get_values(2, 2);

    ASSERT_EQ(values.size(), 2);
    ASSERT_EQ(std::get<std::string>(values[0]), "Start");
    ASSERT_EQ(std::get<std::string>(values[1]), "Middle");
}

TEST_CASE(ColumnSystem, ColumnClear) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 添加一些数据
    col[1].value("Test1");
    col[2].value("Test2");
    col[3].value("Test3");

    ASSERT_FALSE(col.empty());
    ASSERT_EQ(col.size(), 3);

    // 清空列
    col.clear();

    ASSERT_TRUE(col.empty());
    ASSERT_TRUE(col[1].empty());
    ASSERT_TRUE(col[2].empty());
    ASSERT_TRUE(col[3].empty());
}

TEST_CASE(ColumnSystem, BatchOperationsWithMonostate) {
    auto workbook = Workbook::create();
    auto sheet = workbook.active_sheet();
    auto col = sheet.column(1);

    // 设置一些值，包含空值
    std::vector<Cell::CellValue> values = {
        std::string("A"),
        std::monostate{},
        std::string("C")
    };

    col.set_values(values, 1);

    ASSERT_EQ(col[1].as<std::string>(), "A");
    ASSERT_TRUE(col[2].empty());
    ASSERT_EQ(col[3].as<std::string>(), "C");
}

TEST_CASE(ColumnSystem, BatchOperationsInvalidHandle) {
    Column invalid_col;

    std::vector<Cell::CellValue> values = {std::string("Test")};

    ASSERT_THROWS(invalid_col.set_values(values), std::runtime_error);
    ASSERT_THROWS(invalid_col.get_values(), std::runtime_error);
    ASSERT_THROWS(invalid_col.clear(), std::runtime_error);
}

TEST_CASE(ColumnSystem, InvalidHandleOperations) {
    Column invalid_col;

    ASSERT_THROWS(invalid_col.set_width(10.0), std::runtime_error);
    ASSERT_THROWS(invalid_col.set_hidden(true), std::runtime_error);
}
