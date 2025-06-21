/**
 * @file formula_engine.cpp
 * @brief Excel 公式计算引擎实现
 * @author TinaKit Team
 * @date 2025-6-21
 */

#include "tinakit/excel/formula_engine.hpp"
#include "tinakit/internal/workbook_impl.hpp"
#include "tinakit/internal/worksheet_impl.hpp"
#include "tinakit/internal/coordinate_utils.hpp"
#include "tinakit/core/types.hpp"
#include <regex>
#include <algorithm>
#include <numeric>
#include <cctype>
#include <cmath>
#include <limits>

// 使用内部命名空间的类型
using tinakit::internal::cell_data;

namespace tinakit::excel {

// ========================================
// FormulaEngine 构造和初始化
// ========================================

FormulaEngine::FormulaEngine(tinakit::internal::workbook_impl* workbook_impl)
    : workbook_impl_(workbook_impl) {
    register_builtin_functions();
}

void FormulaEngine::register_builtin_functions() {
    // 注册内置函数
    functions_["SUM"] = [this](const std::vector<FormulaResult>& args) {
        return sum_function(args);
    };
    
    functions_["AVERAGE"] = [this](const std::vector<FormulaResult>& args) {
        return average_function(args);
    };
    
    functions_["COUNT"] = [this](const std::vector<FormulaResult>& args) {
        return count_function(args);
    };
    
    functions_["MAX"] = [this](const std::vector<FormulaResult>& args) {
        return max_function(args);
    };
    
    functions_["MIN"] = [this](const std::vector<FormulaResult>& args) {
        return min_function(args);
    };
    
    functions_["IF"] = [this](const std::vector<FormulaResult>& args) {
        return if_function(args);
    };
}

// ========================================
// 公式计算主入口
// ========================================

FormulaResult FormulaEngine::evaluate(const std::string& formula, const std::string& sheet_name) {
    if (formula.empty()) {
        return std::monostate{};
    }
    
    try {
        // 词法分析
        auto tokens = tokenize(formula);
        if (tokens.empty()) {
            return std::monostate{};
        }
        
        // 语法分析
        size_t pos = 0;
        auto expr = parse_expression(tokens, pos);

        if (pos < tokens.size() && tokens[pos].type != Token::EndOfInput) {
            throw FormulaException("Unexpected token: " + tokens[pos].value);
        }
        
        // 计算表达式
        return expr->evaluate(this, sheet_name);
        
    } catch (const std::exception& e) {
        throw FormulaException("Failed to evaluate formula '" + formula + "': " + e.what());
    }
}

void FormulaEngine::register_function(const std::string& name, FormulaFunction function) {
    std::string upper_name = name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);
    functions_[upper_name] = std::move(function);
}

bool FormulaEngine::validate_formula(const std::string& formula) const {
    try {
        auto tokens = tokenize(formula);
        // 这里可以添加更详细的语法验证
        return !tokens.empty();
    } catch (...) {
        return false;
    }
}

// ========================================
// 词法分析器
// ========================================

std::vector<FormulaEngine::Token> FormulaEngine::tokenize(const std::string& formula) const {
    std::vector<Token> tokens;
    size_t i = 0;
    
    while (i < formula.length()) {
        char c = formula[i];
        
        // 跳过空白字符
        if (std::isspace(c)) {
            ++i;
            continue;
        }
        
        // 数字
        if (std::isdigit(c) || c == '.') {
            size_t start = i;
            while (i < formula.length() && (std::isdigit(formula[i]) || formula[i] == '.')) {
                ++i;
            }
            tokens.emplace_back(Token::Number, formula.substr(start, i - start));
            continue;
        }
        
        // 字符串（双引号）
        if (c == '"') {
            size_t start = ++i; // 跳过开始的引号
            while (i < formula.length() && formula[i] != '"') {
                ++i;
            }
            if (i >= formula.length()) {
                throw FormulaException("Unterminated string literal");
            }
            tokens.emplace_back(Token::String, formula.substr(start, i - start));
            ++i; // 跳过结束的引号
            continue;
        }
        
        // 标识符（函数名、单元格引用等）
        if (std::isalpha(c)) {
            size_t start = i;
            while (i < formula.length() && (std::isalnum(formula[i]) || formula[i] == '_')) {
                ++i;
            }
            
            // 检查是否是范围引用（如 A1:B2）
            if (i < formula.length() && formula[i] == ':') {
                // 继续解析范围的结束部分
                ++i; // 跳过冒号
                while (i < formula.length() && (std::isalnum(formula[i]))) {
                    ++i;
                }
                std::string range_ref = formula.substr(start, i - start);
                if (is_range_reference(range_ref)) {
                    tokens.emplace_back(Token::RangeRef, range_ref);
                } else {
                    throw FormulaException("Invalid range reference: " + range_ref);
                }
                continue;
            }
            
            std::string identifier = formula.substr(start, i - start);
            
            if (is_cell_reference(identifier)) {
                tokens.emplace_back(Token::CellRef, identifier);
            } else if (is_function_name(identifier)) {
                tokens.emplace_back(Token::Function, identifier);
            } else {
                throw FormulaException("Unknown identifier: " + identifier);
            }
            continue;
        }
        
        // 运算符和特殊字符
        switch (c) {
            case '+': case '-': case '*': case '/': case '^': case '&':
                tokens.emplace_back(Token::Operator, std::string(1, c));
                break;
            case '=':
                tokens.emplace_back(Token::Operator, "=");
                break;
            case '<':
                if (i + 1 < formula.length()) {
                    if (formula[i + 1] == '=') {
                        tokens.emplace_back(Token::Operator, "<=");
                        ++i; // 跳过下一个字符
                    } else if (formula[i + 1] == '>') {
                        tokens.emplace_back(Token::Operator, "<>");
                        ++i; // 跳过下一个字符
                    } else {
                        tokens.emplace_back(Token::Operator, "<");
                    }
                } else {
                    tokens.emplace_back(Token::Operator, "<");
                }
                break;
            case '>':
                if (i + 1 < formula.length() && formula[i + 1] == '=') {
                    tokens.emplace_back(Token::Operator, ">=");
                    ++i; // 跳过下一个字符
                } else {
                    tokens.emplace_back(Token::Operator, ">");
                }
                break;
            case '(':
                tokens.emplace_back(Token::LeftParen, "(");
                break;
            case ')':
                tokens.emplace_back(Token::RightParen, ")");
                break;
            case ',':
                tokens.emplace_back(Token::Comma, ",");
                break;
            default:
                throw FormulaException("Unexpected character: " + std::string(1, c));
        }
        ++i;
    }
    
    tokens.emplace_back(Token::EndOfInput, "");
    return tokens;
}

// ========================================
// 辅助函数
// ========================================

bool FormulaEngine::is_cell_reference(const std::string& str) const {
    // 简单的单元格引用检查：字母+数字（如 A1, B2, AA10）
    std::regex cell_pattern(R"(^[A-Z]+[0-9]+$)");
    return std::regex_match(str, cell_pattern);
}

bool FormulaEngine::is_range_reference(const std::string& str) const {
    // 范围引用检查：单元格:单元格（如 A1:B2）
    std::regex range_pattern(R"(^[A-Z]+[0-9]+:[A-Z]+[0-9]+$)");
    return std::regex_match(str, range_pattern);
}

bool FormulaEngine::is_function_name(const std::string& str) const {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);
    return functions_.find(upper_str) != functions_.end();
}

// ========================================
// 类型转换函数
// ========================================

double FormulaEngine::to_number(const FormulaResult& result) const {
    if (std::holds_alternative<double>(result)) {
        return std::get<double>(result);
    } else if (std::holds_alternative<bool>(result)) {
        return std::get<bool>(result) ? 1.0 : 0.0;
    } else if (std::holds_alternative<std::string>(result)) {
        const auto& str = std::get<std::string>(result);
        try {
            return std::stod(str);
        } catch (...) {
            return 0.0; // Excel行为：无法转换的字符串视为0
        }
    }
    return 0.0; // monostate
}

std::string FormulaEngine::to_string(const FormulaResult& result) const {
    if (std::holds_alternative<std::string>(result)) {
        return std::get<std::string>(result);
    } else if (std::holds_alternative<double>(result)) {
        return std::to_string(std::get<double>(result));
    } else if (std::holds_alternative<bool>(result)) {
        return std::get<bool>(result) ? "TRUE" : "FALSE";
    }
    return ""; // monostate
}

bool FormulaEngine::to_boolean(const FormulaResult& result) const {
    if (std::holds_alternative<bool>(result)) {
        return std::get<bool>(result);
    } else if (std::holds_alternative<double>(result)) {
        return std::get<double>(result) != 0.0;
    } else if (std::holds_alternative<std::string>(result)) {
        const auto& str = std::get<std::string>(result);
        return !str.empty() && str != "0" && str != "FALSE";
    }
    return false; // monostate
}

// ========================================
// 单元格和范围值获取
// ========================================

FormulaResult FormulaEngine::get_cell_value(const std::string& cell_ref, const std::string& sheet_name) {
    try {
        auto coord = ::tinakit::internal::utils::CoordinateUtils::string_to_coordinate(cell_ref);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        if (worksheet_impl.has_cell_data(coord)) {
            auto cell_data_obj = worksheet_impl.get_cell_data(coord);

            if (std::holds_alternative<std::string>(cell_data_obj.value)) {
                return std::get<std::string>(cell_data_obj.value);
            } else if (std::holds_alternative<double>(cell_data_obj.value)) {
                return std::get<double>(cell_data_obj.value);
            } else if (std::holds_alternative<int>(cell_data_obj.value)) {
                return static_cast<double>(std::get<int>(cell_data_obj.value));
            } else if (std::holds_alternative<bool>(cell_data_obj.value)) {
                return std::get<bool>(cell_data_obj.value);
            }
        }

        return std::monostate{}; // 空单元格
    } catch (...) {
        throw FormulaException("Invalid cell reference: " + cell_ref);
    }
}

std::vector<FormulaResult> FormulaEngine::get_range_values(const std::string& range_ref, const std::string& sheet_name) {
    std::vector<FormulaResult> values;

    try {
        auto range_addr = ::tinakit::internal::utils::CoordinateUtils::string_to_range_address(range_ref);
        auto& worksheet_impl = workbook_impl_->get_worksheet_impl_public(sheet_name);

        for (std::size_t row = range_addr.start.row; row <= range_addr.end.row; ++row) {
            for (std::size_t col = range_addr.start.column; col <= range_addr.end.column; ++col) {
                core::Coordinate coord(row, col);

                if (worksheet_impl.has_cell_data(coord)) {
                    auto cell_data_obj = worksheet_impl.get_cell_data(coord);

                    if (std::holds_alternative<std::string>(cell_data_obj.value)) {
                        values.push_back(std::get<std::string>(cell_data_obj.value));
                    } else if (std::holds_alternative<double>(cell_data_obj.value)) {
                        values.push_back(std::get<double>(cell_data_obj.value));
                    } else if (std::holds_alternative<int>(cell_data_obj.value)) {
                        values.push_back(static_cast<double>(std::get<int>(cell_data_obj.value)));
                    } else if (std::holds_alternative<bool>(cell_data_obj.value)) {
                        values.push_back(std::get<bool>(cell_data_obj.value));
                    } else {
                        values.push_back(std::monostate{});
                    }
                } else {
                    values.push_back(std::monostate{});
                }
            }
        }

    } catch (...) {
        throw FormulaException("Invalid range reference: " + range_ref);
    }

    return values;
}

// ========================================
// 语法分析器（递归下降解析器）
// ========================================

FormulaEngine::ExprNodePtr FormulaEngine::parse_expression(std::vector<Token>& tokens, size_t& pos) {
    auto left = parse_comparison(tokens, pos);

    while (pos < tokens.size() && tokens[pos].type == Token::Operator) {
        const auto& op = tokens[pos].value;
        if (op == "&") {  // 字符串连接操作符
            ++pos;
            auto right = parse_comparison(tokens, pos);
            left = std::make_unique<BinaryOpNode>(std::move(left), std::move(right), op);
        } else {
            break;
        }
    }

    return left;
}

FormulaEngine::ExprNodePtr FormulaEngine::parse_comparison(std::vector<Token>& tokens, size_t& pos) {
    auto left = parse_addition(tokens, pos);

    while (pos < tokens.size() && tokens[pos].type == Token::Operator) {
        const auto& op = tokens[pos].value;
        if (op == ">" || op == "<" || op == "=" || op == ">=" || op == "<=" || op == "<>") {
            ++pos;
            auto right = parse_addition(tokens, pos);
            left = std::make_unique<BinaryOpNode>(std::move(left), std::move(right), op);
        } else {
            break;
        }
    }

    return left;
}

FormulaEngine::ExprNodePtr FormulaEngine::parse_addition(std::vector<Token>& tokens, size_t& pos) {
    auto left = parse_term(tokens, pos);

    while (pos < tokens.size() && tokens[pos].type == Token::Operator) {
        const auto& op = tokens[pos].value;
        if (op == "+" || op == "-") {
            ++pos;
            auto right = parse_term(tokens, pos);
            left = std::make_unique<BinaryOpNode>(std::move(left), std::move(right), op);
        } else {
            break;
        }
    }

    return left;
}

FormulaEngine::ExprNodePtr FormulaEngine::parse_term(std::vector<Token>& tokens, size_t& pos) {
    auto left = parse_factor(tokens, pos);

    while (pos < tokens.size() && tokens[pos].type == Token::Operator) {
        const auto& op = tokens[pos].value;
        if (op == "*" || op == "/" || op == "^") {
            ++pos;
            auto right = parse_factor(tokens, pos);
            left = std::make_unique<BinaryOpNode>(std::move(left), std::move(right), op);
        } else {
            break;
        }
    }

    return left;
}

FormulaEngine::ExprNodePtr FormulaEngine::parse_factor(std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) {
        throw FormulaException("Unexpected end of input");
    }

    const auto& token = tokens[pos];

    switch (token.type) {
        case Token::Number:
            ++pos;
            return std::make_unique<NumberNode>(std::stod(token.value));

        case Token::String:
            ++pos;
            return std::make_unique<StringNode>(token.value);

        case Token::CellRef:
            ++pos;
            return std::make_unique<CellRefNode>(token.value);

        case Token::RangeRef:
            ++pos;
            return std::make_unique<RangeRefNode>(token.value);

        case Token::Function:
            return parse_function(tokens, pos);

        case Token::LeftParen: {
            ++pos;
            auto expr = parse_expression(tokens, pos);
            if (pos >= tokens.size() || tokens[pos].type != Token::RightParen) {
                throw FormulaException("Missing closing parenthesis");
            }
            ++pos;
            return expr;
        }

        case Token::Operator:
            if (token.value == "-" || token.value == "+") {
                ++pos;
                auto operand = parse_factor(tokens, pos);
                return std::make_unique<UnaryOpNode>(std::move(operand), token.value);
            }
            // fallthrough

        default:
            throw FormulaException("Unexpected token: " + token.value);
    }
}

FormulaEngine::ExprNodePtr FormulaEngine::parse_function(std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size() || tokens[pos].type != Token::Function) {
        throw FormulaException("Expected function name");
    }

    std::string function_name = tokens[pos].value;
    ++pos;

    if (pos >= tokens.size() || tokens[pos].type != Token::LeftParen) {
        throw FormulaException("Expected '(' after function name");
    }
    ++pos;

    std::vector<ExprNodePtr> arguments;

    // 解析参数列表
    if (pos < tokens.size() && tokens[pos].type != Token::RightParen) {
        do {
            arguments.push_back(parse_expression(tokens, pos));

            if (pos < tokens.size() && tokens[pos].type == Token::Comma) {
                ++pos;
            } else {
                break;
            }
        } while (pos < tokens.size());
    }

    if (pos >= tokens.size() || tokens[pos].type != Token::RightParen) {
        throw FormulaException("Expected ')' after function arguments");
    }
    ++pos;

    return std::make_unique<FunctionNode>(std::move(function_name), std::move(arguments));
}

// ========================================
// 表达式节点求值实现
// ========================================

FormulaResult FormulaEngine::NumberNode::evaluate(FormulaEngine* /* engine */, const std::string& /* sheet_name */) {
    return value;
}

FormulaResult FormulaEngine::StringNode::evaluate(FormulaEngine* /* engine */, const std::string& /* sheet_name */) {
    return value;
}

FormulaResult FormulaEngine::CellRefNode::evaluate(FormulaEngine* engine, const std::string& sheet_name) {
    return engine->get_cell_value(cell_ref, sheet_name);
}

FormulaResult FormulaEngine::RangeRefNode::evaluate(FormulaEngine* engine, const std::string& sheet_name) {
    // 范围引用通常在函数中使用，这里返回第一个值
    auto values = engine->get_range_values(range_ref, sheet_name);
    return values.empty() ? FormulaResult(std::monostate{}) : values[0];
}

FormulaResult FormulaEngine::BinaryOpNode::evaluate(FormulaEngine* engine, const std::string& sheet_name) {
    auto left_val = left->evaluate(engine, sheet_name);
    auto right_val = right->evaluate(engine, sheet_name);

    if (op == "+") {
        return engine->to_number(left_val) + engine->to_number(right_val);
    } else if (op == "-") {
        return engine->to_number(left_val) - engine->to_number(right_val);
    } else if (op == "*") {
        return engine->to_number(left_val) * engine->to_number(right_val);
    } else if (op == "/") {
        double right_num = engine->to_number(right_val);
        if (right_num == 0.0) {
            throw FormulaException("Division by zero");
        }
        return engine->to_number(left_val) / right_num;
    } else if (op == "^") {
        return std::pow(engine->to_number(left_val), engine->to_number(right_val));
    } else if (op == "&") {
        return engine->to_string(left_val) + engine->to_string(right_val);
    } else if (op == ">") {
        return engine->to_number(left_val) > engine->to_number(right_val);
    } else if (op == "<") {
        return engine->to_number(left_val) < engine->to_number(right_val);
    } else if (op == "=") {
        return engine->to_number(left_val) == engine->to_number(right_val);
    } else if (op == ">=") {
        return engine->to_number(left_val) >= engine->to_number(right_val);
    } else if (op == "<=") {
        return engine->to_number(left_val) <= engine->to_number(right_val);
    } else if (op == "<>") {
        return engine->to_number(left_val) != engine->to_number(right_val);
    }

    throw FormulaException("Unknown binary operator: " + op);
}

FormulaResult FormulaEngine::UnaryOpNode::evaluate(FormulaEngine* engine, const std::string& sheet_name) {
    auto operand_val = operand->evaluate(engine, sheet_name);

    if (op == "-") {
        return -engine->to_number(operand_val);
    } else if (op == "+") {
        return engine->to_number(operand_val);
    }

    throw FormulaException("Unknown unary operator: " + op);
}

FormulaResult FormulaEngine::FunctionNode::evaluate(FormulaEngine* engine, const std::string& sheet_name) {
    std::string upper_name = function_name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    auto it = engine->functions_.find(upper_name);
    if (it == engine->functions_.end()) {
        throw FormulaException("Unknown function: " + function_name);
    }

    std::vector<FormulaResult> args;
    for (auto& arg : arguments) {
        // 特殊处理范围引用
        if (auto range_node = dynamic_cast<RangeRefNode*>(arg.get())) {
            auto range_values = engine->get_range_values(range_node->range_ref, sheet_name);
            args.insert(args.end(), range_values.begin(), range_values.end());
        } else {
            args.push_back(arg->evaluate(engine, sheet_name));
        }
    }

    return it->second(args);
}

// ========================================
// 内置函数实现
// ========================================

FormulaResult FormulaEngine::sum_function(const std::vector<FormulaResult>& args) {
    double sum = 0.0;
    for (const auto& arg : args) {
        if (!std::holds_alternative<std::monostate>(arg)) {
            sum += to_number(arg);
        }
    }
    return sum;
}

FormulaResult FormulaEngine::average_function(const std::vector<FormulaResult>& args) {
    if (args.empty()) {
        throw FormulaException("AVERAGE function requires at least one argument");
    }

    double sum = 0.0;
    size_t count = 0;

    for (const auto& arg : args) {
        if (!std::holds_alternative<std::monostate>(arg)) {
            sum += to_number(arg);
            ++count;
        }
    }

    if (count == 0) {
        throw FormulaException("AVERAGE function: no numeric values found");
    }

    return sum / count;
}

FormulaResult FormulaEngine::count_function(const std::vector<FormulaResult>& args) {
    double count = 0.0;
    for (const auto& arg : args) {
        if (!std::holds_alternative<std::monostate>(arg)) {
            ++count;
        }
    }
    return count;
}

FormulaResult FormulaEngine::max_function(const std::vector<FormulaResult>& args) {
    if (args.empty()) {
        throw FormulaException("MAX function requires at least one argument");
    }

    double max_val = std::numeric_limits<double>::lowest();
    bool found_number = false;

    for (const auto& arg : args) {
        if (!std::holds_alternative<std::monostate>(arg)) {
            double val = to_number(arg);
            if (!found_number || val > max_val) {
                max_val = val;
                found_number = true;
            }
        }
    }

    if (!found_number) {
        throw FormulaException("MAX function: no numeric values found");
    }

    return max_val;
}

FormulaResult FormulaEngine::min_function(const std::vector<FormulaResult>& args) {
    if (args.empty()) {
        throw FormulaException("MIN function requires at least one argument");
    }

    double min_val = std::numeric_limits<double>::max();
    bool found_number = false;

    for (const auto& arg : args) {
        if (!std::holds_alternative<std::monostate>(arg)) {
            double val = to_number(arg);
            if (!found_number || val < min_val) {
                min_val = val;
                found_number = true;
            }
        }
    }

    if (!found_number) {
        throw FormulaException("MIN function: no numeric values found");
    }

    return min_val;
}

FormulaResult FormulaEngine::if_function(const std::vector<FormulaResult>& args) {
    if (args.size() < 2 || args.size() > 3) {
        throw FormulaException("IF function requires 2 or 3 arguments");
    }

    bool condition = to_boolean(args[0]);

    if (condition) {
        return args[1];
    } else {
        return args.size() > 2 ? args[2] : FormulaResult(std::monostate{});
    }
}

} // namespace tinakit::excel
