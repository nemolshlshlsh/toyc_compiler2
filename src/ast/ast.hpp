#pragma once
#include <memory>
#include <vector>
#include <string>
#include <iostream>

// 前向声明
class Visitor;

// AST节点基类
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(Visitor& visitor) = 0;
    virtual void print(int indent = 0) const = 0;
};

// 表达式基类
class Expression : public ASTNode {
public:
    enum Type { INT, VOID };
    virtual Type getType() const = 0;
};

// 语句基类
class Statement : public ASTNode {};

// 二元表达式
class BinaryExpression : public Expression {
public:
    enum Operator { 
        ADD, SUB, MUL, DIV, MOD,
        LT, LE, GT, GE, EQ, NE,
        AND, OR
    };
    
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    Operator op;
    
    BinaryExpression(std::unique_ptr<Expression> l, Operator o, std::unique_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)), op(o) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
    Type getType() const override { return INT; }
};

// 一元表达式
class UnaryExpression : public Expression {
public:
    enum Operator { PLUS, MINUS, NOT };
    
    Operator op;
    std::unique_ptr<Expression> operand;
    
    UnaryExpression(Operator o, std::unique_ptr<Expression> expr)
        : op(o), operand(std::move(expr)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
    Type getType() const override { return INT; }
};

// 数字字面量
class NumberLiteral : public Expression {
public:
    int value;
    
    NumberLiteral(int val) : value(val) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
    Type getType() const override { return INT; }
};

// 标识符表达式
class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n) : name(n) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
    Type getType() const override { return INT; }
};

// 函数调用表达式
class FunctionCall : public Expression {
public:
    std::string functionName;
    std::vector<std::unique_ptr<Expression>> arguments;
    Expression::Type returnType;
    
    FunctionCall(const std::string& name, std::vector<std::unique_ptr<Expression>> args, Expression::Type type)
        : functionName(name), arguments(std::move(args)), returnType(type) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
    Type getType() const override { return returnType; }
};

// 赋值语句
class AssignmentStatement : public Statement {
public:
    std::string variable;
    std::unique_ptr<Expression> value;
    
    AssignmentStatement(const std::string& var, std::unique_ptr<Expression> val)
        : variable(var), value(std::move(val)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// 变量声明语句
class VariableDeclaration : public Statement {
public:
    std::string name;
    std::unique_ptr<Expression> initializer;
    
    VariableDeclaration(const std::string& n, std::unique_ptr<Expression> init)
        : name(n), initializer(std::move(init)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// 语句块
class Block : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    
    void addStatement(std::unique_ptr<Statement> stmt) {
        statements.push_back(std::move(stmt));
    }
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// If语句
class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenStatement;
    std::unique_ptr<Statement> elseStatement; // 可选
    
    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> then, 
                std::unique_ptr<Statement> els = nullptr)
        : condition(std::move(cond)), thenStatement(std::move(then)), elseStatement(std::move(els)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// While语句
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    
    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// Break语句
class BreakStatement : public Statement {
public:
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// Continue语句
class ContinueStatement : public Statement {
public:
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// Return语句
class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value; // 可选
    
    ReturnStatement(std::unique_ptr<Expression> val = nullptr) : value(std::move(val)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// 表达式语句
class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    
    ExpressionStatement(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// 参数定义
class Parameter {
public:
    std::string name;
    Expression::Type type;
    
    Parameter(const std::string& n, Expression::Type t) : name(n), type(t) {}
};

// 函数定义
class FunctionDefinition : public ASTNode {
public:
    std::string name;
    Expression::Type returnType;
    std::vector<Parameter> parameters;
    std::unique_ptr<Block> body;
    
    FunctionDefinition(const std::string& n, Expression::Type ret, 
                      std::vector<Parameter> params, std::unique_ptr<Block> b)
        : name(n), returnType(ret), parameters(std::move(params)), body(std::move(b)) {}
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// 编译单元（程序根节点）
class CompilationUnit : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionDefinition>> functions;
    
    void addFunction(std::unique_ptr<FunctionDefinition> func) {
        functions.push_back(std::move(func));
    }
    
    void accept(Visitor& visitor) override;
    void print(int indent = 0) const override;
};

// 访问者模式接口
class Visitor {
public:
    virtual ~Visitor() = default;
    
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(NumberLiteral& node) = 0;
    virtual void visit(Identifier& node) = 0;
    virtual void visit(FunctionCall& node) = 0;
    virtual void visit(AssignmentStatement& node) = 0;
    virtual void visit(VariableDeclaration& node) = 0;
    virtual void visit(Block& node) = 0;
    virtual void visit(IfStatement& node) = 0;
    virtual void visit(WhileStatement& node) = 0;
    virtual void visit(BreakStatement& node) = 0;
    virtual void visit(ContinueStatement& node) = 0;
    virtual void visit(ReturnStatement& node) = 0;
    virtual void visit(ExpressionStatement& node) = 0;
    virtual void visit(FunctionDefinition& node) = 0;
    virtual void visit(CompilationUnit& node) = 0;
};