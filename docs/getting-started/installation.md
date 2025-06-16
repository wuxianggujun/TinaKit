# 安装指南

本指南将帮助你在不同平台上安装和配置 TinaKit。

## 📋 系统要求

### 编译器支持

TinaKit 需要支持 C++20 的现代编译器：

| 编译器 | 最低版本 | 推荐版本 |
|--------|----------|----------|
| GCC | 10.0 | 12.0+ |
| Clang | 12.0 | 15.0+ |
| MSVC | 2019 16.8 | 2022+ |
| Apple Clang | 13.0 | 14.0+ |

### 构建工具

- **CMake**: 3.18 或更高版本
- **包管理器**: vcpkg (推荐) 或 Conan

### 依赖库

TinaKit 依赖以下第三方库：

- **pugixml**: XML 解析
- **zlib**: 压缩支持
- **fmt**: 格式化输出 (可选，C++20 std::format 的后备)

## 🚀 快速安装

### 方法 1: 使用 vcpkg (推荐)

```bash
# 1. 安装 vcpkg (如果还没有)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
# 或
./bootstrap-vcpkg.bat  # Windows

# 2. 安装 TinaKit
./vcpkg install tinakit

# 3. 在你的 CMakeLists.txt 中使用
find_package(TinaKit CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE TinaKit::TinaKit)
```

### 方法 2: 使用 CMake FetchContent

在你的 `CMakeLists.txt` 中添加：

```cmake
cmake_minimum_required(VERSION 3.18)
project(YourProject)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 获取 TinaKit
include(FetchContent)
FetchContent_Declare(
    TinaKit
    GIT_REPOSITORY https://github.com/your-username/TinaKit.git
    GIT_TAG v1.0.0  # 或使用 main 分支
)
FetchContent_MakeAvailable(TinaKit)

# 链接到你的目标
add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE TinaKit::TinaKit)
```

### 方法 3: 从源码构建

```bash
# 1. 克隆仓库
git clone https://github.com/your-username/TinaKit.git
cd TinaKit

# 2. 创建构建目录
mkdir build && cd build

# 3. 配置 CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. 构建
cmake --build . --config Release

# 5. 安装 (可选)
cmake --install . --prefix /usr/local
```

## 🔧 详细配置

### Windows 配置

#### 使用 Visual Studio

1. **安装 Visual Studio 2019/2022**
   - 确保安装了 "C++ CMake tools for Visual Studio"
   - 确保安装了 "MSVC v143 - VS 2022 C++ x64/x86 build tools"

2. **配置 vcpkg**
   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

3. **安装依赖**
   ```cmd
   .\vcpkg install pugixml:x64-windows
   .\vcpkg install zlib:x64-windows
   .\vcpkg install fmt:x64-windows
   ```

#### 使用 MinGW-w64

```bash
# 使用 MSYS2
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-pugixml
pacman -S mingw-w64-x86_64-zlib
```

### Linux 配置

#### Ubuntu/Debian

```bash
# 安装编译器和构建工具
sudo apt update
sudo apt install build-essential cmake git

# 安装 GCC 12 (如果需要)
sudo apt install gcc-12 g++-12
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100

# 安装依赖
sudo apt install libpugixml-dev zlib1g-dev libfmt-dev
```

#### CentOS/RHEL/Fedora

```bash
# Fedora
sudo dnf install gcc-c++ cmake git pugixml-devel zlib-devel fmt-devel

# CentOS/RHEL (需要 EPEL)
sudo yum install epel-release
sudo yum install gcc-c++ cmake3 git pugixml-devel zlib-devel
```

### macOS 配置

#### 使用 Homebrew

```bash
# 安装 Homebrew (如果还没有)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install cmake pugixml zlib fmt

# 安装现代编译器 (可选)
brew install llvm
```

#### 使用 MacPorts

```bash
sudo port install cmake pugixml zlib fmt
```

## ✅ 验证安装

创建一个简单的测试程序来验证安装：

**test_installation.cpp**:
```cpp
#include <tinakit/tinakit.hpp>
#include <iostream>

int main() {
    try {
        std::cout << "TinaKit 版本: " << tinakit::version() << std::endl;
        std::cout << "支持的格式: ";
        
        for (const auto& format : tinakit::supported_formats()) {
            std::cout << format << " ";
        }
        std::cout << std::endl;
        
        std::cout << "✅ TinaKit 安装成功！" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
}
```

**CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.18)
project(TestTinaKit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(TinaKit CONFIG REQUIRED)

add_executable(test_installation test_installation.cpp)
target_link_libraries(test_installation PRIVATE TinaKit::TinaKit)
```

构建和运行：
```bash
mkdir build && cd build
cmake ..
cmake --build .
./test_installation  # Linux/macOS
# 或
.\test_installation.exe  # Windows
```

## 🐛 常见问题

### 编译器不支持 C++20

**问题**: 编译时出现 C++20 特性相关错误

**解决方案**:
1. 升级到支持的编译器版本
2. 确保 CMake 正确设置了 C++20 标准
3. 检查编译器标志是否正确

### 找不到依赖库

**问题**: CMake 无法找到 pugixml 或其他依赖

**解决方案**:
```cmake
# 在 CMakeLists.txt 中添加
set(CMAKE_PREFIX_PATH "/path/to/your/libs" ${CMAKE_PREFIX_PATH})

# 或使用 pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(PUGIXML REQUIRED pugixml)
```

### vcpkg 集成问题

**问题**: vcpkg 安装的库无法被找到

**解决方案**:
```bash
# 确保正确集成 vcpkg
./vcpkg integrate install

# 在 CMake 配置时指定工具链
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## 📞 获取帮助

如果遇到安装问题：

1. 查看 [故障排除指南](../guides/troubleshooting.md)
2. 在 [GitHub Issues](https://github.com/your-username/TinaKit/issues) 中搜索类似问题
3. 创建新的 Issue 并提供详细的错误信息

---

安装完成后，继续阅读 [编译指南](compilation.md) 了解如何从源码构建项目。
