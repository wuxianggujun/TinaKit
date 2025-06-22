/**
 * @file object.hpp
 * @brief PDF对象基类和具体对象实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include "tinakit/pdf/binary_writer.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace tinakit::pdf::core {

/**
 * @class PdfObject
 * @brief PDF对象基类
 * 
 * 所有PDF对象的基类，定义了PDF对象的基本接口
 */
class PdfObject {
public:
    /**
     * @brief 构造函数
     * @param id 对象ID
     * @param generation 生成号（默认0）
     */
    explicit PdfObject(int id, int generation = 0);
    
    /**
     * @brief 虚析构函数
     */
    virtual ~PdfObject() = default;
    
    /**
     * @brief 获取对象ID
     * @return 对象ID
     */
    int getId() const { return id_; }
    
    /**
     * @brief 获取生成号
     * @return 生成号
     */
    int getGeneration() const { return generation_; }
    
    /**
     * @brief 写入对象到PDF
     * @param writer 写入器
     */
    virtual void writeTo(BinaryWriter& writer) const;
    
    /**
     * @brief 获取对象内容
     * @return 对象内容字符串
     */
    virtual std::string getContent() const = 0;
    
    /**
     * @brief 获取对象类型名称
     * @return 类型名称
     */
    virtual std::string getTypeName() const = 0;

protected:
    int id_;           ///< 对象ID
    int generation_;   ///< 生成号
};

/**
 * @class DictionaryObject
 * @brief PDF字典对象
 */
class DictionaryObject : public PdfObject {
public:
    explicit DictionaryObject(int id, int generation = 0);
    
    /**
     * @brief 设置字典项
     * @param key 键
     * @param value 值
     */
    void set(const std::string& key, const std::string& value);
    
    /**
     * @brief 设置字典项（引用）
     * @param key 键
     * @param obj_id 引用的对象ID
     * @param generation 生成号
     */
    void setReference(const std::string& key, int obj_id, int generation = 0);
    
    /**
     * @brief 设置字典项（数组）
     * @param key 键
     * @param values 数组值
     */
    void setArray(const std::string& key, const std::vector<std::string>& values);
    
    /**
     * @brief 获取字典项
     * @param key 键
     * @return 值，如果不存在返回空字符串
     */
    std::string get(const std::string& key) const;
    
    /**
     * @brief 检查是否包含指定键
     * @param key 键
     * @return 如果包含返回true
     */
    bool contains(const std::string& key) const;
    
    std::string getContent() const override;
    std::string getTypeName() const override { return "Dictionary"; }

protected:
    std::map<std::string, std::string> entries_;  ///< 字典项
};

/**
 * @class StreamObject
 * @brief PDF流对象
 */
class StreamObject : public DictionaryObject {
public:
    explicit StreamObject(int id, int generation = 0);
    
    /**
     * @brief 设置流数据
     * @param data 流数据
     */
    void setStreamData(const std::vector<std::uint8_t>& data);
    
    /**
     * @brief 设置流数据
     * @param data 流数据字符串
     */
    void setStreamData(const std::string& data);
    
    /**
     * @brief 添加流数据
     * @param data 要添加的数据
     */
    void appendStreamData(const std::string& data);
    
    /**
     * @brief 获取流数据
     * @return 流数据
     */
    const std::vector<std::uint8_t>& getStreamData() const;
    
    /**
     * @brief 获取流数据大小
     * @return 数据大小（字节）
     */
    std::size_t getStreamSize() const;
    
    void writeTo(BinaryWriter& writer) const override;
    std::string getContent() const override;
    std::string getTypeName() const override { return "Stream"; }

private:
    std::vector<std::uint8_t> stream_data_;  ///< 流数据
};

/**
 * @class CatalogObject
 * @brief PDF目录对象
 */
class CatalogObject : public DictionaryObject {
public:
    explicit CatalogObject(int id, int pages_id);
    
    /**
     * @brief 设置页面树引用
     * @param pages_id 页面树对象ID
     */
    void setPagesReference(int pages_id);
    
    std::string getTypeName() const override { return "Catalog"; }
};

/**
 * @class PagesObject
 * @brief PDF页面树对象
 */
class PagesObject : public DictionaryObject {
public:
    explicit PagesObject(int id);
    
    /**
     * @brief 添加页面引用
     * @param page_id 页面对象ID
     */
    void addPageReference(int page_id);
    
    /**
     * @brief 设置页面引用列表
     * @param page_ids 页面对象ID列表
     */
    void setPageReferences(const std::vector<int>& page_ids);
    
    /**
     * @brief 获取页面数量
     * @return 页面数量
     */
    std::size_t getPageCount() const;
    
    std::string getTypeName() const override { return "Pages"; }

private:
    std::vector<int> page_ids_;  ///< 页面ID列表
    
    void updateContent();
};

/**
 * @class FontObject
 * @brief PDF字体对象
 */
class FontObject : public DictionaryObject {
public:
    explicit FontObject(int id, const std::string& base_font, 
                       const std::string& subtype = "Type1");
    
    /**
     * @brief 设置字体编码
     * @param encoding 编码名称
     */
    void setEncoding(const std::string& encoding);
    
    /**
     * @brief 设置字体描述符引用
     * @param descriptor_id 字体描述符对象ID
     */
    void setFontDescriptor(int descriptor_id);
    
    std::string getTypeName() const override { return "Font"; }

private:
    std::string base_font_;  ///< 基础字体名称
    std::string subtype_;    ///< 字体子类型
};

/**
 * @class InfoObject
 * @brief PDF文档信息对象
 */
class InfoObject : public DictionaryObject {
public:
    explicit InfoObject(int id);
    
    /**
     * @brief 设置标题
     * @param title 标题
     */
    void setTitle(const std::string& title);
    
    /**
     * @brief 设置作者
     * @param author 作者
     */
    void setAuthor(const std::string& author);
    
    /**
     * @brief 设置主题
     * @param subject 主题
     */
    void setSubject(const std::string& subject);
    
    /**
     * @brief 设置创建者
     * @param creator 创建者
     */
    void setCreator(const std::string& creator);
    
    /**
     * @brief 设置生产者
     * @param producer 生产者
     */
    void setProducer(const std::string& producer);
    
    /**
     * @brief 设置创建时间
     * @param creation_date 创建时间（PDF日期格式）
     */
    void setCreationDate(const std::string& creation_date);
    
    /**
     * @brief 设置修改时间
     * @param mod_date 修改时间（PDF日期格式）
     */
    void setModDate(const std::string& mod_date);
    
    std::string getTypeName() const override { return "Info"; }

private:
    /**
     * @brief 转义PDF字符串
     * @param str 原始字符串
     * @return 转义后的字符串
     */
    std::string escapeString(const std::string& str) const;
};

// ========================================
// 辅助函数
// ========================================

/**
 * @brief 创建PDF引用字符串
 * @param obj_id 对象ID
 * @param generation 生成号
 * @return 引用字符串
 */
std::string makeReference(int obj_id, int generation = 0);

/**
 * @brief 创建PDF名称字符串
 * @param name 名称
 * @return PDF名称字符串
 */
std::string makeName(const std::string& name);

/**
 * @brief 创建PDF字符串
 * @param str 字符串内容
 * @param literal 是否为字面量字符串
 * @return PDF字符串
 */
std::string makeString(const std::string& str, bool literal = true);

/**
 * @brief 创建PDF数组字符串
 * @param values 数组值
 * @return PDF数组字符串
 */
std::string makeArray(const std::vector<std::string>& values);

/**
 * @brief 获取当前时间的PDF日期格式
 * @return PDF日期字符串
 */
std::string getCurrentPdfDate();

} // namespace tinakit::pdf::core
