//
// Created by wuxianggujun on 2025/6/21.
//

#pragma once

#include <string>

namespace tinakit::excel {

/**
 * @brief OpenXML命名空间管理类
 * 
 * 专门管理Excel OpenXML文档中使用的所有命名空间，
 * 避免在代码各处硬编码命名空间字符串，便于维护和修改。
 */
class OpenXmlNamespaces {
public:
    // ========================================
    // Excel OpenXML 主要命名空间
    // ========================================
    
    /**
     * @brief 主要的SpreadsheetML命名空间
     */
    static const std::string& spreadsheet_main() {
        static const std::string ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
        return ns;
    }
    
    /**
     * @brief 关系命名空间
     */
    static const std::string& relationships() {
        static const std::string ns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
        return ns;
    }
    
    /**
     * @brief 内容类型命名空间
     */
    static const std::string& content_types() {
        static const std::string ns = "http://schemas.openxmlformats.org/package/2006/content-types";
        return ns;
    }

    /**
     * @brief 包关系命名空间
     */
    static const std::string& package_relationships() {
        static const std::string ns = "http://schemas.openxmlformats.org/package/2006/relationships";
        return ns;
    }
    
    /**
     * @brief 核心属性命名空间
     */
    static const std::string& core_properties() {
        static const std::string ns = "http://schemas.openxmlformats.org/package/2006/metadata/core-properties";
        return ns;
    }
    
    /**
     * @brief 扩展属性命名空间
     */
    static const std::string& extended_properties() {
        static const std::string ns = "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties";
        return ns;
    }
    
    /**
     * @brief 自定义属性命名空间
     */
    static const std::string& custom_properties() {
        static const std::string ns = "http://schemas.openxmlformats.org/officeDocument/2006/custom-properties";
        return ns;
    }

    // ========================================
    // Excel 绘图和图表命名空间
    // ========================================
    
    /**
     * @brief 绘图命名空间
     */
    static const std::string& drawing() {
        static const std::string ns = "http://schemas.openxmlformats.org/drawingml/2006/main";
        return ns;
    }
    
    /**
     * @brief 图表命名空间
     */
    static const std::string& chart() {
        static const std::string ns = "http://schemas.openxmlformats.org/drawingml/2006/chart";
        return ns;
    }
    
    /**
     * @brief SpreadsheetML绘图命名空间
     */
    static const std::string& spreadsheet_drawing() {
        static const std::string ns = "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing";
        return ns;
    }

    // ========================================
    // Excel 特有命名空间
    // ========================================
    
    /**
     * @brief VML命名空间（用于Excel旧版兼容）
     */
    static const std::string& vml() {
        static const std::string ns = "urn:schemas-microsoft-com:vml";
        return ns;
    }
    
    /**
     * @brief Office VML命名空间
     */
    static const std::string& office_vml() {
        static const std::string ns = "urn:schemas-microsoft-com:office:office";
        return ns;
    }
    
    /**
     * @brief Excel VML命名空间
     */
    static const std::string& excel_vml() {
        static const std::string ns = "urn:schemas-microsoft-com:office:excel";
        return ns;
    }

    // ========================================
    // 命名空间前缀管理
    // ========================================
    
    /**
     * @brief 获取关系命名空间的默认前缀
     */
    static const std::string& relationships_prefix() {
        static const std::string prefix = "r";
        return prefix;
    }
    
    /**
     * @brief 获取绘图命名空间的默认前缀
     */
    static const std::string& drawing_prefix() {
        static const std::string prefix = "a";
        return prefix;
    }
    
    /**
     * @brief 获取图表命名空间的默认前缀
     */
    static const std::string& chart_prefix() {
        static const std::string prefix = "c";
        return prefix;
    }

    // ========================================
    // 便利方法
    // ========================================
    
    /**
     * @brief 获取带前缀的属性名
     * @param prefix 前缀
     * @param local_name 本地名称
     * @return 带前缀的属性名（如 "r:id"）
     */
    static std::string prefixed_name(const std::string& prefix, const std::string& local_name) {
        return prefix + ":" + local_name;
    }
    
    /**
     * @brief 获取关系属性名
     * @param local_name 本地名称
     * @return 关系属性名（如 "r:id"）
     */
    static std::string relationship_attr(const std::string& local_name) {
        return prefixed_name(relationships_prefix(), local_name);
    }

private:
    // 私有构造函数，防止实例化
    OpenXmlNamespaces() = default;
};

/**
 * @brief Excel OpenXML命名空间常量的简化访问
 * 
 * 提供更简洁的Excel命名空间访问方式
 */
namespace openxml_ns {
    // 主要命名空间的简化访问
    inline const std::string& main = OpenXmlNamespaces::spreadsheet_main();
    inline const std::string& rel = OpenXmlNamespaces::relationships();
    inline const std::string& ct = OpenXmlNamespaces::content_types();
    inline const std::string& cp = OpenXmlNamespaces::core_properties();
    inline const std::string& ep = OpenXmlNamespaces::extended_properties();
    inline const std::string& drawing = OpenXmlNamespaces::drawing();
    inline const std::string& chart = OpenXmlNamespaces::chart();
    inline const std::string& pkg_rel = OpenXmlNamespaces::package_relationships();

    // 常用前缀
    inline const std::string& r_prefix = OpenXmlNamespaces::relationships_prefix();
    inline const std::string& a_prefix = OpenXmlNamespaces::drawing_prefix();
    inline const std::string& c_prefix = OpenXmlNamespaces::chart_prefix();
}

} // namespace tinakit::excel
