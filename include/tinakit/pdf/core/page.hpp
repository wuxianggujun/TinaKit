/**
 * @file page.hpp
 * @brief PDF页面管理
 * @author TinaKit Team
 * @date 2025-6-22
 */

#pragma once

#include "tinakit/pdf/core/object.hpp"
#include "tinakit/core/types.hpp"
#include <vector>
#include <string>
#include <memory>

namespace tinakit::pdf::core {

/**
 * @class PdfPage
 * @brief PDF页面类
 * 
 * 管理单个PDF页面的内容和属性
 */
class PdfPage {
public:
    // ========================================
    // 构造函数和析构函数
    // ========================================
    
    /**
     * @brief 构造函数
     * @param id 页面对象ID
     * @param width 页面宽度（点）
     * @param height 页面高度（点）
     */
    PdfPage(int id, double width = 595.0, double height = 842.0);
    
    /**
     * @brief 析构函数
     */
    ~PdfPage() = default;
    
    // 禁用拷贝，支持移动
    PdfPage(const PdfPage&) = delete;
    PdfPage& operator=(const PdfPage&) = delete;
    PdfPage(PdfPage&&) = default;
    PdfPage& operator=(PdfPage&&) = default;

    // ========================================
    // 页面属性
    // ========================================
    
    /**
     * @brief 获取页面ID
     * @return 页面对象ID
     */
    int getId() const { return id_; }
    
    /**
     * @brief 获取页面宽度
     * @return 宽度（点）
     */
    double getWidth() const { return width_; }
    
    /**
     * @brief 获取页面高度
     * @return 高度（点）
     */
    double getHeight() const { return height_; }
    
    /**
     * @brief 设置页面大小
     * @param width 宽度（点）
     * @param height 高度（点）
     */
    void setSize(double width, double height);
    
    /**
     * @brief 获取页面边界框
     * @return 边界框 [x1, y1, x2, y2]
     */
    std::vector<double> getMediaBox() const;
    
    /**
     * @brief 设置页面边界框
     * @param media_box 边界框 [x1, y1, x2, y2]
     */
    void setMediaBox(const std::vector<double>& media_box);

    // ========================================
    // 内容管理
    // ========================================
    
    /**
     * @brief 添加内容流
     * @param content 内容字符串
     */
    void addContent(const std::string& content);
    
    /**
     * @brief 添加内容流（带换行）
     * @param content 内容字符串
     */
    void addContentLine(const std::string& content);
    
    /**
     * @brief 清空内容
     */
    void clearContent();
    
    /**
     * @brief 获取内容流
     * @return 内容流字符串
     */
    std::string getContentStream() const;
    
    /**
     * @brief 获取内容流大小
     * @return 内容大小（字节）
     */
    std::size_t getContentSize() const;

    // ========================================
    // 图形状态
    // ========================================
    
    /**
     * @brief 保存图形状态
     */
    void saveGraphicsState();
    
    /**
     * @brief 恢复图形状态
     */
    void restoreGraphicsState();
    
    /**
     * @brief 设置变换矩阵
     * @param a 缩放X
     * @param b 倾斜Y
     * @param c 倾斜X
     * @param d 缩放Y
     * @param e 平移X
     * @param f 平移Y
     */
    void setTransform(double a, double b, double c, double d, double e, double f);
    
    /**
     * @brief 平移坐标系
     * @param dx X方向偏移
     * @param dy Y方向偏移
     */
    void translate(double dx, double dy);
    
    /**
     * @brief 缩放坐标系
     * @param sx X方向缩放
     * @param sy Y方向缩放
     */
    void scale(double sx, double sy);
    
    /**
     * @brief 旋转坐标系
     * @param angle 旋转角度（弧度）
     */
    void rotate(double angle);

    // ========================================
    // 文本操作
    // ========================================
    
    /**
     * @brief 开始文本对象
     */
    void beginText();
    
    /**
     * @brief 结束文本对象
     */
    void endText();
    
    /**
     * @brief 设置字体
     * @param font_resource 字体资源名称
     * @param size 字体大小
     * @param subtype 字体子类型（用于确定编码方式）
     */
    void setFont(const std::string& font_resource, double size, const std::string& subtype = "Type1");
    
    /**
     * @brief 设置文本位置
     * @param x X坐标
     * @param y Y坐标
     */
    void setTextPosition(double x, double y);
    
    /**
     * @brief 移动文本位置
     * @param dx X方向偏移
     * @param dy Y方向偏移
     */
    void moveTextPosition(double dx, double dy);
    
    /**
     * @brief 显示文本
     * @param text 文本内容
     */
    void showText(const std::string& text);
    
    /**
     * @brief 显示文本并移动到下一行
     * @param text 文本内容
     */
    void showTextLine(const std::string& text);
    
    /**
     * @brief 设置文本颜色
     * @param r 红色分量 (0-1)
     * @param g 绿色分量 (0-1)
     * @param b 蓝色分量 (0-1)
     */
    void setTextColor(double r, double g, double b);

    // ========================================
    // 图形操作
    // ========================================
    
    /**
     * @brief 设置线宽
     * @param width 线宽
     */
    void setLineWidth(double width);
    
    /**
     * @brief 设置描边颜色
     * @param r 红色分量 (0-1)
     * @param g 绿色分量 (0-1)
     * @param b 蓝色分量 (0-1)
     */
    void setStrokeColor(double r, double g, double b);
    
    /**
     * @brief 设置填充颜色
     * @param r 红色分量 (0-1)
     * @param g 绿色分量 (0-1)
     * @param b 蓝色分量 (0-1)
     */
    void setFillColor(double r, double g, double b);
    
    /**
     * @brief 移动到指定点
     * @param x X坐标
     * @param y Y坐标
     */
    void moveTo(double x, double y);
    
    /**
     * @brief 画线到指定点
     * @param x X坐标
     * @param y Y坐标
     */
    void lineTo(double x, double y);
    
    /**
     * @brief 画矩形
     * @param x X坐标
     * @param y Y坐标
     * @param width 宽度
     * @param height 高度
     */
    void rectangle(double x, double y, double width, double height);
    
    /**
     * @brief 描边路径
     */
    void stroke();
    
    /**
     * @brief 填充路径
     */
    void fill();
    
    /**
     * @brief 填充并描边路径
     */
    void fillAndStroke();
    
    /**
     * @brief 关闭路径
     */
    void closePath();

    // ========================================
    // 图像操作
    // ========================================

    /**
     * @brief 添加图像
     * @param image_resource 图像资源名称
     * @param x X坐标
     * @param y Y坐标
     * @param width 图像宽度
     * @param height 图像高度
     */
    void addImage(const std::string& image_resource, double x, double y, double width, double height);

    // ========================================
    // 高级操作
    // ========================================
    
    /**
     * @brief 添加注释
     * @param comment 注释内容
     */
    void addComment(const std::string& comment);
    
    /**
     * @brief 创建页面对象
     * @param parent_id 父页面树对象ID
     * @param content_id 内容流对象ID
     * @param resources 资源字典
     * @return 页面对象
     */
    std::unique_ptr<DictionaryObject> createPageObject(int parent_id, int content_id,
                                                      const std::string& resources = "") const;

    /**
     * @brief 创建页面对象（指定ID）
     * @param page_id 页面对象ID
     * @param parent_id 父页面树对象ID
     * @param content_id 内容流对象ID
     * @param resources 资源字典
     * @return 页面对象
     */
    std::unique_ptr<DictionaryObject> createPageObjectWithId(int page_id, int parent_id, int content_id,
                                                            const std::string& resources = "") const;
    
    /**
     * @brief 创建内容流对象
     * @param content_id 内容流对象ID
     * @return 内容流对象
     */
    std::unique_ptr<StreamObject> createContentObject(int content_id) const;

private:
    // ========================================
    // 成员变量
    // ========================================
    
    int id_;                    ///< 页面对象ID
    double width_;              ///< 页面宽度
    double height_;             ///< 页面高度
    std::vector<double> media_box_;  ///< 页面边界框
    
    std::vector<std::string> content_stream_;  ///< 内容流
    
    // 状态跟踪
    bool in_text_object_ = false;       ///< 是否在文本对象中
    std::string current_font_subtype_;  ///< 当前字体子类型
    int graphics_state_level_ = 0;      ///< 图形状态嵌套级别
    
    // ========================================
    // 内部方法
    // ========================================
    
    /**
     * @brief 转义PDF文本字符串
     * @param text 原始文本
     * @return 转义后的文本
     */
    std::string escapeText(const std::string& text) const;

    /**
     * @brief 智能分段显示文本
     * @param text 要显示的文本
     *
     * 将中英文混合文本智能分段，为不同类型的文本选择合适的编码方式
     */
    void showTextWithSmartSegmentation(const std::string& text);

    /**
     * @brief 判断字符是否为CJK字符
     * @param codepoint Unicode码点
     * @return 是CJK字符返回true，否则返回false
     */
    bool isCJKCharacter(uint32_t codepoint) const;

    /**
     * @brief 显示ASCII文本段
     * @param text ASCII文本
     */
    void showASCIISegment(const std::string& text);

    /**
     * @brief 显示Unicode文本段
     * @param text Unicode文本
     */
    void showUnicodeSegment(const std::string& text);
    
    /**
     * @brief 格式化浮点数
     * @param value 浮点数值
     * @param precision 精度
     * @return 格式化后的字符串
     */
    std::string formatFloat(double value, int precision = 2) const;
};

} // namespace tinakit::pdf::core
