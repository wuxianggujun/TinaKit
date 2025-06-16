//
// Created by wuxianggujun on 2025/6/16.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <tinakit/tinakit.hpp>

using namespace tinakit;

void test_xml_parser_basic() {
    std::cout << "\n=== Testing Basic XML Parser ===" << std::endl;

    try {
        // Create a simple XML string for testing (without namespace prefixes for now)
        std::string xml_content = R"(<?xml version="1.0" encoding="UTF-8"?>
<workbook>
    <sheets>
        <sheet name="Sheet1" sheetId="1" id="rId1"/>
        <sheet name="Sheet2" sheetId="2" id="rId2"/>
    </sheets>
    <definedNames>
        <definedName name="TestRange">Sheet1!$A$1:$C$10</definedName>
    </definedNames>
</workbook>)";

        // Create a string stream for testing
        std::istringstream xml_stream(xml_content);

        // Test the XML parser
        tinakit::io::XmlParser parser(xml_stream, "test_workbook.xml");

        std::cout << "✅ XML Parser created successfully" << std::endl;

        // Iterate through XML elements
        int element_count = 0;
        for (auto it = parser.begin(); it != parser.end(); ++it) {
            element_count++;

            if (it.is_start_element()) {
                std::cout << "📖 Start Element: " << it.name();

                // Check for attributes
                if (it.name() == "sheet") {
                    auto name_attr = it.attribute("name");
                    auto id_attr = it.attribute("sheetId");
                    if (name_attr) {
                        std::cout << " [name=" << *name_attr << "]";
                    }
                    if (id_attr) {
                        std::cout << " [sheetId=" << *id_attr << "]";
                    }
                }
                std::cout << std::endl;
            }
            else if (it.is_end_element()) {
                std::cout << "📕 End Element: " << it.name() << std::endl;
            }

            // Prevent infinite loops in case of issues
            if (element_count > 100) {
                std::cout << "⚠️  Stopping iteration after 100 elements" << std::endl;
                break;
            }
        }

        std::cout << "✅ XML parsing completed. Total elements processed: " << element_count << std::endl;

    } catch (const ParseException& e) {
        std::cerr << "❌ Parse Error: " << e.what() << std::endl;
    } catch (const TinaKitException& e) {
        std::cerr << "❌ TinaKit Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ System Error: " << e.what() << std::endl;
    }
}

void test_xml_parser_file() {
    std::cout << "\n=== Testing XML Parser with File ===" << std::endl;

    try {
        // Create a test XML file
        std::string test_file = "test_sample.xml";
        std::ofstream file(test_file);

        if (!file.is_open()) {
            std::cerr << "❌ Failed to create test file" << std::endl;
            return;
        }

        file << R"(<?xml version="1.0" encoding="UTF-8"?>
<root>
    <data id="1" type="string">Hello World</data>
    <data id="2" type="number">42</data>
    <nested>
        <item value="test1"/>
        <item value="test2"/>
    </nested>
</root>)";
        file.close();

        std::cout << "✅ Test XML file created: " << test_file << std::endl;

        // Test parsing the file
        tinakit::io::XmlParser parser(test_file);

        std::cout << "✅ XML Parser created from file" << std::endl;

        // Parse and display structure
        for (auto it = parser.begin(); it != parser.end(); ++it) {
            if (it.is_start_element()) {
                std::cout << "📖 Element: " << it.name();

                // Show attributes for data elements
                if (it.name() == "data") {
                    auto id_attr = it.attribute("id");
                    auto type_attr = it.attribute("type");
                    if (id_attr) std::cout << " [id=" << *id_attr << "]";
                    if (type_attr) std::cout << " [type=" << *type_attr << "]";
                }
                else if (it.name() == "item") {
                    auto value_attr = it.attribute("value");
                    if (value_attr) std::cout << " [value=" << *value_attr << "]";
                }

                std::cout << std::endl;
            }
        }

        std::cout << "✅ File parsing completed successfully" << std::endl;

        // Clean up test file
        std::remove(test_file.c_str());
        std::cout << "🧹 Test file cleaned up" << std::endl;

    } catch (const FileNotFoundException& e) {
        std::cerr << "❌ File Not Found: " << e.file_path() << std::endl;
    } catch (const ParseException& e) {
        std::cerr << "❌ Parse Error: " << e.what() << std::endl;
    } catch (const TinaKitException& e) {
        std::cerr << "❌ TinaKit Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ System Error: " << e.what() << std::endl;
    }
}

void test_xml_parser_namespace() {
    std::cout << "\n=== Testing XML Parser with Namespaces ===" << std::endl;

    try {
        // Test with properly declared namespaces
        std::string namespace_xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"
          xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
    <sheets>
        <sheet name="Sheet1" sheetId="1" r:id="rId1"/>
        <sheet name="Sheet2" sheetId="2" r:id="rId2"/>
    </sheets>
</workbook>)";

        std::istringstream ns_stream(namespace_xml);
        tinakit::io::XmlParser parser(ns_stream, "namespace_test.xml");

        std::cout << "✅ XML Parser with namespaces created successfully" << std::endl;

        // Parse namespace-aware XML
        for (auto it = parser.begin(); it != parser.end(); ++it) {
            if (it.is_start_element()) {
                std::cout << "📖 Element: " << it.name();

                if (it.name() == "sheet") {
                    auto name_attr = it.attribute("name");
                    auto id_attr = it.attribute("sheetId");
                    if (name_attr) std::cout << " [name=" << *name_attr << "]";
                    if (id_attr) std::cout << " [sheetId=" << *id_attr << "]";
                }
                std::cout << std::endl;
            }
        }

        std::cout << "✅ Namespace XML parsing completed successfully" << std::endl;

    } catch (const ParseException& e) {
        std::cout << "⚠️  Parse error with namespaces: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "⚠️  Error with namespaces: " << e.what() << std::endl;
    }
}

void test_xml_parser_error_handling() {
    std::cout << "\n=== Testing XML Parser Error Handling ===" << std::endl;

    try {
        // Test with invalid XML
        std::string invalid_xml = R"(<?xml version="1.0"?>
<root>
    <unclosed_tag>
    <another>content</another>
</root>)";

        std::istringstream invalid_stream(invalid_xml);
        tinakit::io::XmlParser parser(invalid_stream, "invalid_test.xml");

        std::cout << "Testing with invalid XML..." << std::endl;

        // Try to parse invalid XML
        for (auto it = parser.begin(); it != parser.end(); ++it) {
            if (it.is_start_element()) {
                std::cout << "Element: " << it.name() << std::endl;
            }
        }

        std::cout << "⚠️  Invalid XML was parsed without errors (unexpected)" << std::endl;

    } catch (const ParseException& e) {
        std::cout << "✅ Parse error correctly caught: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✅ Error correctly caught: " << e.what() << std::endl;
    }

    try {
        // Test with non-existent file
        std::cout << "Testing with non-existent file..." << std::endl;
        tinakit::io::XmlParser parser("non_existent_file.xml");

        std::cout << "⚠️  Non-existent file was opened without errors (unexpected)" << std::endl;

    } catch (const FileNotFoundException& e) {
        std::cout << "✅ File not found error correctly caught: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✅ Error correctly caught: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "TinaKit XML Parser Example" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "This example demonstrates TinaKit's XML parsing capabilities." << std::endl;
    std::cout << std::endl;

    // Run all tests
    test_xml_parser_basic();
    test_xml_parser_file();
    test_xml_parser_namespace();
    test_xml_parser_error_handling();

    std::cout << "\n🎉 All examples completed!" << std::endl;

    return 0;
}
