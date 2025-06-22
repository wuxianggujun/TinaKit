/**
 * @file binary_writer.hpp
 * @brief PDF二进制文件写入器
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <cstddef>

namespace tinakit::pdf {

/**
 * @class BinaryWriter
 * @brief PDF文件二进制写入器
 * 
 * 提供高效的PDF文件写入功能，支持文本和二进制数据的写入，
 * 并能跟踪文件偏移量，这对PDF的交叉引用表生成非常重要。
 * 
 * @example
 * ```cpp
 * BinaryWriter writer("output.pdf");
 * writer.writeLine("%PDF-1.4");
 * writer.write("1 0 obj");
 * writer.writeLine("<<");
 * writer.writeLine("/Type /Catalog");
 * writer.writeLine(">>");
 * writer.writeLine("endobj");
 * 
 * long offset = writer.getOffset();
 * writer.close();
 * ```
 */
class BinaryWriter {
public:
    // ========================================
    // 构造函数和析构函数
    // ========================================
    
    /**
     * @brief 构造函数
     * @param filename 输出文件名
     * @throws std::runtime_error 如果无法创建文件
     */
    explicit BinaryWriter(const std::string& filename);
    
    /**
     * @brief 析构函数，自动关闭文件
     */
    ~BinaryWriter();
    
    // 禁用拷贝构造和赋值
    BinaryWriter(const BinaryWriter&) = delete;
    BinaryWriter& operator=(const BinaryWriter&) = delete;
    
    // 支持移动语义
    BinaryWriter(BinaryWriter&& other) noexcept;
    BinaryWriter& operator=(BinaryWriter&& other) noexcept;

    // ========================================
    // 写入方法
    // ========================================
    
    /**
     * @brief 写入字符串（不带换行符）
     * @param text 要写入的文本
     */
    void write(const std::string& text);
    
    /**
     * @brief 写入一行文本，自动添加换行符
     * @param line 要写入的行
     */
    void writeLine(const std::string& line = "");
    
    /**
     * @brief 写入二进制数据
     * @param data 数据指针
     * @param size 数据大小（字节）
     */
    void writeBinary(const void* data, std::size_t size);
    
    /**
     * @brief 写入字节数组
     * @param data 字节数组
     */
    void writeBytes(const std::vector<std::uint8_t>& data);
    
    /**
     * @brief 写入单个字节
     * @param byte 字节值
     */
    void writeByte(std::uint8_t byte);
    
    /**
     * @brief 写入整数（以文本形式）
     * @param value 整数值
     */
    void writeInt(int value);
    
    /**
     * @brief 写入浮点数（以文本形式）
     * @param value 浮点数值
     * @param precision 小数位数（默认2位）
     */
    void writeFloat(double value, int precision = 2);

    // ========================================
    // 位置和状态管理
    // ========================================
    
    /**
     * @brief 获取当前文件偏移量
     * @return 当前偏移量（字节）
     */
    std::streampos getOffset() const;
    
    /**
     * @brief 获取当前文件偏移量（长整型）
     * @return 当前偏移量（字节）
     */
    long getOffsetLong() const;
    
    /**
     * @brief 刷新缓冲区
     */
    void flush();
    
    /**
     * @brief 关闭文件
     */
    void close();
    
    /**
     * @brief 检查文件是否打开
     * @return 如果文件打开返回true
     */
    bool isOpen() const;
    
    /**
     * @brief 检查写入器状态是否良好
     * @return 如果状态良好返回true
     */
    bool good() const;

    // ========================================
    // PDF特定的便利方法
    // ========================================
    
    /**
     * @brief 写入PDF对象开始标记
     * @param obj_id 对象ID
     * @param gen_num 生成号（默认0）
     */
    void writeObjectStart(int obj_id, int gen_num = 0);
    
    /**
     * @brief 写入PDF对象结束标记
     */
    void writeObjectEnd();
    
    /**
     * @brief 写入PDF字典开始标记
     */
    void writeDictStart();
    
    /**
     * @brief 写入PDF字典结束标记
     */
    void writeDictEnd();
    
    /**
     * @brief 写入PDF流开始标记
     * @param length 流长度（可选）
     */
    void writeStreamStart(std::size_t length = 0);
    
    /**
     * @brief 写入PDF流结束标记
     */
    void writeStreamEnd();
    
    /**
     * @brief 写入PDF名称对象
     * @param name 名称（不包含/前缀）
     */
    void writeName(const std::string& name);
    
    /**
     * @brief 写入PDF字符串对象
     * @param str 字符串内容
     * @param literal 是否为字面量字符串（默认true）
     */
    void writeString(const std::string& str, bool literal = true);
    
    /**
     * @brief 写入PDF引用
     * @param obj_id 对象ID
     * @param gen_num 生成号（默认0）
     */
    void writeReference(int obj_id, int gen_num = 0);
    
    /**
     * @brief 写入PDF数组开始标记
     */
    void writeArrayStart();
    
    /**
     * @brief 写入PDF数组结束标记
     */
    void writeArrayEnd();

    // ========================================
    // 格式化输出
    // ========================================
    
    /**
     * @brief 写入带缩进的行
     * @param line 行内容
     * @param indent_level 缩进级别
     * @param indent_char 缩进字符（默认空格）
     * @param indent_size 每级缩进大小（默认2）
     */
    void writeIndentedLine(const std::string& line, int indent_level, 
                          char indent_char = ' ', int indent_size = 2);
    
    /**
     * @brief 写入注释行
     * @param comment 注释内容
     */
    void writeComment(const std::string& comment);

private:
    std::ofstream out_;         ///< 输出文件流
    std::string filename_;      ///< 文件名
    bool is_open_;             ///< 文件是否打开
    
    /**
     * @brief 检查文件状态，如果有错误则抛出异常
     */
    void checkState() const;
    
    /**
     * @brief 转义PDF字符串中的特殊字符
     * @param str 原始字符串
     * @return 转义后的字符串
     */
    std::string escapePdfString(const std::string& str) const;
};

} // namespace tinakit::pdf
