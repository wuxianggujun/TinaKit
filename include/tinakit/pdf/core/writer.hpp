/**
 * @file writer.hpp
 * @brief PDF核心写入器 - 管理PDF对象、结构和xref表
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include "binary_writer.hpp"
#include "font_manager.hpp"
#include "font_subsetter.hpp"
#include "freetype_subsetter.hpp"
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <set>

namespace tinakit::pdf::core {

// 前向声明
class PdfObject;
class PdfPage;

/**
 * @class Writer
 * @brief PDF核心写入器
 * 
 * 负责管理PDF文档的整体结构，包括：
 * - PDF对象的创建和管理
 * - 交叉引用表(xref)的维护
 * - 文档结构的组织
 * - 最终PDF文件的生成
 */
class Writer {
public:
    // ========================================
    // 构造函数和析构函数
    // ========================================
    
    /**
     * @brief 构造函数
     */
    Writer();
    
    /**
     * @brief 析构函数
     */
    ~Writer() = default;
    
    // 禁用拷贝，支持移动
    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;
    Writer(Writer&&) = default;
    Writer& operator=(Writer&&) = default;

    // ========================================
    // 对象管理
    // ========================================
    
    /**
     * @brief 添加PDF对象
     * @param obj PDF对象
     * @return 对象ID
     */
    int addObject(std::unique_ptr<PdfObject> obj);
    
    /**
     * @brief 获取下一个可用的对象ID
     * @return 对象ID
     */
    int getNextObjectId();
    
    /**
     * @brief 根据ID获取对象
     * @param id 对象ID
     * @return 对象指针，如果不存在返回nullptr
     */
    PdfObject* getObject(int id) const;
    
    /**
     * @brief 获取对象数量
     * @return 对象数量
     */
    std::size_t getObjectCount() const;

    // ========================================
    // 页面管理
    // ========================================
    
    /**
     * @brief 创建新页面
     * @param width 页面宽度（点）
     * @param height 页面高度（点）
     * @return 页面对象指针
     */
    PdfPage* createPage(double width = 595.0, double height = 842.0);
    
    /**
     * @brief 获取页面数量
     * @return 页面数量
     */
    std::size_t getPageCount() const;
    
    /**
     * @brief 获取指定页面
     * @param index 页面索引（从0开始）
     * @return 页面对象指针，如果不存在返回nullptr
     */
    PdfPage* getPage(std::size_t index) const;
    
    /**
     * @brief 获取所有页面
     * @return 页面列表
     */
    const std::vector<std::unique_ptr<PdfPage>>& getPages() const;

    // ========================================
    // 资源管理
    // ========================================
    
    /**
     * @brief 注册字体
     * @param font_name 字体名称
     * @param font_data 字体数据（可选，用于嵌入字体）
     * @param embed_font 是否嵌入字体（默认true）
     * @return 字体资源ID
     */
    std::string registerFont(const std::string& font_name,
                           const std::vector<std::uint8_t>& font_data = {},
                           bool embed_font = true);

    /**
     * @brief 注册字体并启用子集化
     * @param font_name 字体名称
     * @param font_data 字体数据
     * @param enable_subsetting 是否启用字体子集化
     * @param embed_font 是否嵌入字体
     * @return 字体资源ID
     */
    std::string registerFontWithSubsetting(const std::string& font_name,
                                          const std::vector<std::uint8_t>& font_data,
                                          bool enable_subsetting = true,
                                          bool embed_font = true);

    /**
     * @brief 获取字体资源ID
     * @param font_name 字体名称
     * @return 字体资源ID，如果不存在返回空字符串
     */
    std::string getFontResourceId(const std::string& font_name) const;

    /**
     * @brief 获取字体子类型
     * @param font_name 字体名称
     * @return 字体子类型（Type0或Type1），如果不存在返回空字符串
     */
    std::string getFontSubtype(const std::string& font_name) const;

    /**
     * @brief 获取字体管理器
     * @return 字体管理器指针
     */
    FontManager* getFontManager() const;

    // 字体回退相关方法已移除，简化字体处理
    
    /**
     * @brief 注册图像（从文件）
     * @param image_path 图像文件路径
     * @return 图像资源ID
     */
    std::string registerImage(const std::string& image_path);

    /**
     * @brief 注册图像（从数据）
     * @param image_data 图像数据
     * @param width 图像宽度
     * @param height 图像高度
     * @param format 图像格式（如"JPEG", "PNG"）
     * @return 图像资源ID
     */
    std::string registerImage(const std::vector<std::uint8_t>& image_data,
                            int width, int height, const std::string& format);

    // ========================================
    // 文档属性
    // ========================================
    
    /**
     * @brief 设置文档信息
     * @param title 标题
     * @param author 作者
     * @param subject 主题
     * @param creator 创建者
     */
    void setDocumentInfo(const std::string& title = "",
                        const std::string& author = "",
                        const std::string& subject = "",
                        const std::string& creator = "TinaKit");
    
    /**
     * @brief 设置PDF版本
     * @param version PDF版本（如"1.4", "1.7"）
     */
    void setPdfVersion(const std::string& version);

    // ========================================
    // 文件生成
    // ========================================
    
    /**
     * @brief 保存到文件
     * @param filename 文件名
     */
    void saveToFile(const std::string& filename);
    
    /**
     * @brief 保存到缓冲区
     * @return PDF数据
     */
    std::vector<std::uint8_t> saveToBuffer();
    
    /**
     * @brief 写入到BinaryWriter
     * @param writer 写入器
     */
    void writeTo(BinaryWriter& writer);

    // ========================================
    // 调试和验证
    // ========================================
    
    /**
     * @brief 验证PDF结构
     * @return 验证结果，空字符串表示无错误
     */
    std::string validate() const;
    
    /**
     * @brief 获取统计信息
     * @return 统计信息字符串
     */
    std::string getStatistics() const;
    
    /**
     * @brief 启用/禁用调试模式
     * @param enable 是否启用
     */
    void setDebugMode(bool enable);

private:
    // ========================================
    // 内部结构
    // ========================================
    
    struct ObjectEntry {
        std::unique_ptr<PdfObject> object;
        std::size_t offset = 0;  // 在文件中的偏移量
        bool written = false;    // 是否已写入
    };
    
    struct DocumentInfo {
        std::string title;
        std::string author;
        std::string subject;
        std::string creator = "TinaKit";
        std::string producer = "TinaKit PDF Library";
    };

    // ========================================
    // 成员变量
    // ========================================
    
    // 对象管理
    std::map<int, ObjectEntry> objects_;        ///< 对象映射表
    int next_object_id_ = 1;                    ///< 下一个对象ID
    
    // 页面管理
    std::vector<std::unique_ptr<PdfPage>> pages_;  ///< 页面列表
    std::vector<int> page_object_ids_;              ///< 页面对象ID列表（PDF生成时分配）
    int pages_object_id_ = 0;                   ///< 页面树对象ID
    int catalog_object_id_ = 0;                 ///< 目录对象ID
    
    // 资源管理
    std::map<std::string, std::string> font_resources_;   ///< 字体资源映射 (字体名 -> 资源ID)
    std::map<std::string, int> font_object_ids_;          ///< 字体对象ID映射 (字体名 -> 对象ID)
    std::map<std::string, std::string> font_subtypes_;    ///< 字体子类型映射 (字体名 -> 子类型)
    // font_fallbacks_ 成员变量已移除
    std::map<std::string, std::string> image_resources_;  ///< 图像资源映射
    std::map<std::string, int> image_object_ids_;         ///< 图像对象ID映射 (资源ID -> 对象ID)
    int next_resource_id_ = 1;                  ///< 下一个资源ID
    
    // 文档属性
    DocumentInfo doc_info_;                     ///< 文档信息
    std::string pdf_version_ = "1.4";          ///< PDF版本
    
    // 状态
    bool debug_mode_ = false;                   ///< 调试模式

    // 字体管理
    std::unique_ptr<FontManager> font_manager_; ///< 字体管理器
    std::unique_ptr<FontSubsetter> font_subsetter_; ///< 字体子集化工具
    std::unique_ptr<FreeTypeSubsetter> freetype_subsetter_; ///< FreeType字体子集化工具
    std::map<std::string, bool> font_subsetting_enabled_; ///< 字体子集化启用状态
    std::map<std::string, std::vector<std::uint8_t>> original_font_data_; ///< 原始字体数据（用于子集化）
    
    // ========================================
    // 内部方法
    // ========================================
    
    /**
     * @brief 创建目录对象
     */
    void createCatalogObject();
    
    /**
     * @brief 创建页面树对象
     */
    void createPagesObject();
    
    /**
     * @brief 创建文档信息对象
     */
    void createInfoObject();
    
    /**
     * @brief 写入PDF头部
     * @param writer 写入器
     */
    void writeHeader(BinaryWriter& writer);
    
    /**
     * @brief 写入所有对象
     * @param writer 写入器
     */
    void writeObjects(BinaryWriter& writer);
    
    /**
     * @brief 写入交叉引用表
     * @param writer 写入器
     * @return xref表的偏移量
     */
    std::size_t writeXrefTable(BinaryWriter& writer);
    
    /**
     * @brief 写入尾部
     * @param writer 写入器
     * @param xref_offset xref表偏移量
     */
    void writeTrailer(BinaryWriter& writer, std::size_t xref_offset);
    
    /**
     * @brief 生成资源字典
     * @return 资源字典字符串
     */
    std::string generateResourceDict() const;

    /**
     * @brief 收集页面中使用指定字体的所有字符码点
     * @param font_name 字体名称
     * @return 使用的字符码点集合
     */
    std::set<uint32_t> collectUsedCodepoints(const std::string& font_name) const;

    /**
     * @brief 收集文档中所有使用的字符码点
     * @param font_name 字体名称，为空则收集所有字体的字符
     * @return 使用的字符码点集合
     */
    std::set<uint32_t> collectAllUsedCodepoints(const std::string& font_name = "") const;



    /**
     * @brief 判断字体是否支持Unicode
     * @param font_name 字体名称
     * @return 支持Unicode返回true，否则返回false
     */
    bool isUnicodeFont(const std::string& font_name) const;

    /**
     * @brief 判断字符是否为CJK字符
     * @param codepoint Unicode码点
     * @return 是CJK字符返回true，否则返回false
     */
    bool isCJKCharacter(uint32_t codepoint) const;

    /**
     * @brief 生成字符宽度数组
     * @param font_name 字体名称
     * @return PDF格式的宽度数组字符串
     */
    std::string generateWidthArray(const std::string& font_name) const;

    /**
     * @brief 生成ToUnicode CMap
     * @return CMap内容字符串
     */
    std::string generateToUnicodeCMap() const;

    /**
     * @brief 执行字体子集化
     * 在所有文本添加完成后调用，生成字体子集并更新字体对象
     */
    void performFontSubsetting();

    /**
     * @brief 为指定字体创建子集
     * @param font_name 字体名称
     * @param used_codepoints 使用的字符码点
     * @return 成功返回true
     */
    bool createFontSubset(const std::string& font_name, const std::set<uint32_t>& used_codepoints);

    /**
     * @brief 更新现有字体的数据
     * @param font_name 字体名称
     * @param font_data 新的字体数据
     */
    void updateFontWithData(const std::string& font_name, const std::vector<std::uint8_t>& font_data);
};

} // namespace tinakit::pdf::core
