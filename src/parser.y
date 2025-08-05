%{
#include "ast/ast.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>

extern int yylex();
extern int yylineno;
extern char* yytext;
void yyerror(const char* s);

std::unique_ptr<CompilationUnit> root;

#define SAFE_FREE(p) { if (p) { free(p); p = NULL; } }
%}

%union {
    int int_val;
    char* str_val;
    Expression* expr;
    Statement* stmt;
    Block* block;
    FunctionDefinition* func_def;
    CompilationUnit* comp_unit;
    std::vector<Parameter>* param_list;
    std::vector<std::unique_ptr<Expression>>* expr_list;
    Parameter* param;
    Expression::Type type_val;
    BinaryExpression::Operator bin_op;
    UnaryExpression::Operator un_op;
}

%token <int_val> NUMBER_LITERAL
%token <str_val> ID
%token INT VOID IF ELSE WHILE BREAK CONTINUE RETURN
%token PLUS MINUS MULTIPLY DIVIDE MOD ASSIGN
%token EQ NE LT LE GT GE AND OR NOT
%token LPAREN RPAREN LBRACE RBRACE COMMA SEMICOLON
%token ERROR

%type <comp_unit> CompUnit
%type <func_def> FuncDef
%type <param_list> ParamList
%type <param> Param
%type <block> Block
%type <stmt> Stmt
%type <expr> Expr LOrExpr LAndExpr RelExpr AddExpr MulExpr UnaryExpr PrimaryExpr
%type <expr_list> ExprList
%type <type_val> Type
%type <bin_op> RelOp AddOp MulOp
%type <un_op> UnaryOp

%nonassoc NO_ELSE
%nonassoc ELSE

%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right NOT UMINUS UPLUS

%%

CompUnit: 
    FuncDef {
        root = std::make_unique<CompilationUnit>();
        root->addFunction(std::unique_ptr<FunctionDefinition>($1));
        $$
 = root.get();
    }
    | CompUnit FuncDef {
        $1->addFunction(std::unique_ptr<FunctionDefinition>($2));
        $$ = $1;
    };

FuncDef: 
    Type ID LPAREN ParamList RPAREN Block {
        auto params = $4 ? *$4 : std::vector<Parameter>();
        $$
 = new FunctionDefinition($2, $1, std::move(params), std::unique_ptr<Block>($6));
        SAFE_FREE($2);
        delete $4;
    }
    | Type ID LPAREN RPAREN Block {
        $$ = new FunctionDefinition($2, $1, std::vector<Parameter>(), std::unique_ptr<Block>($5));
        SAFE_FREE($2);
    };

Type: 
    INT { $$ = Expression::INT; }
    | VOID { $$ = Expression::VOID; };

ParamList: 
    Param {
        $$
 = new std::vector<Parameter>();
        $$->push_back(*$1);
        delete $1;
    }
    | ParamList COMMA Param {
        $1->push_back(*$3);
        delete $3;
        $$
 = $1;
    };

Param: 
    INT ID {
        $$ = new Parameter($2, Expression::INT);
        SAFE_FREE($2);
    };

Block: 
    LBRACE RBRACE {
        $$
 = new Block();
    }
    | LBRACE BlockItems RBRACE {
        $$ = dynamic_cast<Block*>($<stmt>2);
    };

BlockItems: 
    Stmt {
        auto block = new Block();
        block->addStatement(std::unique_ptr<Statement>($1));
        $<stmt>$ = block;
    }
    | BlockItems Stmt {
        auto block = dynamic_cast<Block*>($<stmt>1);
        block->addStatement(std::unique_ptr<Statement>($2));
        $<stmt>$ = block;
    };

Stmt: 
    Block { $$ = $1; }
    | SEMICOLON { $$ = new Block(); }
    | Expr SEMICOLON { $$
 = new ExpressionStatement(std::unique_ptr<Expression>($1)); }
    | ID ASSIGN Expr SEMICOLON {
        $$ = new AssignmentStatement($1, std::unique_ptr<Expression>($3));
        SAFE_FREE($1);
    }
    | INT ID ASSIGN Expr SEMICOLON {
        $$
 = new VariableDeclaration($2, std::unique_ptr<Expression>($4));
        SAFE_FREE($2);
    }
    | IF LPAREN Expr RPAREN Stmt %prec NO_ELSE {
        $$ = new IfStatement(std::unique_ptr<Expression>($3), std::unique_ptr<Statement>($5));
    }
    | IF LPAREN Expr RPAREN Stmt ELSE Stmt {
        $$
 = new IfStatement(std::unique_ptr<Expression>($3), std::unique_ptr<Statement>($5), std::unique_ptr<Statement>($7));
    }
    | WHILE LPAREN Expr RPAREN Stmt {
        $$ = new WhileStatement(std::unique_ptr<Expression>($3), std::unique_ptr<Statement>($5));
    }
    | BREAK SEMICOLON { $$ = new BreakStatement(); }
    | CONTINUE SEMICOLON { $$ = new ContinueStatement(); }
    | RETURN SEMICOLON { $$ = new ReturnStatement(); }
    | RETURN Expr SEMICOLON { $$ = new ReturnStatement(std::unique_ptr<Expression>($2)); };

Expr: 
    LOrExpr { $$ = $1; };

LOrExpr: 
    LAndExpr { $$ = $1; }
    | LOrExpr OR LAndExpr {
        $$ = new BinaryExpression(std::unique_ptr<Expression>($1), BinaryExpression::OR, std::unique_ptr<Expression>($3));
    };

LAndExpr: 
    RelExpr { $$ = $1; }
    | LAndExpr AND RelExpr {
        $$ = new BinaryExpression(std::unique_ptr<Expression>($1), BinaryExpression::AND, std::unique_ptr<Expression>($3));
    };

RelExpr: 
    AddExpr { $$ = $1; }
    | RelExpr RelOp AddExpr {
        $$ = new BinaryExpression(std::unique_ptr<Expression>($1), $2, std::unique_ptr<Expression>($3));
    };

RelOp: 
    LT { $$ = BinaryExpression::LT; }
    | LE { $$ = BinaryExpression::LE; }
    | GT { $$ = BinaryExpression::GT; }
    | GE { $$ = BinaryExpression::GE; }
    | EQ { $$ = BinaryExpression::EQ; }
    | NE { $$ = BinaryExpression::NE; };

AddExpr: 
    MulExpr { $$ = $1; }
    | AddExpr AddOp MulExpr {
        $$ = new BinaryExpression(std::unique_ptr<Expression>($1), $2, std::unique_ptr<Expression>($3));
    };

AddOp: 
    PLUS { $$ = BinaryExpression::ADD; }
    | MINUS { $$ = BinaryExpression::SUB; };

MulExpr: 
    UnaryExpr { $$ = $1; }
    | MulExpr MulOp UnaryExpr {
        $$ = new BinaryExpression(std::unique_ptr<Expression>($1), $2, std::unique_ptr<Expression>($3));
    };

MulOp: 
    MULTIPLY { $$ = BinaryExpression::MUL; }
    | DIVIDE { $$ = BinaryExpression::DIV; }
    | MOD { $$ = BinaryExpression::MOD; };

UnaryExpr: 
    PrimaryExpr { $$
 = $1; }
    | UnaryOp UnaryExpr {
        $$ = new UnaryExpression($1, std::unique_ptr<Expression>($2));
    };

UnaryOp: 
    PLUS { $$ = UnaryExpression::PLUS; }
    | MINUS { $$ = UnaryExpression::MINUS; }
    | NOT { $$
 = UnaryExpression::NOT; };

PrimaryExpr: 
    ID {
        $$ = new Identifier($1);
        SAFE_FREE($1);
    }
    | NUMBER_LITERAL {
        $$ = new NumberLiteral($1);
    }
    | LPAREN Expr RPAREN { $$ = $2; }
    | ID LPAREN ExprList RPAREN {
        auto args = $3 ? std::move(*$3) : std::vector<std::unique_ptr<Expression>>();
        $$
 = new FunctionCall($1, std::move(args), Expression::INT);
        SAFE_FREE($1);
        delete $3;
    }
    | ID LPAREN RPAREN {
        $$ = new FunctionCall($1, std::vector<std::unique_ptr<Expression>>(), Expression::INT);
        SAFE_FREE($1);
    };

ExprList: 
    Expr {
        $$
 = new std::vector<std::unique_ptr<Expression>>();
        $$->push_back(std::unique_ptr<Expression>($1));
    }
    | ExprList COMMA Expr {
        $1->push_back(std::unique_ptr<Expression>($3));
        $$ = $1;
    };

%%

void yyerror(const char* s) {
    std::cerr << "Line " << yylineno << ": " << s;
    if (yytext) std::cerr << " near '" << yytext << "'";
    std::cerr << std::endl;
    
    if (root) root.reset();
}