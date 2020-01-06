%{ /* Declarations section */
#include "stdio.h"
#include "hw3_output.hpp"
#include "Semantic_types.hpp"
#include "parser.tab.hpp"

using namespace output;
%}

%option yylineno
%option noyywrap
relational (<|>|<=|>=)
equality (==|!=)
additive ([+\-])
mul ([*\/])
letter ([a-zA-Z])
digit ([0-9])
whitespace([ \t\n\r]+)
comment (\/\/[^\r\n]*[\r|\n|\r\n]?)

%%
"void" return VOID;
"int" return INT;
"byte" return BYTE;
"b" return B;
"bool" return BOOL;
"enum" return ENUM;
"and" return AND;
"or" return OR;
"not" return NOT;
"true" return TRUE;
"false" return FALSE;
"return" return RETURN;
"if" return IF;
"else" return ELSE;
"while" return WHILE;
"break" return BREAK;
"continue" return CONTINUE;
";" return SC;
"," return COMMA;
"(" return LPAREN;
")" return RPAREN;
"{" return LBRACE;
"}" return RBRACE;
"=" return ASSIGN;
{relational} return RELATIONAL;
{equality} return EQUALITY;
{mul} return MUL;
{additive} return ADDITIVE;
{letter}({letter}|{digit})* {yylval.strType = (char*)malloc(yyleng + 1); strcpy(yylval.strType, yytext); return ID;}
0|[1-9]{digit}* {yylval.numeric = atoi(yytext); return NUM;};
\"([^\n\r\"\\]|\\[rnt"\\])+\" return STRING;
{comment} ;
{whitespace} ;
. {errorLex(yylineno); exit(0);};


%%


