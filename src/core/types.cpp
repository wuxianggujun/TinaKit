/**
 * @file types.cpp
 * @brief TinaKit 核心类型实现
 * @author TinaKit Team
 * @date 2025-6-16
 */

#include "tinakit/core/types.hpp"
#include <algorithm>
#include <cctype>

namespace tinakit {

// Document type 实现
DocumentType get_document_type(const std::string& extension) {
    std::string ext = extension;
    if (!ext.empty() && ext[0] != '.') {
        ext = "." + ext;
    }
    
    // 转换为小写
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".xlsx" || ext == ".xlsm" || ext == ".xls") {
        return DocumentType::Excel;
    } else if (ext == ".docx" || ext == ".doc") {
        return DocumentType::Word;
    } else if (ext == ".pptx" || ext == ".ppt") {
        return DocumentType::PowerPoint;
    }
    
    return DocumentType::Unknown;
}

bool is_supported_format(const std::string& extension) {
    return get_document_type(extension) != DocumentType::Unknown;
}

} // namespace tinakit