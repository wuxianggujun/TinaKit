#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <cctype>
#include <map>
#include <memory>

// 简化的测试版本
struct Position {
    std::size_t row;
    std::size_t column;
    
    Position(std::size_t r, std::size_t c) : row(r), column(c) {
        if (r == 0 || c == 0) {
            throw std::invalid_argument("Position indices must be 1-based (row and column must be >= 1)");
        }
    }
    
    static Position from_address(const std::string& address) {
        std::cout << "Parsing address: '" << address << "'" << std::endl;
        
        // 简单的地址解析
        std::size_t i = 0;
        std::size_t col = 0;
        
        // 解析列（字母部分）
        while (i < address.length() && std::isalpha(address[i])) {
            col = col * 26 + (std::toupper(address[i]) - 'A' + 1);
            i++;
        }
        
        std::cout << "Parsed column: " << col << std::endl;
        
        // 解析行（数字部分）
        std::size_t row = 0;
        if (i < address.length()) {
            row = std::stoull(address.substr(i));
        }
        
        std::cout << "Parsed row: " << row << std::endl;
        
        return Position(row, col);
    }
};

std::string column_number_to_name(std::size_t column) {
    if (column == 0) {
        throw std::invalid_argument("Column number must be 1-based");
    }
    
    std::string result;
    while (column > 0) {
        column--; // 转换为 0-based
        result = static_cast<char>('A' + column % 26) + result;
        column /= 26;
    }
    
    return result;
}

// 模拟 Cell 类
class Cell {
public:
    Cell(std::size_t row, std::size_t column) : row_(row), column_(column) {
        std::cout << "Creating Cell at row=" << row << ", column=" << column << std::endl;
    }

    std::string address() const {
        std::cout << "Getting address for Cell at row=" << row_ << ", column=" << column_ << std::endl;
        return column_number_to_name(column_) + std::to_string(row_);
    }

    Cell& value(const std::string& val) {
        std::cout << "Setting value '" << val << "' for cell " << address() << std::endl;
        return *this;
    }

private:
    std::size_t row_, column_;
};

// 模拟 Worksheet 类
class Worksheet {
public:
    Cell& operator[](const std::string& address) {
        std::cout << "Accessing cell with address: '" << address << "'" << std::endl;
        auto pos = Position::from_address(address);

        // 查找或创建 Cell
        auto key = std::make_pair(pos.row, pos.column);
        auto it = cells_.find(key);
        if (it == cells_.end()) {
            std::cout << "Creating new cell at row=" << pos.row << ", column=" << pos.column << std::endl;
            auto cell = std::make_unique<Cell>(pos.row, pos.column);
            auto& cell_ref = *cell;
            cells_[key] = std::move(cell);
            return cell_ref;
        }
        return *it->second;
    }

private:
    std::map<std::pair<std::size_t, std::size_t>, std::unique_ptr<Cell>> cells_;
};

int main() {
    try {
        std::cout << "=== Position Address Parsing Test ===" << std::endl;

        // 测试一些常见的地址
        std::vector<std::string> addresses = {"A1", "B2", "C3", "AA1", "AB10"};

        for (const auto& addr : addresses) {
            try {
                auto pos = Position::from_address(addr);
                std::cout << "Address: " << addr << " -> Row: " << pos.row << ", Column: " << pos.column << std::endl;
                std::cout << "Column name: " << column_number_to_name(pos.column) << std::endl;
                std::cout << "---" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Error parsing " << addr << ": " << e.what() << std::endl;
                std::cout << "---" << std::endl;
            }
        }

        std::cout << "\n=== Worksheet Cell Access Test ===" << std::endl;

        // 模拟演示代码的执行
        Worksheet sheet;

        try {
            std::cout << "Testing sheet[\"A1\"]..." << std::endl;
            auto& cell_a1 = sheet["A1"];
            cell_a1.value("TinaKit Excel Library");

            std::cout << "Testing sheet[\"A3\"]..." << std::endl;
            auto& cell_a3 = sheet["A3"];
            cell_a3.value("Bold Text");

            std::cout << "All tests passed!" << std::endl;

        } catch (const std::exception& e) {
            std::cout << "Error in worksheet test: " << e.what() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
