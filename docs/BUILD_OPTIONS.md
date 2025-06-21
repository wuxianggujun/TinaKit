# TinaKit 构建选项

TinaKit 提供了多种构建选项，让您可以根据需要选择性地构建不同的组件。

## 🎯 **主要构建选项**

### **核心选项**

| 选项 | 默认值 | 描述 |
|------|--------|------|
| `TINAKIT_BUILD_TESTS` | `ON` | 构建测试程序 |
| `TINAKIT_BUILD_EXAMPLES` | `ON` | 构建示例程序 |

### **示例程序细分选项**

当 `TINAKIT_BUILD_EXAMPLES=ON` 时，可以进一步控制具体构建哪些示例：

| 选项 | 默认值 | 描述 |
|------|--------|------|
| `TINAKIT_BUILD_CORE_EXAMPLES` | `ON` | 核心功能示例 |
| `TINAKIT_BUILD_STYLE_EXAMPLES` | `ON` | 样式和格式示例 |
| `TINAKIT_BUILD_PERFORMANCE_EXAMPLES` | `ON` | 性能测试示例 |
| `TINAKIT_BUILD_DEBUG_EXAMPLES` | `OFF` | 调试和工具示例 |

## 🚀 **常用构建命令**

### **完整构建（默认）**
```bash
cmake -B build
cmake --build build --config Release
```

### **仅构建库（不构建示例和测试）**
```bash
cmake -B build -DTINAKIT_BUILD_TESTS=OFF -DTINAKIT_BUILD_EXAMPLES=OFF
cmake --build build --config Release --target tinakit
```

### **构建库和测试（不构建示例）**
```bash
cmake -B build -DTINAKIT_BUILD_EXAMPLES=OFF
cmake --build build --config Release
```

### **仅构建核心示例**
```bash
cmake -B build \
  -DTINAKIT_BUILD_STYLE_EXAMPLES=OFF \
  -DTINAKIT_BUILD_PERFORMANCE_EXAMPLES=OFF \
  -DTINAKIT_BUILD_DEBUG_EXAMPLES=OFF
cmake --build build --config Release
```

### **开发者模式（包含调试工具）**
```bash
cmake -B build -DTINAKIT_BUILD_DEBUG_EXAMPLES=ON
cmake --build build --config Release
```

## 🧪 **测试相关命令**

### **运行所有测试**
```bash
cmake --build build --target run_tests
# 或者
cd build && ctest --output-on-failure
```

### **运行特定测试套件**
```bash
./build/tests/tinakit_tests CoordinateSystem
./build/tests/tinakit_tests RangeSystem
```

### **详细测试输出**
```bash
cmake --build build --target run_tests_verbose
```

## 🎯 **便利构建目标**

TinaKit 提供了一些便利的构建目标：

| 目标 | 描述 |
|------|------|
| `build_all` | 构建所有启用的组件 |
| `run_tests` | 运行所有测试 |
| `run_tests_verbose` | 运行测试（详细输出） |
| `quick_test` | 快速测试 |

### **使用便利目标**
```bash
# 构建所有组件
cmake --build build --target build_all

# 快速测试
cmake --build build --target quick_test
```

## 📊 **示例程序分类**

### **核心功能示例** (`TINAKIT_BUILD_CORE_EXAMPLES`)
- `tinakit_xml_parser_example` - XML解析示例
- `stream_demo` - 流式处理示例
- `xlsx_reader_test` - Excel读取测试
- `excel_read_edit_test` - Excel读写测试

### **样式和格式示例** (`TINAKIT_BUILD_STYLE_EXAMPLES`)
- `comprehensive_style_test` - 综合样式测试
- `excel_with_styles` - 带样式的Excel示例
- `conditional_format_test` - 条件格式测试
- `style_template_test` - 样式模板测试

### **性能测试示例** (`TINAKIT_BUILD_PERFORMANCE_EXAMPLES`)
- `unified_performance_test` - 统一性能测试

### **调试和工具示例** (`TINAKIT_BUILD_DEBUG_EXAMPLES`)
- `xlsx_archiver_demo` - 压缩包操作演示
- `debug_excel_xml` - Excel XML调试工具
- `debug_style_test` - 样式调试测试

## 💡 **推荐配置**

### **用户使用**
```bash
cmake -B build -DTINAKIT_BUILD_DEBUG_EXAMPLES=OFF
```

### **开发调试**
```bash
cmake -B build -DTINAKIT_BUILD_DEBUG_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug
```

### **CI/CD**
```bash
cmake -B build -DTINAKIT_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
```

### **性能测试**
```bash
cmake -B build \
  -DTINAKIT_BUILD_CORE_EXAMPLES=OFF \
  -DTINAKIT_BUILD_STYLE_EXAMPLES=OFF \
  -DTINAKIT_BUILD_DEBUG_EXAMPLES=OFF \
  -DCMAKE_BUILD_TYPE=Release
```

## 🔧 **高级选项**

如果您需要更精细的控制，可以直接编辑 `CMakeLists.txt` 或使用 CMake GUI 工具来配置构建选项。

构建配置信息会在 CMake 配置阶段显示，帮助您确认当前的构建设置。
