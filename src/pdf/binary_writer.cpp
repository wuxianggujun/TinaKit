/**
 * @file binary_writer.cpp
 * @brief PDF二进制文件写入器实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/binary_writer.hpp"
#include <stdexcept>
#include <iomanip>
#include <sstream>

namespace tinakit::pdf {

// ========================================
// 构造函数和析构函数
// ========================================

BinaryWriter::BinaryWriter(const std::string& filename)
    : filename_(filename), is_open_(false) {
    out_.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!out_.is_open()) {
        throw std::runtime_error("无法创建文件: " + filename);
    }
    is_open_ = true;
}

BinaryWriter::~BinaryWriter() {
    if (is_open_) {
        close();
    }
}

BinaryWriter::BinaryWriter(BinaryWriter&& other) noexcept
    : out_(std::move(other.out_)), filename_(std::move(other.filename_)), is_open_(other.is_open_) {
    other.is_open_ = false;
}

BinaryWriter& BinaryWriter::operator=(BinaryWriter&& other) noexcept {
    if (this != &other) {
        if (is_open_) {
            close();
        }
        out_ = std::move(other.out_);
        filename_ = std::move(other.filename_);
        is_open_ = other.is_open_;
        other.is_open_ = false;
    }
    return *this;
}

// ========================================
// 写入方法
// ========================================

void BinaryWriter::write(const std::string& text) {
    checkState();
    out_ << text;
}

void BinaryWriter::writeLine(const std::string& line) {
    checkState();
    out_ << line << '\n';
}

void BinaryWriter::writeBinary(const void* data, std::size_t size) {
    checkState();
    out_.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
}

void BinaryWriter::writeBytes(const std::vector<std::uint8_t>& data) {
    writeBinary(data.data(), data.size());
}

void BinaryWriter::writeByte(std::uint8_t byte) {
    writeBinary(&byte, 1);
}

void BinaryWriter::writeInt(int value) {
    write(std::to_string(value));
}

void BinaryWriter::writeFloat(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    write(oss.str());
}

// ========================================
// 位置和状态管理
// ========================================

std::streampos BinaryWriter::getOffset() const {
    checkState();
    return const_cast<std::ofstream&>(out_).tellp();
}

long BinaryWriter::getOffsetLong() const {
    return static_cast<long>(getOffset());
}

void BinaryWriter::flush() {
    if (is_open_) {
        out_.flush();
    }
}

void BinaryWriter::close() {
    if (is_open_) {
        out_.close();
        is_open_ = false;
    }
}

bool BinaryWriter::isOpen() const {
    return is_open_ && out_.is_open();
}

bool BinaryWriter::good() const {
    return is_open_ && out_.good();
}

// ========================================
// PDF特定的便利方法
// ========================================

void BinaryWriter::writeObjectStart(int obj_id, int gen_num) {
    writeInt(obj_id);
    write(" ");
    writeInt(gen_num);
    writeLine(" obj");
}

void BinaryWriter::writeObjectEnd() {
    writeLine("endobj");
}

void BinaryWriter::writeDictStart() {
    writeLine("<<");
}

void BinaryWriter::writeDictEnd() {
    writeLine(">>");
}

void BinaryWriter::writeStreamStart(std::size_t length) {
    if (length > 0) {
        write("/Length ");
        writeInt(static_cast<int>(length));
        writeLine();
    }
    writeLine("stream");
}

void BinaryWriter::writeStreamEnd() {
    writeLine("endstream");
}

void BinaryWriter::writeName(const std::string& name) {
    write("/" + name);
}

void BinaryWriter::writeString(const std::string& str, bool literal) {
    if (literal) {
        write("(" + escapePdfString(str) + ")");
    } else {
        write("<" + str + ">");
    }
}

void BinaryWriter::writeReference(int obj_id, int gen_num) {
    writeInt(obj_id);
    write(" ");
    writeInt(gen_num);
    write(" R");
}

void BinaryWriter::writeArrayStart() {
    write("[");
}

void BinaryWriter::writeArrayEnd() {
    write("]");
}

// ========================================
// 格式化输出
// ========================================

void BinaryWriter::writeIndentedLine(const std::string& line, int indent_level, 
                                     char indent_char, int indent_size) {
    std::string indent(indent_level * indent_size, indent_char);
    writeLine(indent + line);
}

void BinaryWriter::writeComment(const std::string& comment) {
    writeLine("% " + comment);
}

// ========================================
// 私有方法
// ========================================

void BinaryWriter::checkState() const {
    if (!is_open_) {
        throw std::runtime_error("文件未打开");
    }
    if (!out_.good()) {
        throw std::runtime_error("文件写入错误");
    }
}

std::string BinaryWriter::escapePdfString(const std::string& str) const {
    std::string result;
    result.reserve(str.length() * 2);  // 预分配空间
    
    for (char c : str) {
        switch (c) {
            case '(':
                result += "\\(";
                break;
            case ')':
                result += "\\)";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (c < 32 || c > 126) {
                    // 非打印字符用八进制表示
                    std::ostringstream oss;
                    oss << "\\" << std::oct << std::setfill('0') << std::setw(3) << static_cast<unsigned char>(c);
                    result += oss.str();
                } else {
                    result += c;
                }
                break;
        }
    }
    
    return result;
}

} // namespace tinakit::pdf
