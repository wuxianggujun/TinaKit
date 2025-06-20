/**
 * @file excel.cpp
 * @brief Excel 命名空间函数实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/tinakit.hpp"
#include "tinakit/excel/workbook.hpp"
#include "tinakit/core/async.hpp"
#include <unordered_map>
#include <mutex>

namespace tinakit::excel {

namespace {
    // 存储注册的自定义函数
    std::unordered_map<std::string, std::function<double(const std::vector<double>&)>> g_custom_functions;
    std::mutex g_functions_mutex;
}

Workbook open(const std::filesystem::path& path) {
    return workbook::load(path);
}

async::Task<Workbook> open_async(const std::filesystem::path& path) {
    return workbook::load_async(path);
}

Workbook create() {
    return workbook::create();
}

void register_function(std::string_view name,
                      std::function<double(const std::vector<double>&)> function) {
    std::lock_guard<std::mutex> lock(g_functions_mutex);
    g_custom_functions[std::string(name)] = std::move(function);
}

} // namespace tinakit::excel