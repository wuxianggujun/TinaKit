/**
 * @file image.cpp
 * @brief TinaKit核心图像处理类实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/core/image.hpp"
#include "tinakit/core/logger.hpp"

// 将STB_IMAGE_IMPLEMENTATION定义移到这里，确保它只被编译一次
#define STB_IMAGE_IMPLEMENTATION
#include "tinakit/internal/stb_image.h"

#include <cstring>
#include <algorithm>

namespace tinakit::core {

// ========================================
// Image::Impl - PIMPL实现类
// ========================================

class Image::Impl {
public:
    Impl() : width_(0), height_(0), channels_(0), data_(nullptr) {}

    ~Impl() {
        clear();
    }

    // 拷贝构造
    Impl(const Impl& other) : width_(0), height_(0), channels_(0), data_(nullptr) {
        copyFrom(other);
    }

    // 拷贝赋值
    Impl& operator=(const Impl& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // 移动构造
    Impl(Impl&& other) noexcept 
        : width_(other.width_), height_(other.height_), channels_(other.channels_), 
          data_(other.data_), last_error_(std::move(other.last_error_)) {
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
        other.data_ = nullptr;
    }

    // 移动赋值
    Impl& operator=(Impl&& other) noexcept {
        if (this != &other) {
            clear();
            width_ = other.width_;
            height_ = other.height_;
            channels_ = other.channels_;
            data_ = other.data_;
            last_error_ = std::move(other.last_error_);
            
            other.width_ = 0;
            other.height_ = 0;
            other.channels_ = 0;
            other.data_ = nullptr;
        }
        return *this;
    }

    bool loadFromFile(const std::string& filename) {
        clear();
        last_error_.clear();

        data_ = stbi_load(filename.c_str(), &width_, &height_, &channels_, 0);
        
        if (!data_) {
            last_error_ = "Failed to load image: " + filename + " - " + std::string(stbi_failure_reason());
            CORE_ERROR("Image loading failed: " + last_error_);
            return false;
        }

        CORE_DEBUG("Image loaded successfully: " + filename + " (" +
                  std::to_string(width_) + "x" + std::to_string(height_) +
                  ", " + std::to_string(channels_) + " channels)");
        return true;
    }

    bool loadFromMemory(const std::uint8_t* buffer, std::size_t length) {
        clear();
        last_error_.clear();

        if (!buffer || length == 0) {
            last_error_ = "Invalid buffer or length";
            CORE_ERROR("Image loading failed: " + last_error_);
            return false;
        }

        data_ = stbi_load_from_memory(buffer, static_cast<int>(length), &width_, &height_, &channels_, 0);

        if (!data_) {
            last_error_ = "Failed to load image from memory - " + std::string(stbi_failure_reason());
            CORE_ERROR("Image loading failed: " + last_error_);
            return false;
        }

        CORE_DEBUG("Image loaded from memory successfully (" +
                  std::to_string(width_) + "x" + std::to_string(height_) +
                  ", " + std::to_string(channels_) + " channels)");
        return true;
    }

    void clear() {
        if (data_) {
            stbi_image_free(data_);
            data_ = nullptr;
        }
        width_ = 0;
        height_ = 0;
        channels_ = 0;
        last_error_.clear();
    }

    std::vector<std::uint8_t> getDataCopy() const {
        if (!data_ || width_ <= 0 || height_ <= 0 || channels_ <= 0) {
            return {};
        }

        std::size_t data_size = static_cast<std::size_t>(width_) * height_ * channels_;
        std::vector<std::uint8_t> copy(data_size);
        std::memcpy(copy.data(), data_, data_size);
        return copy;
    }

private:
    void copyFrom(const Impl& other) {
        if (!other.data_ || other.width_ <= 0 || other.height_ <= 0 || other.channels_ <= 0) {
            return;
        }

        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;
        last_error_ = other.last_error_;

        std::size_t data_size = static_cast<std::size_t>(width_) * height_ * channels_;
        data_ = static_cast<std::uint8_t*>(std::malloc(data_size));
        
        if (data_) {
            std::memcpy(data_, other.data_, data_size);
        } else {
            width_ = 0;
            height_ = 0;
            channels_ = 0;
            last_error_ = "Failed to allocate memory for image copy";
        }
    }

public:
    int width_;
    int height_;
    int channels_;
    std::uint8_t* data_;
    std::string last_error_;
};

// ========================================
// Image类实现
// ========================================

Image::Image() : pimpl_(std::make_unique<Impl>()) {}

Image::~Image() = default;

Image::Image(const Image& other) : pimpl_(std::make_unique<Impl>(*other.pimpl_)) {}

Image::Image(Image&& other) noexcept : pimpl_(std::move(other.pimpl_)) {}

Image& Image::operator=(const Image& other) {
    if (this != &other) {
        *pimpl_ = *other.pimpl_;
    }
    return *this;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        pimpl_ = std::move(other.pimpl_);
    }
    return *this;
}

bool Image::loadFromFile(const std::string& filename) {
    return pimpl_->loadFromFile(filename);
}

bool Image::loadFromMemory(const std::uint8_t* buffer, std::size_t length) {
    return pimpl_->loadFromMemory(buffer, length);
}

bool Image::loadFromMemory(const std::vector<std::uint8_t>& buffer) {
    return pimpl_->loadFromMemory(buffer.data(), buffer.size());
}

int Image::getWidth() const { 
    return pimpl_->width_; 
}

int Image::getHeight() const { 
    return pimpl_->height_; 
}

int Image::getChannels() const { 
    return pimpl_->channels_; 
}

const std::uint8_t* Image::getData() const {
    return pimpl_->data_;
}

std::size_t Image::getDataSize() const {
    if (!pimpl_->data_ || pimpl_->width_ <= 0 || pimpl_->height_ <= 0 || pimpl_->channels_ <= 0) {
        return 0;
    }
    return static_cast<std::size_t>(pimpl_->width_) * pimpl_->height_ * pimpl_->channels_;
}

bool Image::isLoaded() const {
    return pimpl_->data_ != nullptr && pimpl_->width_ > 0 && pimpl_->height_ > 0 && pimpl_->channels_ > 0;
}

std::string Image::getLastError() const {
    return pimpl_->last_error_;
}

void Image::clear() {
    pimpl_->clear();
}

std::vector<std::uint8_t> Image::getDataCopy() const {
    return pimpl_->getDataCopy();
}

} // namespace tinakit::core
