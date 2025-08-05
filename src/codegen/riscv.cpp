#include "codegen/riscv.hpp"
#include <iostream>
#include <sstream>

// 在RISCVCodeGenerator类中添加这些方法

void RISCVCodeGenerator::optimizeConstantFolding(BinaryExpression& node) {
    if (!optimizationsEnabled) return;
    
    // 检查是否可以进行常量折叠
    if (isConstantExpression(node.left.get()) && isConstantExpression(node.right.get())) {
        int leftVal = evaluateConstantExpression(node.left.get());
        int rightVal = evaluateConstantExpression(node.right.get());
        int result = 0;
        
        switch (node.op) {
            case BinaryExpression::ADD: result = leftVal + rightVal; break;
            case BinaryExpression::SUB: result = leftVal - rightVal; break;
            case BinaryExpression::MUL: result = leftVal * rightVal; break;
            case BinaryExpression::DIV: 
                if (rightVal != 0) result = leftVal / rightVal; 
                else return; // 避免除零
                break;
            case BinaryExpression::MOD: 
                if (rightVal != 0) result = leftVal % rightVal; 
                else return;
                break;
            default: return; // 其他操作不优化
        }
        
        // 直接加载常量结果
        emit("li t0, " + std::to_string(result));
        emit("addi sp, sp, -4");
        emit("sw t0, 0(sp)");
        return;
    }
    
    // 如果不能完全折叠，检查是否有简单优化
    if (isConstantExpression(node.right.get())) {
        int rightVal = evaluateConstantExpression(node.right.get());
        
        // 优化加0、乘1等
        if (node.op == BinaryExpression::ADD && rightVal == 0) {
            node.left->accept(*this);
            return;
        }
        if (node.op == BinaryExpression::MUL && rightVal == 1) {
            node.left->accept(*this);
            return;
        }
        if (node.op == BinaryExpression::MUL && rightVal == 0) {
            emit("li t0, 0");
            emit("addi sp, sp, -4");
            emit("sw t0, 0(sp)");
            return;
        }
    }
}

bool RISCVCodeGenerator::isConstantExpression(Expression* expr) {
    if (!expr) return false;
    
    // 检查是否是数字字面量
    if (dynamic_cast<NumberLiteral*>(expr)) {
        return true;
    }
    
    // 检查是否是已知常量变量
    if (auto ident = dynamic_cast<Identifier*>(expr)) {
        return constantValues.find(ident->name) != constantValues.end();
    }
    
    return false;
}

int RISCVCodeGenerator::evaluateConstantExpression(Expression* expr) {
    if (auto numLit = dynamic_cast<NumberLiteral*>(expr)) {
        return numLit->value;
    }
    
    if (auto ident = dynamic_cast<Identifier*>(expr)) {
        auto it = constantValues.find(ident->name);
        if (it != constantValues.end()) {
            return it->second;
        }
    }
    
    return 0;
}

void RISCVCodeGenerator::visit(BinaryExpression& node) {
    // 尝试优化
    if (optimizationsEnabled) {
        optimizeConstantFolding(node);
        return;
    }
    
    // 原有的代码生成逻辑
    node.left->accept(*this);
    node.right->accept(*this);
    
    emit("lw t1, 0(sp)");
    emit("addi sp, sp, 4");
    emit("lw t0, 0(sp)");
    
    switch (node.op) {
        case BinaryExpression::ADD:
            emit("add t0, t0, t1");
            break;
        case BinaryExpression::SUB:
            emit("sub t0, t0, t1");
            break;
        case BinaryExpression::MUL:
            emit("mul t0, t0, t1");
            break;
        case BinaryExpression::DIV:
            emit("div t0, t0, t1");
            break;
        case BinaryExpression::MOD:
            emit("rem t0, t0, t1");
            break;
        case BinaryExpression::LT:
            emit("slt t0, t0, t1");
            break;
        case BinaryExpression::LE:
            emit("slt t2, t1, t0");
            emit("xori t0, t2, 1");
            break;
        case BinaryExpression::GT:
            emit("slt t0, t1, t0");
            break;
        case BinaryExpression::GE:
            emit("slt t2, t0, t1");
            emit("xori t0, t2, 1");
            break;
        case BinaryExpression::EQ:
            emit("sub t0, t0, t1");
            emit("seqz t0, t0");
            break;
        case BinaryExpression::NE:
            emit("sub t0, t0, t1");
            emit("snez t0, t0");
            break;
        case BinaryExpression::AND:
            emit("and t0, t0, t1");
            break;
        case BinaryExpression::OR:
            emit("or t0, t0, t1");
            break;
    }
    
    emit("sw t0, 0(sp)");
}

// 添加所有缺失的方法实现
std::string RISCVCodeGenerator::generate(CompilationUnit& unit, const std::unordered_map<std::string, FunctionInfo>& funcTable) {
    output.clear();
    functions = funcTable;
    stackOffset = 0;
    labelCounter = 0;
    
    // 生成数据段
    emit(".data");
    emit(".text");
    emit(".global main");
    
    // 访问编译单元
    unit.accept(*this);
    
    return output;
}

void RISCVCodeGenerator::emit(const std::string& instruction) {
    output += instruction + "\n";
}

void RISCVCodeGenerator::emitLabel(const std::string& label) {
    output += label + ":\n";
}

std::string RISCVCodeGenerator::newLabel(const std::string& prefix) {
    return prefix + std::to_string(labelCounter++);
}

void RISCVCodeGenerator::generatePrologue(const std::string& funcName, int localSize) {
    emitLabel(funcName);
    emit("addi sp, sp, -" + std::to_string(localSize));
    emit("sw ra, " + std::to_string(localSize - 4) + "(sp)");
    emit("sw fp, " + std::to_string(localSize - 8) + "(sp)");
    emit("addi fp, sp, " + std::to_string(localSize));
}

void RISCVCodeGenerator::generateEpilogue() {
    emit("lw ra, -4(fp)");
    emit("lw fp, -8(fp)");
    emit("addi sp, sp, 8");
    emit("ret");
}

void RISCVCodeGenerator::optimizeDeadCodeElimination() {
    // 简单的死代码消除实现
    for (const auto& code : deadCode) {
        (void)code; // 避免未使用变量警告
        // 这里可以实现更复杂的死代码消除逻辑
    }
}

// Visitor 方法实现
void RISCVCodeGenerator::visit(UnaryExpression& node) {
    node.operand->accept(*this);
    emit("lw t0, 0(sp)");
    
    switch (node.op) {
        case UnaryExpression::PLUS:
            // 正号不需要操作
            break;
        case UnaryExpression::MINUS:
            emit("neg t0, t0");
            break;
        case UnaryExpression::NOT:
            emit("seqz t0, t0");
            break;
    }
    
    emit("sw t0, 0(sp)");
}

void RISCVCodeGenerator::visit(NumberLiteral& node) {
    emit("li t0, " + std::to_string(node.value));
    emit("addi sp, sp, -4");
    emit("sw t0, 0(sp)");
}

void RISCVCodeGenerator::visit(Identifier& node) {
    // 查找变量在栈中的位置
    auto it = localVariables.find(node.name);
    if (it != localVariables.end()) {
        emit("lw t0, " + std::to_string(it->second) + "(fp)");
        emit("addi sp, sp, -4");
        emit("sw t0, 0(sp)");
    } else {
        // 全局变量或未定义变量
        emit("la t0, " + node.name);
        emit("lw t0, 0(t0)");
        emit("addi sp, sp, -4");
        emit("sw t0, 0(sp)");
    }
}

void RISCVCodeGenerator::visit(FunctionCall& node) {
    // 生成函数调用
    for (const auto& arg : node.arguments) {
        arg->accept(*this);
    }
    
    emit("call " + node.functionName);
    emit("addi sp, sp, " + std::to_string(node.arguments.size() * 4));
    emit("addi sp, sp, -4");
    emit("sw a0, 0(sp)");
}

void RISCVCodeGenerator::visit(AssignmentStatement& node) {
    node.value->accept(*this);
    emit("lw t0, 0(sp)");
    emit("addi sp, sp, 4");
    
    auto it = localVariables.find(node.variable);
    if (it != localVariables.end()) {
        emit("sw t0, " + std::to_string(it->second) + "(fp)");
    } else {
        emit("la t1, " + node.variable);
        emit("sw t0, 0(t1)");
    }
}

void RISCVCodeGenerator::visit(VariableDeclaration& node) {
    if (node.initializer) {
        node.initializer->accept(*this);
        emit("lw t0, 0(sp)");
        emit("addi sp, sp, 4");
    } else {
        emit("li t0, 0");
    }
    
    stackOffset -= 4;
    localVariables[node.name] = stackOffset;
    emit("sw t0, " + std::to_string(stackOffset) + "(fp)");
}

void RISCVCodeGenerator::visit(Block& node) {
    for (const auto& stmt : node.statements) {
        stmt->accept(*this);
    }
}

void RISCVCodeGenerator::visit(IfStatement& node) {
    std::string elseLabel = newLabel("else");
    std::string endLabel = newLabel("endif");
    
    node.condition->accept(*this);
    emit("lw t0, 0(sp)");
    emit("addi sp, sp, 4");
    emit("beqz t0, " + elseLabel);
    
    node.thenStatement->accept(*this);
    emit("j " + endLabel);
    
    emitLabel(elseLabel);
    if (node.elseStatement) {
        node.elseStatement->accept(*this);
    }
    
    emitLabel(endLabel);
}

void RISCVCodeGenerator::visit(WhileStatement& node) {
    std::string loopLabel = newLabel("loop");
    std::string endLabel = newLabel("endloop");
    
    emitLabel(loopLabel);
    node.condition->accept(*this);
    emit("lw t0, 0(sp)");
    emit("addi sp, sp, 4");
    emit("beqz t0, " + endLabel);
    
    node.body->accept(*this);
    emit("j " + loopLabel);
    
    emitLabel(endLabel);
}

void RISCVCodeGenerator::visit(BreakStatement& node) {
    (void)node; // 避免未使用参数警告
    // 简单的 break 实现，需要更复杂的标签管理
    emit("j break_label"); // 这里需要实际的 break 标签
}

void RISCVCodeGenerator::visit(ContinueStatement& node) {
    (void)node; // 避免未使用参数警告
    // 简单的 continue 实现，需要更复杂的标签管理
    emit("j continue_label"); // 这里需要实际的 continue 标签
}

void RISCVCodeGenerator::visit(ReturnStatement& node) {
    if (node.value) {
        node.value->accept(*this);
        emit("lw a0, 0(sp)");
        emit("addi sp, sp, 4");
    } else {
        emit("li a0, 0");
    }
    
    generateEpilogue();
}

void RISCVCodeGenerator::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
    emit("addi sp, sp, 4"); // 弹出表达式结果
}

void RISCVCodeGenerator::visit(FunctionDefinition& node) {
    currentFunction = node.name;
    localVariables.clear();
    stackOffset = 0;
    
    // 计算局部变量空间
    int localSize = 8; // ra, fp
    
    generatePrologue(node.name, localSize);
    node.body->accept(*this);
    generateEpilogue();
}

void RISCVCodeGenerator::visit(CompilationUnit& node) {
    for (const auto& func : node.functions) {
        func->accept(*this);
    }
}