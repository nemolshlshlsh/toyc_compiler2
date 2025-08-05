toyc-compiler/
├── CMakeLists.txt          # CMake 构建配置
├── src/                    # 主要源代码
│   ├── main.cpp            # 编译器主入口
│   ├── lexer.l             # Flex 词法分析规则
│   ├── parser.y            # Bison 语法分析规则
│   ├── ast/                # AST 相关代码
│   │   ├── ast.hpp         # AST 节点定义
│   │   ├── ast.cpp         # AST 节点实现
│   ├── semantic/           # 语义分析
│   │   ├── analyzer.hpp    
│   │   ├── analyzer.cpp    
│   ├── codegen/            # 代码生成（RISC-V）
│   │   ├── riscv.hpp       
│   │   ├── riscv.cpp       
│   ├── utils/              # 工具函数
│   │   ├── utils.hpp       
│   │   ├── utils.cpp       
├── tests/                  # 测试用例
│   ├── test_lexer.cpp      # 词法分析测试
│   ├── test_parser.cpp     # 语法分析测试
│   ├── test_semantic.cpp   # 语义分析测试
│   ├── test_codegen.cpp    # 代码生成测试
│   ├── samples/            # 示例 ToyC 代码
│   │   ├── hello.tc        
│   │   ├── factorial.tc   
├── build/                  # 构建目录（CMake 生成）