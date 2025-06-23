/**
 * @file serializer.hpp
 * @brief PDF序列化器 - 中间层，负责PDF语法结构的序列化
 * @author TinaKit Team
 * @date 2025-6-23
 */

#pragma once

#include "tinakit/pdf/core/binary_writer.hpp"
#include <string>
#include <string_view>
#include <span>
#include <cstdint>

namespace tinakit::pdf::core {

// 前向声明
class PdfSerializer;

/**
 * @brief RAII字典作用域助手
 *
 * 自动管理字典的开始和结束，防止忘记调用endDict()
 */
class DictScope {
public:
    explicit DictScope(PdfSerializer& serializer);
    ~DictScope();

    // 禁用拷贝和移动
    DictScope(const DictScope&) = delete;
    DictScope& operator=(const DictScope&) = delete;
    DictScope(DictScope&&) = delete;
    DictScope& operator=(DictScope&&) = delete;

private:
    PdfSerializer& serializer_;
};

/**
 * @brief RAII数组作用域助手
 *
 * 自动管理数组的开始和结束，防止忘记调用endArray()
 */
class ArrayScope {
public:
    explicit ArrayScope(PdfSerializer& serializer);
    ~ArrayScope();

    // 禁用拷贝和移动
    ArrayScope(const ArrayScope&) = delete;
    ArrayScope& operator=(const ArrayScope&) = delete;
    ArrayScope(ArrayScope&&) = delete;
    ArrayScope& operator=(ArrayScope&&) = delete;

private:
    PdfSerializer& serializer_;
};

/**
 * @brief RAII对象作用域助手
 *
 * 自动管理对象的开始和结束，防止忘记调用endObject()
 */
class ObjectScope {
public:
    ObjectScope(PdfSerializer& serializer, int obj_id, int gen_num = 0);
    ~ObjectScope();

    // 禁用拷贝和移动
    ObjectScope(const ObjectScope&) = delete;
    ObjectScope& operator=(const ObjectScope&) = delete;
    ObjectScope(ObjectScope&&) = delete;
    ObjectScope& operator=(ObjectScope&&) = delete;

private:
    PdfSerializer& serializer_;
};

/**
 * @brief PDF序列化器
 *
 * 中间层组件，负责将PDF语法结构序列化为字节流。
 * 提供语法级别的API，隐藏底层字节操作细节。
 */
class PdfSerializer {
public:
    /**
     * @brief 构造函数
     * @param writer 底层二进制写入器
     */
    explicit PdfSerializer(BinaryWriter& writer);

    // ========================================
    // 基本数据类型
    // ========================================

    /**
     * @brief 写入PDF名称对象
     * @param name 名称（不包含前导斜杠）
     */
    void name(std::string_view name);

    /**
     * @brief 写入整数
     * @param value 整数值
     */
    void number(int value);

    /**
     * @brief 写入浮点数
     * @param value 浮点数值
     * @param precision 精度
     */
    void number(double value, int precision = 6);

    /**
     * @brief 写入字符串字面量
     * @param str 字符串内容
     */
    void string(std::string_view str);

    /**
     * @brief 写入十六进制字符串
     * @param data 二进制数据
     */
    void hexString(std::span<const uint8_t> data);

    /**
     * @brief 写入十六进制字符串
     * @param hex_str 十六进制字符串（不包含尖括号）
     */
    void hexString(std::string_view hex_str);

    /**
     * @brief 写入对象引用
     * @param obj_id 对象ID
     * @param gen_num 生成号（默认为0）
     */
    void reference(int obj_id, int gen_num = 0);

    // ========================================
    // 复合数据类型
    // ========================================

    /**
     * @brief 开始字典
     */
    void beginDict();

    /**
     * @brief 结束字典
     */
    void endDict();

    /**
     * @brief 开始数组
     */
    void beginArray();

    /**
     * @brief 结束数组
     */
    void endArray();

    // ========================================
    // PDF对象结构
    // ========================================

    /**
     * @brief 开始对象定义
     * @param obj_id 对象ID
     * @param gen_num 生成号
     */
    void beginObject(int obj_id, int gen_num = 0);

    /**
     * @brief 结束对象定义
     */
    void endObject();

    /**
     * @brief 开始流对象
     * @param length 流长度（如果已知）
     */
    void beginStream(int length = -1);

    /**
     * @brief 结束流对象
     */
    void endStream();

    /**
     * @brief 写入流数据
     * @param data 流数据
     */
    void streamData(std::span<const uint8_t> data);

    /**
     * @brief 写入流数据
     * @param data 流数据
     */
    void streamData(std::string_view data);

    // ========================================
    // 格式控制
    // ========================================

    /**
     * @brief 写入换行符
     */
    void newline();

    /**
     * @brief 写入空格
     */
    void space();

    /**
     * @brief 写入原始数据（不做任何处理）
     * @param data 原始数据
     */
    void raw(std::string_view data);

    /**
     * @brief 获取当前写入位置
     * @return 字节偏移量
     */
    size_t getOffset() const;

    // ========================================
    // 错误检测
    // ========================================

    /**
     * @brief 检查是否有错误
     * @return 如果有错误返回true
     */
    bool hasError() const;

    /**
     * @brief 获取错误信息
     * @return 错误信息字符串
     */
    std::string getErrorMessage() const;

    // ========================================
    // 便利方法
    // ========================================

    /**
     * @brief 写入字典条目
     * @param key 键名
     * @param value 值（字符串）
     */
    void dictEntry(std::string_view key, std::string_view value);

    /**
     * @brief 写入字典条目
     * @param key 键名
     * @param value 值（整数）
     */
    void dictEntry(std::string_view key, int value);

    /**
     * @brief 写入字典条目
     * @param key 键名
     * @param obj_id 对象引用ID
     */
    void dictEntryRef(std::string_view key, int obj_id);

private:
    BinaryWriter& writer_;
    int depth_ = 0;  // 缩进深度
    bool need_space_ = false;  // 是否需要空格分隔
    bool has_error_ = false;  // 是否有错误
    std::string error_message_;  // 错误信息

    enum class State {
        Normal,
        InDict,
        InArray,
        InObject
    };
    State state_ = State::Normal;  // 当前状态

    /**
     * @brief 写入缩进
     */
    void writeIndent();

    /**
     * @brief 确保适当的分隔符
     */
    void ensureSeparator();

    /**
     * @brief 设置错误状态
     * @param message 错误信息
     */
    void setError(const std::string& message);

    /**
     * @brief 检查状态是否有效
     * @param expected_state 期望的状态
     * @param operation 操作名称
     * @return 如果状态有效返回true
     */
    bool checkState(State expected_state, const std::string& operation);
};

} // namespace tinakit::pdf::core
