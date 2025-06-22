/**
 * @file object.cpp
 * @brief PDF对象系统实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/core/object.hpp"
#include "tinakit/core/logger.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace tinakit::pdf::core {

// ========================================
// PdfObject基类实现
// ========================================

PdfObject::PdfObject(int id, int generation)
    : id_(id), generation_(generation) {
}

void PdfObject::writeTo(BinaryWriter& writer) const {
    std::string content = getContent();
    PDF_DEBUG("Writing object " + std::to_string(id_) + " (" + getTypeName() + "): " + content.substr(0, 100) + (content.length() > 100 ? "..." : ""));

    writer.writeObjectStart(id_, generation_);
    writer.write(content);
    writer.writeObjectEnd();
}

// ========================================
// DictionaryObject实现
// ========================================

DictionaryObject::DictionaryObject(int id, int generation)
    : PdfObject(id, generation) {
}

void DictionaryObject::set(const std::string& key, const std::string& value) {
    entries_[key] = value;
}

void DictionaryObject::setReference(const std::string& key, int obj_id, int generation) {
    entries_[key] = makeReference(obj_id, generation);
}

void DictionaryObject::setArray(const std::string& key, const std::vector<std::string>& values) {
    entries_[key] = makeArray(values);
}

std::string DictionaryObject::get(const std::string& key) const {
    auto it = entries_.find(key);
    return (it != entries_.end()) ? it->second : "";
}

bool DictionaryObject::contains(const std::string& key) const {
    return entries_.find(key) != entries_.end();
}

std::string DictionaryObject::getContent() const {
    std::ostringstream oss;
    oss << "<<\n";
    
    for (const auto& [key, value] : entries_) {
        oss << "/" << key << " " << value << "\n";
    }
    
    oss << ">>";
    return oss.str();
}

// ========================================
// StreamObject实现
// ========================================

StreamObject::StreamObject(int id, int generation)
    : DictionaryObject(id, generation) {
}

void StreamObject::setStreamData(const std::vector<std::uint8_t>& data) {
    stream_data_ = data;
    set("Length", std::to_string(data.size()));
}

void StreamObject::setStreamData(const std::string& data) {
    stream_data_.assign(data.begin(), data.end());
    set("Length", std::to_string(data.size()));
}

void StreamObject::appendStreamData(const std::string& data) {
    stream_data_.insert(stream_data_.end(), data.begin(), data.end());
    set("Length", std::to_string(stream_data_.size()));
}

const std::vector<std::uint8_t>& StreamObject::getStreamData() const {
    return stream_data_;
}

std::size_t StreamObject::getStreamSize() const {
    return stream_data_.size();
}

void StreamObject::writeTo(BinaryWriter& writer) const {
    std::string stream_content(stream_data_.begin(), stream_data_.end());

    PDF_DEBUG("Writing object " + std::to_string(getId()) + " (Stream): stream=" + stream_content.substr(0, 50) + "... (total " + std::to_string(stream_data_.size()) + " bytes)");

    writer.writeObjectStart(getId(), getGeneration());

    // 写入字典部分（包含Length）
    writer.writeLine("<< /Length " + std::to_string(stream_data_.size()) + " >>");

    // 写入流数据
    writer.writeLine("stream");
    writer.writeBytes(stream_data_);
    writer.writeLine("endstream");

    writer.writeObjectEnd();
}

std::string StreamObject::getContent() const {
    std::ostringstream oss;
    oss << DictionaryObject::getContent() << "\n";
    oss << "stream\n";
    oss << std::string(stream_data_.begin(), stream_data_.end());
    oss << "\nendstream";
    return oss.str();
}

// ========================================
// CatalogObject实现
// ========================================

CatalogObject::CatalogObject(int id, int pages_id)
    : DictionaryObject(id) {
    set("Type", "/Catalog");
    setPagesReference(pages_id);
}

void CatalogObject::setPagesReference(int pages_id) {
    setReference("Pages", pages_id);
}

// ========================================
// PagesObject实现
// ========================================

PagesObject::PagesObject(int id)
    : DictionaryObject(id) {
    set("Type", "/Pages");
    set("Count", "0");
    set("Kids", "[]");
}

void PagesObject::addPageReference(int page_id) {
    page_ids_.push_back(page_id);
    updateContent();
}

void PagesObject::setPageReferences(const std::vector<int>& page_ids) {
    page_ids_ = page_ids;
    updateContent();
}

std::size_t PagesObject::getPageCount() const {
    return page_ids_.size();
}

void PagesObject::updateContent() {
    set("Count", std::to_string(page_ids_.size()));
    
    std::vector<std::string> refs;
    refs.reserve(page_ids_.size());
    for (int id : page_ids_) {
        refs.push_back(makeReference(id));
    }
    set("Kids", makeArray(refs));
}

// ========================================
// FontObject实现
// ========================================

FontObject::FontObject(int id, const std::string& base_font, const std::string& subtype)
    : DictionaryObject(id), base_font_(base_font), subtype_(subtype) {
    set("Type", "/Font");
    set("Subtype", "/" + subtype);
    set("BaseFont", "/" + base_font);

    PDF_DEBUG("FontObject created: ID=" + std::to_string(id) + ", BaseFont=/" + base_font + ", Subtype=/" + subtype);
}

void FontObject::setEncoding(const std::string& encoding) {
    set("Encoding", "/" + encoding);
}

void FontObject::setFontDescriptor(int descriptor_id) {
    setReference("FontDescriptor", descriptor_id);
}

// ========================================
// InfoObject实现
// ========================================

InfoObject::InfoObject(int id)
    : DictionaryObject(id) {
    // 设置默认的创建时间
    setCreationDate(getCurrentPdfDate());
    setModDate(getCurrentPdfDate());
}

void InfoObject::setTitle(const std::string& title) {
    set("Title", makeString(title));
}

void InfoObject::setAuthor(const std::string& author) {
    set("Author", makeString(author));
}

void InfoObject::setSubject(const std::string& subject) {
    set("Subject", makeString(subject));
}

void InfoObject::setCreator(const std::string& creator) {
    set("Creator", makeString(creator));
}

void InfoObject::setProducer(const std::string& producer) {
    set("Producer", makeString(producer));
}

void InfoObject::setCreationDate(const std::string& creation_date) {
    set("CreationDate", makeString(creation_date));
}

void InfoObject::setModDate(const std::string& mod_date) {
    set("ModDate", makeString(mod_date));
}

std::string InfoObject::escapeString(const std::string& str) const {
    std::string result;
    result.reserve(str.length() * 2);
    
    for (char c : str) {
        switch (c) {
            case '(':
            case ')':
            case '\\':
                result += '\\';
                result += c;
                break;
            default:
                result += c;
                break;
        }
    }
    
    return result;
}

// ========================================
// 辅助函数实现
// ========================================

std::string makeReference(int obj_id, int generation) {
    return std::to_string(obj_id) + " " + std::to_string(generation) + " R";
}

std::string makeName(const std::string& name) {
    return "/" + name;
}

std::string makeString(const std::string& str, bool literal) {
    if (literal) {
        std::string escaped;
        escaped.reserve(str.length() * 2);
        
        for (char c : str) {
            switch (c) {
                case '(':
                case ')':
                case '\\':
                    escaped += '\\';
                    escaped += c;
                    break;
                default:
                    escaped += c;
                    break;
            }
        }
        
        return "(" + escaped + ")";
    } else {
        return "<" + str + ">";
    }
}

std::string makeArray(const std::vector<std::string>& values) {
    std::ostringstream oss;
    oss << "[";
    
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) oss << " ";
        oss << values[i];
    }
    
    oss << "]";
    return oss.str();
}

std::string getCurrentPdfDate() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

#ifdef _WIN32
    std::tm tm;
    localtime_s(&tm, &time_t);
#else
    auto tm = *std::localtime(&time_t);
#endif
    
    std::ostringstream oss;
    oss << "D:"
        << std::setfill('0')
        << std::setw(4) << (tm.tm_year + 1900)
        << std::setw(2) << (tm.tm_mon + 1)
        << std::setw(2) << tm.tm_mday
        << std::setw(2) << tm.tm_hour
        << std::setw(2) << tm.tm_min
        << std::setw(2) << tm.tm_sec;
    
    return oss.str();
}

} // namespace tinakit::pdf::core
