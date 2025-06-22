#!/usr/bin/env python3
"""
TinaKit Release Package Creator
Creates a complete release package for TinaKit v1.0.0
"""

import os
import shutil
import zipfile
import subprocess
import sys
from pathlib import Path

def run_command(cmd, cwd=None):
    """Run a command and return the result"""
    try:
        result = subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Error running command: {cmd}")
            print(f"Error: {result.stderr}")
            return False
        return True
    except Exception as e:
        print(f"Exception running command {cmd}: {e}")
        return False

def create_release_package():
    """Create the complete release package"""
    
    # Get the project root directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    print("🚀 Creating TinaKit v1.0.0 Release Package...")
    print(f"📁 Project root: {project_root}")
    
    # Create release directory
    release_dir = project_root / "release" / "tinakit-v1.0.0"
    if release_dir.exists():
        shutil.rmtree(release_dir)
    release_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"📦 Release directory: {release_dir}")
    
    # 1. Copy source code
    print("📋 Copying source code...")
    source_dirs = ["include", "src", "examples", "tests", "docs", "third_party"]
    for dir_name in source_dirs:
        src_path = project_root / dir_name
        if src_path.exists():
            dst_path = release_dir / dir_name
            shutil.copytree(src_path, dst_path, ignore=shutil.ignore_patterns(
                '*.obj', '*.exe', '*.dll', '*.lib', '*.pdb', 
                '__pycache__', '*.pyc', '.git*', 'build*', 'cmake-build-*'
            ))
            print(f"   ✅ Copied {dir_name}")
    
    # 2. Copy important files
    print("📄 Copying important files...")
    important_files = [
        "README.md", "CHANGELOG.md", "CMakeLists.txt", 
        "LICENSE", ".gitignore"
    ]
    for file_name in important_files:
        src_file = project_root / file_name
        if src_file.exists():
            shutil.copy2(src_file, release_dir / file_name)
            print(f"   ✅ Copied {file_name}")
    
    # 3. Create build scripts
    print("🔧 Creating build scripts...")
    
    # Windows build script
    windows_build = release_dir / "build_windows.bat"
    with open(windows_build, 'w', encoding='utf-8') as f:
        f.write("""@echo off
echo Building TinaKit v1.0.0 for Windows...

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DTINAKIT_BUILD_TESTS=ON -DTINAKIT_BUILD_EXAMPLES=ON

REM Build
cmake --build . --config Release

REM Run tests
echo Running tests...
ctest --output-on-failure

echo Build complete! Check the build directory for binaries.
pause
""")
    
    # Linux/macOS build script
    unix_build = release_dir / "build_unix.sh"
    with open(unix_build, 'w', encoding='utf-8') as f:
        f.write("""#!/bin/bash
echo "Building TinaKit v1.0.0 for Unix/Linux/macOS..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DTINAKIT_BUILD_TESTS=ON -DTINAKIT_BUILD_EXAMPLES=ON

# Build
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Run tests
echo "Running tests..."
ctest --output-on-failure

echo "Build complete! Check the build directory for binaries."
""")
    
    # Make Unix script executable
    os.chmod(unix_build, 0o755)
    
    print("   ✅ Created build scripts")
    
    # 4. Create installation guide
    print("📖 Creating installation guide...")
    install_guide = release_dir / "INSTALL.md"
    with open(install_guide, 'w', encoding='utf-8') as f:
        f.write("""# TinaKit v1.0.0 Installation Guide

## Requirements

- **C++20 compatible compiler**
  - GCC 10+ or Clang 12+ (Linux/macOS)
  - Visual Studio 2019 16.11+ or Visual Studio 2022 (Windows)
- **CMake 3.18 or later**
- **Git** (for cloning dependencies)

## Quick Start

### Windows
1. Open Command Prompt or PowerShell
2. Navigate to the TinaKit directory
3. Run: `build_windows.bat`

### Linux/macOS
1. Open terminal
2. Navigate to the TinaKit directory
3. Run: `chmod +x build_unix.sh && ./build_unix.sh`

### Manual Build
```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release -DTINAKIT_BUILD_TESTS=ON -DTINAKIT_BUILD_EXAMPLES=ON

# Build
cmake --build . --config Release

# Test
ctest --output-on-failure
```

## Integration

### CMake Integration
```cmake
# Add TinaKit to your project
add_subdirectory(path/to/tinakit)
target_link_libraries(your_target tinakit)
```

### Basic Usage
```cpp
#include "tinakit/tinakit.hpp"

int main() {
    auto workbook = tinakit::excel::Workbook::create();
    auto sheet = workbook.active_sheet();
    
    sheet["A1"].value("Hello, TinaKit!");
    workbook.save("hello.xlsx");
    
    return 0;
}
```

## Documentation

- See `docs/` directory for detailed documentation
- Check `examples/` for usage examples
- Read `README.md` for feature overview

## Support

- GitHub Issues: https://github.com/your-username/TinaKit/issues
- Documentation: See docs/ directory
""")
    
    print("   ✅ Created installation guide")
    
    # 5. Create version info file
    print("ℹ️ Creating version info...")
    version_info = release_dir / "VERSION.txt"
    with open(version_info, 'w', encoding='utf-8') as f:
        f.write("""TinaKit Version 1.0.0
Release Date: 2025-06-21
Build Type: Production Release

Features:
- Complete Excel (.xlsx) support
- Modern C++20 architecture
- High performance optimizations
- 100% test coverage
- Cross-platform compatibility

Git Tag: v1.0.0
Commit: [Latest commit hash]

For detailed changes, see CHANGELOG.md
""")
    
    print("   ✅ Created version info")
    
    # 6. Create ZIP package
    print("🗜️ Creating ZIP package...")
    zip_path = project_root / "release" / "tinakit-v1.0.0.zip"
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(release_dir):
            for file in files:
                file_path = Path(root) / file
                arc_path = file_path.relative_to(release_dir.parent)
                zipf.write(file_path, arc_path)
    
    print(f"   ✅ Created ZIP package: {zip_path}")
    
    # 7. Generate checksums
    print("🔐 Generating checksums...")
    import hashlib
    
    def get_file_hash(filepath, algorithm='sha256'):
        hash_obj = hashlib.new(algorithm)
        with open(filepath, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_obj.update(chunk)
        return hash_obj.hexdigest()
    
    checksums_file = project_root / "release" / "checksums.txt"
    with open(checksums_file, 'w') as f:
        sha256_hash = get_file_hash(zip_path, 'sha256')
        md5_hash = get_file_hash(zip_path, 'md5')
        
        f.write(f"TinaKit v1.0.0 Checksums\n")
        f.write(f"========================\n\n")
        f.write(f"File: tinakit-v1.0.0.zip\n")
        f.write(f"SHA256: {sha256_hash}\n")
        f.write(f"MD5:    {md5_hash}\n")
    
    print(f"   ✅ Generated checksums: {checksums_file}")
    
    # 8. Summary
    print("\n🎉 Release package created successfully!")
    print(f"📦 Package location: {zip_path}")
    print(f"📁 Source location: {release_dir}")
    print(f"🔐 Checksums: {checksums_file}")
    print(f"📊 Package size: {zip_path.stat().st_size / 1024 / 1024:.2f} MB")
    
    print("\n📋 Next steps:")
    print("1. Test the release package on different platforms")
    print("2. Upload to GitHub Releases")
    print("3. Update documentation links")
    print("4. Announce the release!")

if __name__ == "__main__":
    create_release_package()
