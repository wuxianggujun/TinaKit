/**
 * @file font_subsetter.cpp
 * @brief 字体子集化工具实现
 * @author TinaKit Team
 * @date 2025-6-23
 */

#include "tinakit/pdf/core/font_subsetter.hpp"
#include "tinakit/core/logger.hpp"
#include <fstream>
#include <sstream>
#include <random>
#include <filesystem>
#include <cstdlib>
#include <iomanip>

namespace tinakit::pdf::core {

std::string FontSubsetter::pyftsubset_path_ = "pyftsubset";

FontSubsetter::FontSubsetter() {
    PDF_DEBUG("FontSubsetter initialized");
}

FontSubsetter::~FontSubsetter() {
    PDF_DEBUG("FontSubsetter destroyed");
}

bool FontSubsetter::createSubset(const std::string& input_font_path,
                                const std::string& output_font_path,
                                const std::set<uint32_t>& used_codepoints,
                                const std::string& font_name_prefix) {
    if (!std::filesystem::exists(input_font_path)) {
        PDF_ERROR("Input font file not found: " + input_font_path);
        return false;
    }
    
    if (used_codepoints.empty()) {
        PDF_WARN("No codepoints provided for font subsetting");
        return false;
    }
    
    // 记录统计信息
    original_size_ = std::filesystem::file_size(input_font_path);
    codepoints_count_ = used_codepoints.size();
    
    PDF_DEBUG("Creating font subset: " + std::to_string(codepoints_count_) + " codepoints, " +
              "original size: " + std::to_string(original_size_) + " bytes");
    
    // 创建临时码点文件
    std::string temp_codepoints_file = output_font_path + ".codepoints.txt";
    if (!writeCodepointsToFile(used_codepoints, temp_codepoints_file)) {
        return false;
    }
    
    // 调用pyftsubset
    bool success = callPyftsubset(input_font_path, output_font_path, 
                                 temp_codepoints_file, font_name_prefix);
    
    // 清理临时文件
    std::filesystem::remove(temp_codepoints_file);
    
    if (success && std::filesystem::exists(output_font_path)) {
        subset_size_ = std::filesystem::file_size(output_font_path);
        PDF_DEBUG("Font subset created successfully: " + std::to_string(subset_size_) + " bytes " +
                  "(" + std::to_string(100.0 * subset_size_ / original_size_) + "% of original)");
    }
    
    return success;
}

bool FontSubsetter::createSubsetFromMemory(const std::vector<uint8_t>& input_font_data,
                                          const std::string& output_font_path,
                                          const std::set<uint32_t>& used_codepoints,
                                          const std::string& font_name_prefix) {
    if (input_font_data.empty()) {
        PDF_ERROR("Empty font data provided");
        return false;
    }
    
    // 创建临时输入文件
    std::string temp_input_file = output_font_path + ".temp.otf";
    std::ofstream temp_file(temp_input_file, std::ios::binary);
    if (!temp_file.is_open()) {
        PDF_ERROR("Failed to create temporary font file: " + temp_input_file);
        return false;
    }
    
    temp_file.write(reinterpret_cast<const char*>(input_font_data.data()), input_font_data.size());
    temp_file.close();
    
    // 调用标准的子集化方法
    bool success = createSubset(temp_input_file, output_font_path, used_codepoints, font_name_prefix);
    
    // 清理临时文件
    std::filesystem::remove(temp_input_file);
    
    return success;
}

std::string FontSubsetter::generateFontNamePrefix() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 25);
    
    std::string prefix;
    for (int i = 0; i < 6; ++i) {
        prefix += static_cast<char>('A' + dis(gen));
    }
    prefix += "+";
    
    return prefix;
}

bool FontSubsetter::isPyftsubsetAvailable() {
    std::string command = pyftsubset_path_ + " --help > /dev/null 2>&1";
    int result = std::system(command.c_str());
    return (result == 0);
}

void FontSubsetter::setPyftsubsetPath(const std::string& path) {
    pyftsubset_path_ = path;
}

std::string FontSubsetter::getStatistics() const {
    std::ostringstream oss;
    oss << "Font Subsetting Statistics:\n";
    oss << "  Original Size: " << original_size_ << " bytes\n";
    oss << "  Subset Size: " << subset_size_ << " bytes\n";
    oss << "  Compression Ratio: " << std::fixed << std::setprecision(1) 
        << (original_size_ > 0 ? 100.0 * subset_size_ / original_size_ : 0.0) << "%\n";
    oss << "  Codepoints: " << codepoints_count_ << "\n";
    oss << "  Size Reduction: " << (original_size_ - subset_size_) << " bytes\n";
    return oss.str();
}

bool FontSubsetter::writeCodepointsToFile(const std::set<uint32_t>& codepoints,
                                         const std::string& file_path) const {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        PDF_ERROR("Failed to create codepoints file: " + file_path);
        return false;
    }
    
    for (uint32_t codepoint : codepoints) {
        file << "U+" << std::hex << std::uppercase << codepoint << "\n";
    }
    
    file.close();
    PDF_DEBUG("Written " + std::to_string(codepoints.size()) + " codepoints to: " + file_path);
    return true;
}

bool FontSubsetter::callPyftsubset(const std::string& input_font_path,
                                  const std::string& output_font_path,
                                  const std::string& codepoints_file,
                                  const std::string& font_name_prefix) const {
    std::ostringstream command;
    command << pyftsubset_path_ << " \"" << input_font_path << "\"";
    command << " --text-file=\"" << codepoints_file << "\"";
    command << " --output-file=\"" << output_font_path << "\"";
    command << " --flavor=truetype";
    command << " --layout-features=''";  // 只保留基本字形
    command << " --no-hinting";          // 移除hinting以减小文件大小
    
    if (!font_name_prefix.empty()) {
        command << " --name-IDs='*'";    // 保留所有名称ID
        // 注意：pyftsubset不直接支持名称前缀，需要后处理
    }
    
    std::string cmd_str = command.str();
    PDF_DEBUG("Executing: " + cmd_str);
    
    int result = std::system(cmd_str.c_str());
    if (result != 0) {
        PDF_ERROR("pyftsubset failed with exit code: " + std::to_string(result));
        return false;
    }
    
    return true;
}

} // namespace tinakit::pdf::core
