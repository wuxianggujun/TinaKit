#include <iostream>
#include "tinakit/tinakit.hpp"
#include "tinakit/pdf/core/serializer.hpp"
#include "tinakit/pdf/core/binary_writer.hpp"

using namespace tinakit;
using namespace tinakit::pdf;

void testNewSerializer() {
    std::cout << "🔧 测试新的PdfSerializer..." << std::endl;

    try {
        // 创建二进制写入器
        tinakit::pdf::BinaryWriter writer("test_serializer.pdf");
        tinakit::pdf::core::PdfSerializer serializer(writer);

        // 测试RAII作用域
        {
            tinakit::pdf::core::ObjectScope obj(serializer, 1);
            {
                tinakit::pdf::core::DictScope dict(serializer);
                serializer.dictEntry("Type", "Font");
                serializer.dictEntry("Subtype", "Type0");
                serializer.dictEntryRef("DescendantFonts", 2);
            }
        }

        // 检查错误
        if (serializer.hasError()) {
            std::cerr << "❌ 序列化器错误: " << serializer.getErrorMessage() << std::endl;
        } else {
            std::cout << "✅ 新序列化器测试成功" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ 序列化器测试异常: " << e.what() << std::endl;
    }
}

int main() {
    try {
        // 初始化日志系统
        tinakit::core::initializeDefaultLogging(tinakit::core::LogLevel::DEBUG);

        std::cout << "🔍 PDF简单测试 - 生成最小PDF验证语法" << std::endl;

        // 先测试新的序列化器
        testNewSerializer();

        // 创建PDF文档
        auto pdf = pdf::Document::create();
        
        // 设置文档信息
        pdf::DocumentInfo info;
        info.title = "Simple Test";
        info.author = "TinaKit";
        
        pdf.set_document_info(info);
        pdf.set_page_size(pdf::PageSize::A4);

        // 添加页面
        pdf.add_page();

        // 只添加一行简单的英文文本进行测试
        pdf::Font font;
        font.family = "SimSun";
        font.size = 12;
        font.color = Color::Black;

        pdf.add_text("Hello World", {100, 100}, font);

        // 保存文档
        pdf.save("simple_test.pdf");

        std::cout << "✅ 简单测试PDF已生成: simple_test.pdf" << std::endl;
        std::cout << "请使用PDF阅读器打开查看是否显示文本" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
