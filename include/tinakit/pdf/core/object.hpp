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
 * @class CIDFontObject
 * @brief PDF CID字体对象（用于Unicode支持）
 */
class CIDFontObject : public DictionaryObject {
public:
    /**
     * @brief 构造函数
     * @param id 对象ID
     * @param base_font 基础字体名称
     * @param subtype CID字体子类型（CIDFontType0或CIDFontType2）
     */
    CIDFontObject(int id, const std::string& base_font, const std::string& subtype = "CIDFontType0");

    /**
     * @brief 设置CID系统信息
     * @param registry 注册表
     * @param ordering 排序
     * @param supplement 补充
     */
    void setCIDSystemInfo(const std::string& registry, const std::string& ordering, int supplement);

    /**
     * @brief 设置字体描述符引用
     * @param descriptor_id 字体描述符对象ID
     */
    void setFontDescriptor(int descriptor_id);

    /**
     * @brief 设置默认宽度
     * @param width 默认字符宽度
     */
    void setDefaultWidth(int width);

    std::string getTypeName() const override { return "CIDFont"; }

private:
    std::string base_font_;  ///< 基础字体名称
    std::string subtype_;    ///< CID字体子类型
};

/**
 * @class FontFileObject
 * @brief PDF字体文件对象（用于字体嵌入）
 */
class FontFileObject : public StreamObject {
public:
    /**
     * @brief 构造函数
     * @param id 对象ID
     * @param font_data 字体文件数据
     * @param subtype 字体文件子类型（FontFile2用于TrueType）
     */
    FontFileObject(int id, const std::vector<uint8_t>& font_data, const std::string& subtype = "FontFile2");

    std::string getTypeName() const override { return "FontFile"; }

private:
    std::string subtype_;
};

/**
 * @class ImageObject
 * @brief PDF图像对象（用于图像嵌入）
 */
class ImageObject : public StreamObject {
public:
    /**
     * @brief 构造函数
     * @param id 对象ID
     * @param image_data 图像数据
     * @param width 图像宽度
     * @param height 图像高度
     * @param color_space 颜色空间（RGB, Gray等）
     * @param bits_per_component 每个颜色分量的位数
     */
    ImageObject(int id, const std::vector<uint8_t>& image_data,
                int width, int height,
                const std::string& color_space = "DeviceRGB",
                int bits_per_component = 8);

    std::string getTypeName() const override { return "Image"; }

private:
    int width_;
    int height_;
    std::string color_space_;
    int bits_per_component_;
};

/**
 * @class FontDescriptorObject
 * @brief PDF字体描述符对象
 */
class FontDescriptorObject : public DictionaryObject {
public:
    /**
     * @brief 构造函数
     * @param id 对象ID
     * @param font_name 字体名称
     */
    FontDescriptorObject(int id, const std::string& font_name);

    /**
     * @brief 设置字体标志
     * @param flags 字体标志
     */
    void setFlags(int flags);

    /**
     * @brief 设置字体边界框
     * @param bbox 边界框数组 [llx, lly, urx, ury]
     */
    void setFontBBox(const std::vector<int>& bbox);

    /**
     * @brief 设置字体度量
     */
    void setFontMetrics(int ascent, int descent, int cap_height, int stem_v);

    /**
     * @brief 设置字体文件引用（用于字体嵌入）
     * @param font_file_id 字体文件对象ID
     * @param subtype 字体文件类型（FontFile2等）
     */
    void setFontFile(int font_file_id, const std::string& subtype = "FontFile2");

    std::string getTypeName() const override { return "FontDescriptor"; }

private:
    std::string font_name_;
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

    /**
     * @brief 设置后代字体引用（用于Type0字体）
     * @param descendant_font_id CID字体对象ID
     */
    void setDescendantFont(int descendant_font_id);

    /**
     * @brief 设置ToUnicode CMap引用
     * @param tounicode_id ToUnicode CMap对象ID
     */
    void setToUnicode(int tounicode_id);

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

/**
 * @brief 将UTF-8字符串转换为UTF-16BE十六进制格式
 * @param utf8_text UTF-8编码的文本
 * @return UTF-16BE十六进制字符串，格式为<FEFF...>
 */
std::string convertToUTF16BE(const std::string& utf8_text);

/**
 * @brief 检查字符串是否包含非ASCII字符
 * @param text 要检查的文本
 * @return 如果包含非ASCII字符返回true
 */
bool containsNonASCII(const std::string& text);

/**
 * @brief 文本段结构
 */
struct TextSegment {
    std::string text;       ///< 文本内容
    bool is_unicode;        ///< 是否为Unicode文本
};

/**
 * @brief 将混合文本分割为ASCII和Unicode段
 * @param text 输入文本
 * @return 文本段列表
 */
std::vector<TextSegment> segmentText(const std::string& text);

/**
 * @brief 从文件加载字体数据
 * @param font_path 字体文件路径
 * @return 字体文件数据，失败时返回空vector
 */
std::vector<uint8_t> loadFontFile(const std::string& font_path);

/**
 * @brief 获取系统字体路径
 * @param font_name 字体名称
 * @return 字体文件路径，未找到时返回空字符串
 */
std::string getSystemFontPath(const std::string& font_name);

/**
 * @brief 图像数据结构
 */
struct ImageData {
    std::vector<uint8_t> data;  ///< 图像原始数据
    int width;                  ///< 图像宽度
    int height;                 ///< 图像高度
    int channels;               ///< 颜色通道数
    std::string format;         ///< 图像格式（JPEG, PNG等）
};

/**
 * @brief 从文件加载图像
 * @param image_path 图像文件路径
 * @return 图像数据，失败时返回空结构
 */
ImageData loadImageFile(const std::string& image_path);

} // namespace tinakit::pdf::core
