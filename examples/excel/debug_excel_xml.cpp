/**
 * @file debug_excel_xml.cpp
 * @brief 调试工具：解压Excel文件并查看XML内容
 */

#include <iostream>
#include <fstream>
#include <filesystem>
#include "tinakit/core/openxml_archiver.hpp"
#include "tinakit/core/async.hpp"

using namespace tinakit;

int main() {
    try {
        std::cout << "=== Excel XML 调试工具 ===" << std::endl;
        
        // 打开生成的Excel文件
        std::string excel_path = "comprehensive_style_test.xlsx";
        if (!std::filesystem::exists(excel_path)) {
            std::cout << "错误：找不到文件 " << excel_path << std::endl;
            return 1;
        }
        
        std::cout << "正在解析文件: " << excel_path << std::endl;
        
        // 使用OpenXmlArchiver打开文件
        auto archiver = async::sync_wait(core::OpenXmlArchiver::open_from_file(excel_path));
        
        // 检查条件格式工作表的XML
        std::string worksheet_path = "xl/worksheets/sheet6.xml";  // 条件格式是第6个工作表
        
        std::cout << "\n=== 检查工作表XML: " << worksheet_path << " ===" << std::endl;
        
        if (async::sync_wait(archiver.has_file(worksheet_path))) {
            auto xml_data = async::sync_wait(archiver.read_file(worksheet_path));
            std::string xml_content(
                reinterpret_cast<const char*>(xml_data.data()),
                xml_data.size()
            );
            
            std::cout << "XML内容长度: " << xml_content.length() << " 字节" << std::endl;
            
            // 查找条件格式相关的XML
            size_t cf_pos = xml_content.find("conditionalFormatting");
            if (cf_pos != std::string::npos) {
                std::cout << "\n✅ 找到条件格式XML！" << std::endl;
                
                // 提取条件格式部分（前后各200字符）
                size_t start = (cf_pos > 200) ? cf_pos - 200 : 0;
                size_t end = std::min(cf_pos + 1000, xml_content.length());
                
                std::cout << "\n=== 条件格式XML片段 ===" << std::endl;
                std::cout << xml_content.substr(start, end - start) << std::endl;
                
                // 保存完整的工作表XML到文件
                std::ofstream debug_file("debug_worksheet6.xml");
                debug_file << xml_content;
                debug_file.close();
                std::cout << "\n📁 完整XML已保存到: debug_worksheet6.xml" << std::endl;
                
            } else {
                std::cout << "\n❌ 没有找到条件格式XML！" << std::endl;
                
                // 查看XML的结构
                std::cout << "\n=== XML结构预览 ===" << std::endl;
                if (xml_content.length() > 1000) {
                    std::cout << xml_content.substr(0, 500) << std::endl;
                    std::cout << "..." << std::endl;
                    std::cout << xml_content.substr(xml_content.length() - 500) << std::endl;
                } else {
                    std::cout << xml_content << std::endl;
                }
            }
            
        } else {
            std::cout << "❌ 工作表文件不存在: " << worksheet_path << std::endl;
            
            // 列出所有工作表文件
            std::cout << "\n=== 可用的工作表文件 ===" << std::endl;
            for (int i = 1; i <= 10; ++i) {
                std::string sheet_path = "xl/worksheets/sheet" + std::to_string(i) + ".xml";
                if (async::sync_wait(archiver.has_file(sheet_path))) {
                    std::cout << "  ✅ " << sheet_path << std::endl;
                } else {
                    std::cout << "  ❌ " << sheet_path << std::endl;
                }
            }
        }
        
        // 检查styles.xml
        std::cout << "\n=== 检查样式文件 ===" << std::endl;
        std::string styles_path = "xl/styles.xml";
        if (async::sync_wait(archiver.has_file(styles_path))) {
            auto styles_data = async::sync_wait(archiver.read_file(styles_path));
            std::string styles_content(
                reinterpret_cast<const char*>(styles_data.data()),
                styles_data.size()
            );

            std::cout << "styles.xml文件大小: " << styles_content.length() << " 字节" << std::endl;

            // 查找dxfs（差异格式）
            size_t dxfs_pos = styles_content.find("dxfs");
            if (dxfs_pos != std::string::npos) {
                std::cout << "✅ 找到dxfs定义" << std::endl;

                // 查找完整的dxfs部分
                size_t dxfs_start = styles_content.find("<dxfs", dxfs_pos);
                size_t dxfs_end = styles_content.find("</dxfs>", dxfs_pos);

                std::cout << "dxfs_start位置: " << dxfs_start << std::endl;
                std::cout << "dxfs_end位置: " << dxfs_end << std::endl;

                // 保存完整的styles.xml到文件
                std::ofstream styles_debug_file("debug_styles.xml");
                styles_debug_file << styles_content;
                styles_debug_file.close();
                std::cout << "📁 完整styles.xml已保存到: debug_styles.xml" << std::endl;

                if (dxfs_start != std::string::npos && dxfs_end != std::string::npos) {
                    dxfs_end += 7; // 包含</dxfs>
                    std::cout << "\n=== 完整的dxfs部分 ===" << std::endl;
                    std::cout << styles_content.substr(dxfs_start, dxfs_end - dxfs_start) << std::endl;
                } else {
                    std::cout << "❌ dxfs部分不完整，显示dxfs附近内容:" << std::endl;
                    size_t start = (dxfs_pos > 200) ? dxfs_pos - 200 : 0;
                    size_t end = std::min(dxfs_pos + 1000, styles_content.length());
                    std::cout << styles_content.substr(start, end - start) << std::endl;
                }
            } else {
                std::cout << "❌ 没有找到dxfs定义" << std::endl;
            }
        }
        
        std::cout << "\n=== 调试完成 ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
