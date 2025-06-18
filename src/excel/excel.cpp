/**
 * @file excel.cpp
 * @brief Excel 命名空间实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/tinakit.hpp"
#include "tinakit/excel/workbook.hpp"
#include <unordered_map>
#include <string>
#include <functional>

namespace tinakit::Excel {

// 全局函数注册表
static std::unordered_map<std::string, std::function<double(const std::vector<double>&)>> custom_functions_;

excel::Workbook open(const std::filesystem::path& path) {
    return excel::Workbook::open(path);
}

async::Task<excel::Workbook> open_async(const std::filesystem::path& path) {
    co_return co_await excel::Workbook::open_async(path);
}

excel::Workbook create() {
    return excel::Workbook::create();
}

void register_function(std::string_view name,
                      std::function<double(const std::vector<double>&)> function) {
    custom_functions_[std::string(name)] = std::move(function);
}

// 获取已注册的函数（内部使用）
const std::unordered_map<std::string, std::function<double(const std::vector<double>&)>>& 
get_custom_functions() {
    return custom_functions_;
}

} // namespace tinakit::Excel 