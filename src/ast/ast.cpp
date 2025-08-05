#include "ast/ast.hpp"
#include <iostream>
#include <iomanip>

void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

// BinaryExpression
void BinaryExpression::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void BinaryExpression::print(int indent) const {
    printIndent(indent);
    std::cout << "BinaryExpression: ";
    switch (op) {
        case ADD: std::cout << "+"; break;
        case SUB: std::cout << "-"; break;
        case MUL: std::cout << "*"; break;
        case DIV: std::cout << "/"; break;
        case MOD: std::cout << "%"; break;
        case LT: std::cout << "<"; break;
        case LE: std::cout << "<="; break;
        case GT: std::cout << ">"; break;
        case GE: std::cout << ">="; break;
        case EQ: std::cout << "=="; break;
        case NE: std::cout << "!="; break;
        case AND: std::cout << "&&"; break;
        case OR: std::cout << "||"; break;
    }
    std::cout << std::endl;
    left->print(indent + 1);
    right->print(indent + 1);
}

// UnaryExpression
void UnaryExpression::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void UnaryExpression::print(int indent) const {
    printIndent(indent);
    std::cout << "UnaryExpression: ";
    switch (op) {
        case PLUS: std::cout << "+"; break;
        case MINUS: std::cout << "-"; break;
        case NOT: std::cout << "!"; break;
    }
    std::cout << std::endl;
    operand->print(indent + 1);
}

// NumberLiteral
void NumberLiteral::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void NumberLiteral::print(int indent) const {
    printIndent(indent);
    std::cout << "NumberLiteral: " << value << std::endl;
}

// Identifier
void Identifier::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void Identifier::print(int indent) const {
    printIndent(indent);
    std::cout << "Identifier: " << name << std::endl;
}

// FunctionCall
void FunctionCall::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void FunctionCall::print(int indent) const {
    printIndent(indent);
    std::cout << "FunctionCall: " << functionName << std::endl;
    for (const auto& arg : arguments) {
        arg->print(indent + 1);
    }
}

// AssignmentStatement
void AssignmentStatement::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void AssignmentStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "Assignment: " << variable << std::endl;
    value->print(indent + 1);
}

// VariableDeclaration
void VariableDeclaration::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void VariableDeclaration::print(int indent) const {
    printIndent(indent);
    std::cout << "VariableDeclaration: " << name << std::endl;
    if (initializer) {
        initializer->print(indent + 1);
    }
}

// Block
void Block::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void Block::print(int indent) const {
    printIndent(indent);
    std::cout << "Block:" << std::endl;
    for (const auto& stmt : statements) {
        stmt->print(indent + 1);
    }
}

// IfStatement
void IfStatement::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void IfStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "IfStatement:" << std::endl;
    printIndent(indent + 1);
    std::cout << "Condition:" << std::endl;
    condition->print(indent + 2);
    printIndent(indent + 1);
    std::cout << "Then:" << std::endl;
    thenStatement->print(indent + 2);
    if (elseStatement) {
        printIndent(indent + 1);
        std::cout << "Else:" << std::endl;
        elseStatement->print(indent + 2);
    }
}

// WhileStatement
void WhileStatement::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void WhileStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "WhileStatement:" << std::endl;
    printIndent(indent + 1);
    std::cout << "Condition:" << std::endl;
    condition->print(indent + 2);
    printIndent(indent + 1);
    std::cout << "Body:" << std::endl;
    body->print(indent + 2);
}

// BreakStatement
void BreakStatement::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void BreakStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "BreakStatement" << std::endl;
}

// ContinueStatement
void ContinueStatement::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void ContinueStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "ContinueStatement" << std::endl;
}

// ReturnStatement
void ReturnStatement::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void ReturnStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "ReturnStatement:" << std::endl;
    if (value) {
        value->print(indent + 1);
    }
}

// ExpressionStatement
void ExpressionStatement::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void ExpressionStatement::print(int indent) const {
    printIndent(indent);
    std::cout << "ExpressionStatement:" << std::endl;
    expression->print(indent + 1);
}

// FunctionDefinition
void FunctionDefinition::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void FunctionDefinition::print(int indent) const {
    printIndent(indent);
    std::cout << "FunctionDefinition: " << name;
    std::cout << " (" << (returnType == Expression::INT ? "int" : "void") << ")" << std::endl;
    for (const auto& param : parameters) {
        printIndent(indent + 1);
        std::cout << "Parameter: " << param.name << " (int)" << std::endl;
    }
    body->print(indent + 1);
}

// CompilationUnit
void CompilationUnit::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void CompilationUnit::print(int indent) const {
    printIndent(indent);
    std::cout << "CompilationUnit:" << std::endl;
    for (const auto& func : functions) {
        func->print(indent + 1);
    }
}