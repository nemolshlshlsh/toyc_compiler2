#!/bin/bash

COMPILER=${1:-"./build/compiler"}
TEST_DIR="test_samples"
TEMP_DIR="/tmp/toyc_test_$$"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

if [ ! -x "$COMPILER" ]; then
    echo -e "${RED}Error: Compiler not found or not executable: $COMPILER${NC}"
    exit 1
fi

mkdir -p "$TEMP_DIR"

echo -e "${BLUE}ToyC Compiler Test Suite (stdin/stdout interface)${NC}"
echo "=================================================="
echo "Compiler: $COMPILER"
echo "Test directory: $TEST_DIR"
echo ""

total_tests=0
passed_tests=0

# 测试函数
run_test() {
    local test_file=$1
    local test_name=$(basename "$test_file" .tc)
    local use_opt=$2
    
    echo -n "Testing $test_name"
    if [ "$use_opt" = "true" ]; then
        echo -n " (optimized)"
    fi
    echo -n "... "
    
    total_tests=$((total_tests + 1))
    
    # 使用stdin/stdout接口测试
    local output_file="$TEMP_DIR/$test_name.s"
    local opt_flag=""
    if [ "$use_opt" = "true" ]; then
        opt_flag="-opt"
        output_file="$TEMP_DIR/${test_name}_opt.s"
    fi
    
    if "$COMPILER" $opt_flag < "$test_file" > "$output_file" 2>"$TEMP_DIR/$test_name.err"; then
        if [ -f "$output_file" ] && [ -s "$output_file" ]; then
            echo -e "${GREEN}PASS${NC}"
            passed_tests=$((passed_tests + 1))
            
            # 显示生成的汇编代码行数
            local line_count=$(wc -l < "$output_file")
            echo "  Generated $line_count lines of assembly"
        else
            echo -e "${RED}FAIL${NC} (empty output)"
        fi
    else
        echo -e "${RED}FAIL${NC}"
        if [ -f "$TEMP_DIR/$test_name.err" ]; then
            echo "  Error output:"
            sed 's/^/    /' "$TEMP_DIR/$test_name.err"
        fi
    fi
}

# 运行编译测试
echo "Running compilation tests (normal mode):"
for test_file in "$TEST_DIR"/*.tc; do
    if [ -f "$test_file" ]; then
        run_test "$test_file" false
    fi
done

echo ""
echo "Running compilation tests (optimized mode):"
for test_file in "$TEST_DIR"/*.tc; do
    if [ -f "$test_file" ]; then
        run_test "$test_file" true
    fi
done

# 测试错误处理
echo ""
echo "Testing error handling:"
cat > "$TEMP_DIR/syntax_error.tc" << 'EOF'
int main() {
    int x = ;  // 语法错误
    return x;
}
EOF

echo -n "Testing syntax error detection... "
if "$COMPILER" < "$TEMP_DIR/syntax_error.tc" > /dev/null 2>&1; then
    echo -e "${RED}FAIL${NC} (should have failed)"
else
    echo -e "${GREEN}PASS${NC}"
    passed_tests=$((passed_tests + 1))
fi
total_tests=$((total_tests + 1))

echo ""
echo "=================================================="
echo -e "Tests completed: ${GREEN}$passed_tests${NC}/${total_tests} passed"

if [ $passed_tests -eq $total_tests ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    rm -rf "$TEMP_DIR"
    exit 0
else
    echo -e "${RED}$((total_tests - passed_tests)) tests failed${NC}"
    echo "Generated files are in: $TEMP_DIR"
    exit 1
fi