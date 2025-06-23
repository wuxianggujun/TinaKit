/**
 * @file serializer.cpp
 * @brief PDF序列化器实现
 * @author TinaKit Team
 * @date 2025-6-23
 */

#include "tinakit/pdf/core/serializer.hpp"
#include "tinakit/core/logger.hpp"
#include <sstream>
#include <iomanip>

namespace tinakit::pdf::core {

// ========================================
// RAII作用域助手实现
// ========================================

DictScope::DictScope(PdfSerializer& serializer) : serializer_(serializer) {
    serializer_.beginDict();
}

DictScope::~DictScope() {
    serializer_.endDict();
}

ArrayScope::ArrayScope(PdfSerializer& serializer) : serializer_(serializer) {
    serializer_.beginArray();
}

ArrayScope::~ArrayScope() {
    serializer_.endArray();
}

ObjectScope::ObjectScope(PdfSerializer& serializer, int obj_id, int gen_num)
    : serializer_(serializer) {
    serializer_.beginObject(obj_id, gen_num);
}

ObjectScope::~ObjectScope() {
    serializer_.endObject();
}

// ========================================
// PdfSerializer实现
// ========================================

PdfSerializer::PdfSerializer(BinaryWriter& writer) : writer_(writer) {
}

// ========================================
// 基本数据类型
// ========================================

void PdfSerializer::name(std::string_view name) {
    ensureSeparator();
    writer_.write("/");
    writer_.write(std::string(name));
    need_space_ = true;
}

void PdfSerializer::number(int value) {
    ensureSeparator();
    writer_.writeInt(value);
    need_space_ = true;
}

void PdfSerializer::number(double value, int precision) {
    ensureSeparator();
    writer_.writeFloat(value, precision);
    need_space_ = true;
}

void PdfSerializer::string(std::string_view str) {
    ensureSeparator();
    writer_.writeString(std::string(str), true);
    need_space_ = true;
}

void PdfSerializer::hexString(std::span<const uint8_t> data) {
    ensureSeparator();
    writer_.write("<");

    for (uint8_t byte : data) {
        char hex[3];
        std::sprintf(hex, "%02X", byte);
        writer_.write(std::string(hex, 2));
    }

    writer_.write(">");
    need_space_ = true;
}

void PdfSerializer::hexString(std::string_view hex_str) {
    ensureSeparator();
    writer_.write("<");
    writer_.write(std::string(hex_str));
    writer_.write(">");
    need_space_ = true;
}

void PdfSerializer::reference(int obj_id, int gen_num) {
    ensureSeparator();
    writer_.writeReference(obj_id, gen_num);
    need_space_ = true;
}

// ========================================
// 复合数据类型
// ========================================

void PdfSerializer::beginDict() {
    if (has_error_) return;

    ensureSeparator();
    writer_.writeDictStart();
    newline();
    ++depth_;
    state_ = State::InDict;
    need_space_ = false;
}

void PdfSerializer::endDict() {
    if (has_error_) return;

    if (depth_ <= 0) {
        setError("endDict() called without matching beginDict()");
        return;
    }

    --depth_;
    writeIndent();
    writer_.writeDictEnd();
    state_ = State::Normal;
    need_space_ = true;
}

void PdfSerializer::beginArray() {
    ensureSeparator();
    writer_.writeArrayStart();
    space();
    need_space_ = false;
}

void PdfSerializer::endArray() {
    space();
    writer_.writeArrayEnd();
    need_space_ = true;
}

// ========================================
// PDF对象结构
// ========================================

void PdfSerializer::beginObject(int obj_id, int gen_num) {
    writer_.writeObjectStart(obj_id, gen_num);
    need_space_ = false;
}

void PdfSerializer::endObject() {
    writer_.writeObjectEnd();
    need_space_ = false;
}

void PdfSerializer::beginStream(int length) {
    writer_.writeStreamStart(length >= 0 ? static_cast<size_t>(length) : 0);
    need_space_ = false;
}

void PdfSerializer::endStream() {
    writer_.writeStreamEnd();
    need_space_ = false;
}

void PdfSerializer::streamData(std::span<const uint8_t> data) {
    std::vector<uint8_t> vec(data.begin(), data.end());
    writer_.writeBytes(vec);
}

void PdfSerializer::streamData(std::string_view data) {
    writer_.write(std::string(data));
}

// ========================================
// 格式控制
// ========================================

void PdfSerializer::newline() {
    writer_.writeLine("");
    need_space_ = false;
}

void PdfSerializer::space() {
    writer_.write(" ");
    need_space_ = false;
}

void PdfSerializer::raw(std::string_view data) {
    writer_.write(std::string(data));
    need_space_ = true;
}

size_t PdfSerializer::getOffset() const {
    return writer_.getOffset();
}

bool PdfSerializer::hasError() const {
    return has_error_;
}

std::string PdfSerializer::getErrorMessage() const {
    return error_message_;
}

// ========================================
// 便利方法
// ========================================

void PdfSerializer::dictEntry(std::string_view key, std::string_view value) {
    writeIndent();
    name(key);
    space();
    name(value);
    newline();
}

void PdfSerializer::dictEntry(std::string_view key, int value) {
    writeIndent();
    name(key);
    space();
    number(value);
    newline();
}

void PdfSerializer::dictEntryRef(std::string_view key, int obj_id) {
    writeIndent();
    name(key);
    space();
    reference(obj_id);
    newline();
}

// ========================================
// 私有方法
// ========================================

void PdfSerializer::writeIndent() {
    for (int i = 0; i < depth_; ++i) {
        writer_.write("  ");  // 两个空格的缩进
    }
    need_space_ = false;
}

void PdfSerializer::ensureSeparator() {
    if (need_space_) {
        space();
    }
}

void PdfSerializer::setError(const std::string& message) {
    has_error_ = true;
    error_message_ = message;
    CORE_ERROR("PdfSerializer error: " + message);
}

bool PdfSerializer::checkState(State expected_state, const std::string& operation) {
    if (state_ != expected_state) {
        setError(operation + " called in wrong state");
        return false;
    }
    return true;
}

} // namespace tinakit::pdf::core
