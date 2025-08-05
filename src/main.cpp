#include <iostream>
#include <memory>
#include <sstream>
#include "ast/ast.hpp"
#include "common/types.hpp"
#include "semantic/analyzer.hpp"
#include "codegen/riscv.hpp"
#include "utils/utils.hpp"

// 外部函数声明（由flex/bison生成）
extern FILE* yyin;
extern int yyparse();
extern std::unique_ptr<CompilationUnit> root;
extern int yylineno;

void printUsage(const char* programName) {
	std::cerr << "ToyC Compiler v1.0\n"
	<< "Usage: " << programName << " [-opt]\n\n"
	<< "Options:\n"
	<< "  -opt         Enable optimizations\n"
	<< "\n"
	<< "Input: Read from stdin\n"
	<< "Output: Write to stdout\n"
	<< "Errors: Write to stderr\n\n"
	<< "Example: " << programName << " < input.tc > output.s\n";
}

int main(int argc, char* argv[]) {
	bool enableOptimizations = false;
	
	// 解析命令行参数
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		
		if (arg == "-opt") {
			enableOptimizations = true;
		} else if (arg == "--help" || arg == "-h") {
			printUsage(argv[0]);
			return 0;
		} else {
			std::cerr << "Error: Unknown option: " << arg << std::endl;
			printUsage(argv[0]);
			return 1;
		}
	}
	
	try {
		// 调试信息输出到stderr
		if (enableOptimizations) {
			std::cerr << "[INFO] Optimizations enabled" << std::endl;
		}
		
		// 1. 从stdin读取输入并解析
		std::cerr << "[INFO] Reading from stdin..." << std::endl;
		
		// 设置输入为stdin
		yyin = stdin;
		yylineno = 1;
		root.reset();
		
		// 语法分析
		int parseResult = yyparse();
		
		if (parseResult != 0) {
			std::cerr << "Error: Parsing failed" << std::endl;
			return 1;
		}
		
		if (!root) {
			std::cerr << "Error: No AST generated" << std::endl;
			return 1;
		}
		
		std::cerr << "[INFO] Parsing completed successfully" << std::endl;
		
		// 2. 语义分析
		std::cerr << "[INFO] Performing semantic analysis..." << std::endl;
		
		SemanticAnalyzer analyzer;
		if (!analyzer.analyze(*root)) {
			std::cerr << "Semantic analysis failed:" << std::endl;
			const auto& errors = analyzer.getErrors();
			for (size_t i = 0; i < errors.size(); ++i) {
				std::cerr << "  Error " << (i+1) << ": " << errors[i] << std::endl;
			}
			return 1;
		}
		
		std::cerr << "[INFO] Semantic analysis completed successfully" << std::endl;
		
		// 3. 代码生成
		std::cerr << "[INFO] Generating code..." << std::endl;
		
		RISCVCodeGenerator generator;
		
		// 如果启用优化，设置优化标志
		if (enableOptimizations) {
			generator.enableOptimizations();
		}
		
		// 构建函数表
		std::unordered_map<std::string, FunctionInfo> functionTable;
		for (const auto& func : root->functions) {
			std::vector<Expression::Type> paramTypes;
			for (const auto& param : func->parameters) {
				paramTypes.push_back(param.type);
			}
			functionTable[func->name] = FunctionInfo(func->name, func->returnType, paramTypes, true);
		}
		
		std::string assemblyCode = generator.generate(*root, functionTable);
		
		std::cerr << "[INFO] Code generation completed" << std::endl;
		
		// 4. 输出到stdout
		std::cout << assemblyCode;
		
		// 确保输出被刷新
		std::cout.flush();
		
		std::cerr << "[INFO] Compilation successful!" << std::endl;
		return 0;
		
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Error: Unknown error occurred" << std::endl;
		return 1;
	}
}
