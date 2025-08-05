#pragma once
#include "ast/ast.hpp"
#include <string>
#include <vector>

// 函数信息结构体
struct FunctionInfo {
    std::string name;
    Expression::Type returnType;
    std::vector<Expression::Type> paramTypes;
    bool isDefined;
    
    // 默认构造函数（解决 unordered_map operator[] 的问题）
    FunctionInfo() : name(""), returnType(Expression::INT), isDefined(false) {}
    
    // 带参数的构造函数
    FunctionInfo(const std::string& n, Expression::Type ret, 
                const std::vector<Expression::Type>& params, bool def = false)
        : name(n), returnType(ret), paramTypes(params), isDefined(def) {}
}; 