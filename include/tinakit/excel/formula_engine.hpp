/**
 * @file formula_engine.hpp
 * @brief Excel 公式计算引擎
 * @author TinaKit Team
 * @date 2025-6-21
 */

#pragma once

#include "tinakit/core/types.hpp"
#include "tinakit/core/exceptions.hpp"
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

// 前向声明
namespace tinakit::internal {
    class workbook_impl;
}

namespace tinakit::excel {

/**
 * @brief 公式计算结果类型
 */
using FormulaResult = std::variant<double, std::string, bool, std::monostate>;

/**
 * @brief 公式函数类型
 */
using FormulaFunction = std::function<FormulaResult(const std::vector<FormulaResult>&)>;

/**
 * @class FormulaEngine
 * @brief Excel 公式计算引擎
 * 
 * 支持基础的Excel公式计算，包括：
 * - 基本算术运算 (+, -, *, /, ^)
 * - 单元格引用 (A1, B2等)
 * - 范围引用 (A1:B2)
 * - 内置函数 (SUM, AVERAGE, COUNT等)
 * - 自定义函数注册
 */
class FormulaEngine {
public:
    /**
     * @brief 构造函数
     * @param workbook_impl 工作簿实现指针
     */
    explicit FormulaEngine(tinakit::internal::workbook_impl* workbook_impl);

    /**
     * @brief 计算公式
     * @param formula 公式字符串（不包含等号）
     * @param sheet_name 当前工作表名称
     * @return 计算结果
     * @throws FormulaException 公式计算错误
     */
    FormulaResult evaluate(const std::string& formula, const std::string& sheet_name);

    /**
     * @brief 注册自定义函数
     * @param name 函数名称（大写）
     * @param function 函数实现
     */
    void register_function(const std::string& name, FormulaFunction function);

    /**
     * @brief 检查公式语法是否正确
     * @param formula 公式字符串
     * @return 如果语法正确返回true
     */
    bool validate_formula(const std::string& formula) const;

private:
    /**
     * @brief 词法分析器 - 将公式分解为标记
     */
    struct Token {
        enum Type {
            Number,      // 数字
            String,      // 字符串
            CellRef,     // 单元格引用 (A1)
            RangeRef,    // 范围引用 (A1:B2)
            Function,    // 函数名
            Operator,    // 运算符
            LeftParen,   // 左括号
            RightParen,  // 右括号
            Comma,       // 逗号
            EndOfInput   // 输入结束
        };

        Type type;
        std::string value;
        
        Token(Type t, std::string v) : type(t), value(std::move(v)) {}
    };

    /**
     * @brief 表达式节点
     */
    struct ExprNode {
        virtual ~ExprNode() = default;
        virtual FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) = 0;
    };

    using ExprNodePtr = std::unique_ptr<ExprNode>;

    // 具体的表达式节点类型
    struct NumberNode : ExprNode {
        double value;
        explicit NumberNode(double v) : value(v) {}
        FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) override;
    };

    struct StringNode : ExprNode {
        std::string value;
        explicit StringNode(std::string v) : value(std::move(v)) {}
        FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) override;
    };

    struct CellRefNode : ExprNode {
        std::string cell_ref;
        explicit CellRefNode(std::string ref) : cell_ref(std::move(ref)) {}
        FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) override;
    };

    struct RangeRefNode : ExprNode {
        std::string range_ref;
        explicit RangeRefNode(std::string ref) : range_ref(std::move(ref)) {}
        FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) override;
    };

    struct BinaryOpNode : ExprNode {
        ExprNodePtr left;
        ExprNodePtr right;
        std::string op;
        
        BinaryOpNode(ExprNodePtr l, ExprNodePtr r, std::string o) 
            : left(std::move(l)), right(std::move(r)), op(std::move(o)) {}
        FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) override;
    };

    struct UnaryOpNode : ExprNode {
        ExprNodePtr operand;
        std::string op;
        
        UnaryOpNode(ExprNodePtr o, std::string op_str) 
            : operand(std::move(o)), op(std::move(op_str)) {}
        FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) override;
    };

    struct FunctionNode : ExprNode {
        std::string function_name;
        std::vector<ExprNodePtr> arguments;
        
        FunctionNode(std::string name, std::vector<ExprNodePtr> args) 
            : function_name(std::move(name)), arguments(std::move(args)) {}
        FormulaResult evaluate(FormulaEngine* engine, const std::string& sheet_name) override;
    };

private:
    // 词法分析
    std::vector<Token> tokenize(const std::string& formula) const;
    
    // 语法分析（递归下降解析器）
    ExprNodePtr parse_expression(std::vector<Token>& tokens, size_t& pos);
    ExprNodePtr parse_comparison(std::vector<Token>& tokens, size_t& pos);
    ExprNodePtr parse_addition(std::vector<Token>& tokens, size_t& pos);
    ExprNodePtr parse_term(std::vector<Token>& tokens, size_t& pos);
    ExprNodePtr parse_factor(std::vector<Token>& tokens, size_t& pos);
    ExprNodePtr parse_function(std::vector<Token>& tokens, size_t& pos);
    
    // 辅助函数
    bool is_cell_reference(const std::string& str) const;
    bool is_range_reference(const std::string& str) const;
    bool is_function_name(const std::string& str) const;
    
    // 内置函数
    void register_builtin_functions();
    FormulaResult sum_function(const std::vector<FormulaResult>& args);
    FormulaResult average_function(const std::vector<FormulaResult>& args);
    FormulaResult count_function(const std::vector<FormulaResult>& args);
    FormulaResult max_function(const std::vector<FormulaResult>& args);
    FormulaResult min_function(const std::vector<FormulaResult>& args);
    FormulaResult if_function(const std::vector<FormulaResult>& args);
    
    // 类型转换和运算
    double to_number(const FormulaResult& result) const;
    std::string to_string(const FormulaResult& result) const;
    bool to_boolean(const FormulaResult& result) const;
    
    // 获取单元格值
    FormulaResult get_cell_value(const std::string& cell_ref, const std::string& sheet_name);
    std::vector<FormulaResult> get_range_values(const std::string& range_ref, const std::string& sheet_name);

private:
    tinakit::internal::workbook_impl* workbook_impl_;
    std::unordered_map<std::string, FormulaFunction> functions_;
};

/**
 * @class FormulaException
 * @brief 公式计算异常
 */
class FormulaException : public TinaKitException {
public:
    explicit FormulaException(const std::string& message) 
        : TinaKitException("Formula Error: " + message) {}
};

} // namespace tinakit::excel
