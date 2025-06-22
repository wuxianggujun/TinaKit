/**
 * @file types.cpp
 * @brief PDF模块基础类型实现
 * @author TinaKit Team
 * @date 2025-6-22
 */

#include "tinakit/pdf/types.hpp"
#include <algorithm>

namespace tinakit::pdf {

std::pair<double, double> page_size_to_points(PageSize size, PageOrientation orientation) {
    double width, height;
    
    switch (size) {
        case PageSize::A4:
            width = 595.0;   // 210mm
            height = 842.0;  // 297mm
            break;
        case PageSize::A3:
            width = 842.0;   // 297mm
            height = 1191.0; // 420mm
            break;
        case PageSize::A5:
            width = 420.0;   // 148mm
            height = 595.0;  // 210mm
            break;
        case PageSize::Letter:
            width = 612.0;   // 8.5 inch
            height = 792.0;  // 11 inch
            break;
        case PageSize::Legal:
            width = 612.0;   // 8.5 inch
            height = 1008.0; // 14 inch
            break;
        case PageSize::Tabloid:
            width = 792.0;   // 11 inch
            height = 1224.0; // 17 inch
            break;
        case PageSize::Custom:
        default:
            width = 595.0;   // 默认A4
            height = 842.0;
            break;
    }
    
    if (orientation == PageOrientation::Landscape) {
        std::swap(width, height);
    }
    
    return {width, height};
}

} // namespace tinakit::pdf
