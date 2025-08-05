#pragma once
#include "ast/ast.hpp"
#include "common/types.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class RISCVCodeGenerator : public Visitor {
private:
    std::string output;
    std::unordered_map<std::string, int> localVariables;
    std::unordered_map<std::string, FunctionInfo> functions;
    int stackOffset;
    int labelCounter;
    std::string currentFunction;
    
    // 优化相关
    bool optimizationsEnabled;
    std::unordered_map<std::string, int> constantValues; // 常量传播
    std::vector<std::string> deadCode; // 死代码消除
    
public:
    RISCVCodeGenerator() : stackOffset(0), labelCounter(0), optimizationsEnabled(false) {}
    
    std::string generate(CompilationUnit& unit, const std::unordered_map<std::string, FunctionInfo>& funcTable);
    
    // 启用优化
    void enableOptimizations() { optimizationsEnabled = true; }
    
    // Visitor接口
    void visit(BinaryExpression& node) override;
    void visit(UnaryExpression& node) override;
    void visit(NumberLiteral& node) override;
    void visit(Identifier& node) override;
    void visit(FunctionCall& node) override;
    void visit(AssignmentStatement& node) override;
    void visit(VariableDeclaration& node) override;
    void visit(Block& node) override;
    void visit(IfStatement& node) override;
    void visit(WhileStatement& node) override;
    void visit(BreakStatement& node) override;
    void visit(ContinueStatement& node) override;
    void visit(ReturnStatement& node) override;
    void visit(ExpressionStatement& node) override;
    void visit(FunctionDefinition& node) override;
    void visit(CompilationUnit& node) override;
    
private:
    void emit(const std::string& instruction);
    void emitLabel(const std::string& label);
    std::string newLabel(const std::string& prefix = "L");
    void generatePrologue(const std::string& funcName, int localSize);
    void generateEpilogue();
    
    // 优化相关方法
    void optimizeConstantFolding(BinaryExpression& node);
    void optimizeDeadCodeElimination();
    bool isConstantExpression(Expression* expr);
    int evaluateConstantExpression(Expression* expr);
};