/**
 * @file font_config_demo.cpp
 * @brief 字体配置使用示例
 * @author TinaKit Team
 * @date 2025-6-24
 */

#include "tinakit/pdf/document.hpp"
#include "tinakit/pdf/config/font_config.hpp"
#include "tinakit/core/logger.hpp"
#include <iostream>

using namespace tinakit::pdf;
using namespace tinakit::pdf::config;

void demo_minimal_size_config() {
    std::cout << "\n=== 最小文件大小配置示例 ===\n";
    
    // 创建最小文件大小配置
    auto font_config = FontConfigFactory::createMinimalSize();

    // 创建PDF文档并应用配置
    auto doc = Document::create();
    doc.set_font_config(font_config);
    
    // 添加页面和内容
    doc.add_page();
    doc.add_text("Hello World! 你好世界！", {100, 700},
                 Font{"SourceHanSansSC-Regular", 12});

    doc.save("minimal_size_demo.pdf");
    std::cout << "已生成 minimal_size_demo.pdf（使用子集化）\n";
}

void demo_max_compatibility_config() {
    std::cout << "\n=== 最大兼容性配置示例 ===\n";
    
    // 创建最大兼容性配置（使用系统字体）
    auto font_config = FontConfigFactory::createMaxCompatibility();

    auto doc = Document::create();
    doc.set_font_config(font_config);

    doc.add_page();
    doc.add_text("Hello World!", {100, 700}, Font{"Arial", 12});
    doc.add_text("System font text", {100, 650}, Font{"Helvetica", 12});

    doc.save("max_compatibility_demo.pdf");
    std::cout << "已生成 max_compatibility_demo.pdf（使用系统字体）\n";
}

void demo_custom_config() {
    std::cout << "\n=== 自定义配置示例 ===\n";
    
    // 创建自定义配置
    FontConfig font_config(FontEmbeddingStrategy::SUBSET_EMBED);
    
    // 自定义子集化选项
    FontSubsetOptions subset_opts;
    subset_opts.retain_gids = true;
    subset_opts.min_char_threshold = 50;   // 使用50个字符以上才子集化
    subset_opts.max_char_threshold = 5000; // 超过5000字符使用完整嵌入
    subset_opts.compression_threshold = 0.7; // 70%压缩比阈值
    subset_opts.include_basic_charset = true;
    
    font_config.setSubsetOptions(subset_opts);
    
    // 启用字体回退
    font_config.enableFontFallback(true);
    font_config.addFontFallback("MyCustomFont", {"Arial", "Helvetica"});
    
    auto doc = Document::create();
    doc.set_font_config(font_config);

    doc.add_page();
    doc.add_text("Custom configured text with 自定义配置文本", {100, 700},
                 Font{"SourceHanSansSC-Regular", 14});

    doc.save("custom_config_demo.pdf");
    std::cout << "已生成 custom_config_demo.pdf（自定义配置）\n";
}

void demo_individual_font_config() {
    std::cout << "\n=== 单个字体配置示例 ===\n";
    
    // 全局配置：默认使用子集化
    auto global_config = FontConfigFactory::createBalanced();

    auto doc = Document::create();
    doc.set_font_config(global_config);

    // 为特定字体设置个别配置
    IndividualFontConfig title_font_config("SourceHanSansSC-Bold");
    title_font_config.overrideEmbeddingStrategy(FontEmbeddingStrategy::FULL_EMBED); // 标题字体完整嵌入

    IndividualFontConfig body_font_config("SourceHanSansSC-Regular");
    FontSubsetOptions body_subset;
    body_subset.retain_gids = false; // 正文字体可以重新映射GID以获得更小文件
    body_font_config.overrideSubsetOptions(body_subset);

    doc.add_individual_font_config(title_font_config);
    doc.add_individual_font_config(body_font_config);

    doc.add_page();
    doc.add_text("标题文本（完整嵌入）", {100, 750},
                 Font{"SourceHanSansSC-Bold", 18});
    doc.add_text("正文内容使用子集化嵌入，可以重新映射字形ID以获得更小的文件大小。",
                 {100, 700}, Font{"SourceHanSansSC-Regular", 12});

    doc.save("individual_config_demo.pdf");
    std::cout << "已生成 individual_config_demo.pdf（混合配置）\n";
}

void demo_development_mode() {
    std::cout << "\n=== 开发模式配置示例 ===\n";
    
    // 开发模式：快速生成，不进行优化
    auto font_config = FontConfigFactory::createDevelopment();

    auto doc = Document::create();
    doc.set_font_config(font_config);

    doc.add_page();
    doc.add_text("Development mode - fast generation", {100, 700},
                 Font{"Arial", 12});
    doc.add_text("开发模式 - 快速生成", {100, 650},
                 Font{"SimSun", 12});

    doc.save("development_demo.pdf");
    std::cout << "已生成 development_demo.pdf（开发模式，快速生成）\n";
}

void demo_font_loading_strategies() {
    std::cout << "\n=== 字体加载策略对比 ===\n";
    
    // 策略1：不嵌入（最小文件）- 使用系统字体
    {
        auto config = FontConfig(FontEmbeddingStrategy::NONE);
        auto doc = Document::create();
        doc.set_font_config(config);
        doc.add_page();
        doc.add_text("No embedding - smallest file", {100, 700}, Font{"Arial", 12});
        doc.add_text("系统字体测试", {100, 650}, Font{"SimSun", 12});
        doc.save("no_embed_demo.pdf");
        std::cout << "生成 no_embed_demo.pdf（不嵌入字体）\n";
    }

    // 策略2：完整嵌入（最大兼容性）- 需要字体文件
    {
        auto config = FontConfig(FontEmbeddingStrategy::FULL_EMBED);
        auto doc = Document::create();
        doc.set_font_config(config);
        doc.add_page();
        doc.add_text("Full embedding - largest file 完整嵌入测试", {100, 700},
                     Font{"SourceHanSansSC-Regular", 12});
        doc.save("full_embed_demo.pdf");
        std::cout << "生成 full_embed_demo.pdf（完整嵌入）\n";
    }

    // 策略3：子集化嵌入（平衡）- 需要字体文件
    {
        auto config = FontConfig(FontEmbeddingStrategy::SUBSET_EMBED);
        auto doc = Document::create();
        doc.set_font_config(config);
        doc.add_page();
        doc.add_text("Subset embedding - balanced 子集化测试", {100, 700},
                     Font{"SourceHanSansSC-Regular", 12});
        doc.save("subset_embed_demo.pdf");
        std::cout << "生成 subset_embed_demo.pdf（子集化嵌入）\n";
    }
    
    std::cout << "已生成三个对比文件，可以查看文件大小差异\n";
}

int main() {
    try {
        // 初始化日志系统
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::DEBUG);
        
        std::cout << "TinaKit PDF 字体配置示例\n";
        std::cout << "========================\n";
        
        demo_minimal_size_config();
        demo_max_compatibility_config();
        demo_custom_config();
        demo_individual_font_config();
        demo_development_mode();
        demo_font_loading_strategies();
        
        std::cout << "\n所有示例已完成！\n";
        std::cout << "请检查生成的PDF文件大小和显示效果的差异。\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
