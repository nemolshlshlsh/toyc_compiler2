#include "semantic/analyzer.hpp"
#include <iostream>

bool SemanticAnalyzer::analyze(CompilationUnit& unit) {
    errors.clear();
    
    // 收集所有函数声明
    for (auto& func : unit.functions) {
        std::vector<Expression::Type> paramTypes;
        for (const auto& param : func->parameters) {
            paramTypes.push_back(param.type);
        }
        
        if (functions.find(func->name) != functions.end()) {
            addError("Function '" + func->name + "' is already declared");
            continue;
        }
        
        functions[func->name] = FunctionInfo(func->name, func->returnType, paramTypes, true);
    }
    
    // 检查main函数
    if (!checkMainFunction()) {
        addError("Missing main function with signature: int main()");
    }
    
    // 分析函数体
    unit.accept(*this);
    
    return errors.empty();
}

void SemanticAnalyzer::addError(const std::string& message) {
    errors.push_back(message);
}

bool SemanticAnalyzer::checkMainFunction() {
    auto it = functions.find("main");
    if (it == functions.end()) {
        return false;
    }
    
    const FunctionInfo& mainFunc = it->second;
    return mainFunc.returnType == Expression::INT && mainFunc.paramTypes.empty();
}

// Visitor实现
void SemanticAnalyzer::visit(CompilationUnit& node) {
    for (auto& func : node.functions) {
        func->accept(*this);
    }
}

void SemanticAnalyzer::visit(FunctionDefinition& node) {
    currentFunction = node.name;
    hasReturn = false;
    
    scope.enterScope();
    scope.resetOffset();
    
    // 添加参数到符号表
    for (const auto& param : node.parameters) {
        if (!scope.declareVariable(param.name, param.type, true)) {
            addError("Parameter '" + param.name + "' is already declared");
        }
    }
    
    // 分析函数体
    node.body->accept(*this);
    
    // 检查返回值
    if (node.returnType == Expression::INT && !hasReturn) {
        addError("Function '" + node.name + "' must return a value");
    }
    
    scope.exitScope();
}

void SemanticAnalyzer::visit(Block& node) {
    scope.enterScope();
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    scope.exitScope();
}

void SemanticAnalyzer::visit(VariableDeclaration& node) {
    if (!scope.declareVariable(node.name, Expression::INT)) {
        addError("Variable '" + node.name + "' is already declared in this scope");
        return;
    }
    
    if (node.initializer) {
        node.initializer->accept(*this);
    }
}

void SemanticAnalyzer::visit(AssignmentStatement& node) {
    Symbol* symbol = scope.lookupVariable(node.variable);
    if (!symbol) {
        addError("Undefined variable '" + node.variable + "'");
        return;
    }
    
    node.value->accept(*this);
}

void SemanticAnalyzer::visit(Identifier& node) {
    Symbol* symbol = scope.lookupVariable(node.name);
    if (!symbol) {
        addError("Undefined variable '" + node.name + "'");
    }
}

void SemanticAnalyzer::visit(FunctionCall& node) {
    auto it = functions.find(node.functionName);
    if (it == functions.end()) {
        addError("Undefined function '" + node.functionName + "'");
        return;
    }
    
    const FunctionInfo& funcInfo = it->second;
    
    // 检查参数数量
    if (node.arguments.size() != funcInfo.paramTypes.size()) {
        addError("Function '" + node.functionName + "' expects " + 
                std::to_string(funcInfo.paramTypes.size()) + " arguments, got " + 
                std::to_string(node.arguments.size()));
        return;
    }
    
    // 检查参数
    for (auto& arg : node.arguments) {
        arg->accept(*this);
    }
    
    // 设置返回类型
    const_cast<FunctionCall&>(node).returnType = funcInfo.returnType;
}

void SemanticAnalyzer::visit(BinaryExpression& node) {
    node.left->accept(*this);
    node.right->accept(*this);
}

void SemanticAnalyzer::visit(UnaryExpression& node) {
    node.operand->accept(*this);
}

void SemanticAnalyzer::visit(NumberLiteral& node) {
    // 数字字面量总是有效的
}

void SemanticAnalyzer::visit(IfStatement& node) {
    node.condition->accept(*this);
    node.thenStatement->accept(*this);
    if (node.elseStatement) {
        node.elseStatement->accept(*this);
    }
}

void SemanticAnalyzer::visit(WhileStatement& node) {
    node.condition->accept(*this);
    loopDepth++;
    node.body->accept(*this);
    loopDepth--;
}

void SemanticAnalyzer::visit(BreakStatement& node) {
    if (loopDepth == 0) {
        addError("break statement not within a loop");
    }
}

void SemanticAnalyzer::visit(ContinueStatement& node) {
    if (loopDepth == 0) {
        addError("continue statement not within a loop");
    }
}

void SemanticAnalyzer::visit(ReturnStatement& node) {
    hasReturn = true;
    
    auto it = functions.find(currentFunction);
    if (it != functions.end()) {
        const FunctionInfo& funcInfo = it->second;
        
        if (funcInfo.returnType == Expression::VOID && node.value) {
            addError("void function should not return a value");
        } else if (funcInfo.returnType == Expression::INT && !node.value) {
            addError("non-void function must return a value");
        }
    }
    
    if (node.value) {
        node.value->accept(*this);
    }
}

void SemanticAnalyzer::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
}