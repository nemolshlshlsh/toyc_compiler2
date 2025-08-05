%{
#include "ast/ast.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <cstring>

extern int yylineno;
extern YYSTYPE yylval;
%}

%option noyywrap
%option warn nodefault
%option yylineno


DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
NUMBER      -?(0|[1-9][0-9]*)
WHITESPACE  [ \t\r]+
NEWLINE     \n

%%

{WHITESPACE}    { /* 忽略空白字符 */ }
{NEWLINE}       { yylineno++; }


"//".*          { /* 忽略单行注释 */ }


"/*"            { 
    int c;
    while ((c = yyinput()) != 0) {
        if (c == '*') {
            c = yyinput();
            if (c == '/') break;
            
        }
        if (c == EOF) {
            fprintf(stderr, "Unterminated comment at line %d\n", yylineno);
            break;
        }
    }
}


"int"           { return INT; }
"void"          { return VOID; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }
"return"        { return RETURN; }


"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULTIPLY; }
"/"             { return DIVIDE; }
"%"             { return MOD; }
"="             { return ASSIGN; }
"=="            { return EQ; }
"!="            { return NE; }
"<"             { return LT; }
"<="            { return LE; }
">"             { return GT; }
">="            { return GE; }
"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }


"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
","             { return COMMA; }
";"             { return SEMICOLON; }


{ID}            { 
    yylval.str_val = strdup(yytext);
    if (!yylval.str_val) {
        fprintf(stderr, "Memory error at line %d\n", yylineno);
        exit(EXIT_FAILURE);
    }
    return ID;
}

{NUMBER}        { 
    yylval.int_val = strtol(yytext, NULL, 10);
    return NUMBER_LITERAL;
}


.               { 
    fprintf(stderr, "Illegal character '%s' at line %d\n", yytext, yylineno);
    return ERROR;
}

%%