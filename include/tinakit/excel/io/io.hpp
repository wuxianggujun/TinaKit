//
// Created by wuxianggujun on 2025/6/17.
//

#pragma once

#include "tinakit/core/async.hpp"
#include <string>
#include <vector>
#include <cstddef>

namespace tinakit::io
{
    async::Task<void> write_file_binary(const std::string& path, const std::vector<std::byte>& data);

    async::Task<std::vector<std::byte>> read_file_binary(const std::string& path);
}
