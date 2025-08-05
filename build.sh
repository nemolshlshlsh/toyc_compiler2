#!/bin/bash

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Building ToyC Compiler...${NC}"

# 检查依赖
echo "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake not found${NC}"
    echo "Please install CMake: sudo apt-get install cmake"
    exit 1
fi

if ! command -v flex &> /dev/null; then
    echo -e "${RED}Error: Flex not found${NC}"
    echo "Please install Flex: sudo apt-get install flex"
    exit 1
fi

if ! command -v bison &> /dev/null; then
    echo -e "${RED}Error: Bison not found${NC}"
    echo "Please install Bison: sudo apt-get install bison"
    exit 1
fi

echo -e "${GREEN}All dependencies found${NC}"

# 显示版本信息
echo ""
echo "Dependency versions:"
echo "  CMake: $(cmake --version | head -n1 | cut -d' ' -f3)"
echo "  Flex: $(flex --version | cut -d' ' -f2)"
echo "  Bison: $(bison --version | head -n1 | cut -d' ' -f4)"

# 清理并创建构建目录
echo ""
echo "Setting up build directory..."
rm -rf build
mkdir build
cd build

# 配置
echo "Configuring..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 构建
echo "Building..."
make -j$(nproc) toyc

echo -e "${GREEN}Build completed successfully!${NC}"
echo ""
echo "Executable: build/toyc"
echo ""

# 运行测试
if [ "$1" != "--no-test" ]; then
    echo -e "${BLUE}Running tests...${NC}"
    cd ..
    chmod +x run_tests.sh
    ./run_tests.sh build/toyc
else
    echo "Skipping tests (--no-test specified)"
fi

echo ""
echo -e "${GREEN}Ready to use!${NC}"
echo ""
echo "Usage examples:"
echo "  ./build/toyc test_samples/hello.tc"
echo "  ./build/toyc --ast test_samples/factorial.tc"
echo "  ./build/toyc -v test_samples/fibonacci.tc -o fib.s"