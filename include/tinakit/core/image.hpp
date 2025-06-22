/**
 * @file image.hpp
 * @brief TinaKit核心图像处理类
 * @author TinaKit Team
 * @date 2025-6-22
 * 
 * 提供统一的图像加载和处理接口，封装stb_image库的功能。
 * 支持JPEG、PNG、BMP、TGA等常见图像格式。
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace tinakit::core {

/**
 * @brief 图像类
 * 
 * 封装图像加载和基本操作功能，使用PIMPL模式隐藏stb_image实现细节。
 * 支持多种图像格式的加载和基本信息查询。
 * 
 * @example
 * ```cpp
 * // 加载图像
 * tinakit::core::Image image;
 * if (image.loadFromFile("photo.jpg")) {
 *     std::cout << "图像尺寸: " << image.getWidth() << "x" << image.getHeight() << std::endl;
 *     std::cout << "颜色通道: " << image.getChannels() << std::endl;
 *     
 *     // 获取原始数据用于进一步处理
 *     const auto* data = image.getData();
 *     // ... 处理图像数据
 * }
 * ```
 */
class Image {
public:
    // ========================================
    // 构造函数和析构函数
    // ========================================
    
    /**
     * @brief 默认构造函数
     */
    Image();
    
    /**
     * @brief 析构函数
     */
    ~Image();
    
    /**
     * @brief 拷贝构造函数
     */
    Image(const Image& other);
    
    /**
     * @brief 移动构造函数
     */
    Image(Image&& other) noexcept;
    
    /**
     * @brief 拷贝赋值操作符
     */
    Image& operator=(const Image& other);
    
    /**
     * @brief 移动赋值操作符
     */
    Image& operator=(Image&& other) noexcept;

    // ========================================
    // 图像加载
    // ========================================
    
    /**
     * @brief 从文件加载图像
     * @param filename 图像文件路径
     * @return 加载成功返回true，失败返回false
     * 
     * 支持的格式：JPEG, PNG, BMP, TGA, PSD, GIF, HDR, PIC, PNM
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * @brief 从内存缓冲区加载图像
     * @param buffer 图像数据缓冲区
     * @param length 缓冲区长度
     * @return 加载成功返回true，失败返回false
     */
    bool loadFromMemory(const std::uint8_t* buffer, std::size_t length);
    
    /**
     * @brief 从内存缓冲区加载图像
     * @param buffer 图像数据缓冲区
     * @return 加载成功返回true，失败返回false
     */
    bool loadFromMemory(const std::vector<std::uint8_t>& buffer);

    // ========================================
    // 图像信息查询
    // ========================================
    
    /**
     * @brief 获取图像宽度
     * @return 图像宽度（像素）
     */
    int getWidth() const;
    
    /**
     * @brief 获取图像高度
     * @return 图像高度（像素）
     */
    int getHeight() const;
    
    /**
     * @brief 获取颜色通道数
     * @return 颜色通道数（1=灰度，3=RGB，4=RGBA）
     */
    int getChannels() const;
    
    /**
     * @brief 获取图像原始数据
     * @return 指向图像数据的指针，如果未加载图像则返回nullptr
     * 
     * 数据格式：每个像素按通道顺序存储，每个通道占用1字节
     * - 灰度图：[Gray]
     * - RGB图：[R, G, B]
     * - RGBA图：[R, G, B, A]
     */
    const std::uint8_t* getData() const;
    
    /**
     * @brief 获取图像数据大小
     * @return 图像数据字节数
     */
    std::size_t getDataSize() const;
    
    /**
     * @brief 检查是否已加载图像
     * @return 已加载图像返回true，否则返回false
     */
    bool isLoaded() const;
    
    /**
     * @brief 获取最后一次操作的错误信息
     * @return 错误信息字符串，无错误时返回空字符串
     */
    std::string getLastError() const;

    // ========================================
    // 图像操作
    // ========================================
    
    /**
     * @brief 清空图像数据
     */
    void clear();
    
    /**
     * @brief 获取图像数据的副本
     * @return 图像数据的副本
     */
    std::vector<std::uint8_t> getDataCopy() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace tinakit::core
