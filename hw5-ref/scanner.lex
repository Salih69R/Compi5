%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string>
	#include "SymTable.h"
	#include "output.hpp"
	#include "parser.tab.hpp"
	using namespace output;
%}

%option yylineno
%option noyywrap
digit ([0-9])
letter ([a-zA-Z])
hexa ([a-fA-F0-9])
octa ([0-7])
reg (([\+\-]{digit}+)|{digit}+)
whitespace ([\n\t\r ])|(\/\/[^\r\n]*[\r|\n|\r\n]?)

%%

void                            	      return VOID;
int                                 	  return INT;
byte   	            	                  return BYTE;
b       	                              return B;
bool        	                          return BOOL;
and             	                      return AND;
or      	                              return OR;
not         	                          return NOT;
true            	                      return TRUE;
false               	                  return FALSE;
return                  	              return RETURN;
if                          	          return IF;
else                            	      return ELSE;
while                               	  return WHILE;
break	                                  return BREAK;
\;       	                              return SC;
\,           	                          return COMMA;
\(      	                              return LPAREN;
\)          	                          return RPAREN;
\{              	                      return LBRACE;
\}                  	                  return RBRACE;
\[                      	              return LBRACK;
\]                          	          return RBRACK;
=                               	      return ASSIGN;
==|!=|<|>|<=|>=                     	  {yylval=new Var(yytext); return RELOP;}
(\+|\-)          	                      {yylval=new Var(yytext); return ADDBIN;}
(\*|\/)                                   {yylval=new Var(yytext); return MULBIN;}
[a-zA-Z][a-zA-Z0-9]*     	              {yylval=new Var(yytext); return ID;}
0|[1-9][0-9]*	  						  {yylval=new Var("", "", atoi(yytext)); return NUM;}
\"([^\n\r\"\\]|\\[rnt\\])+\"      	      {yylval=new Var(yytext); return STRING;}
{whitespace}							  ;
.										  {errorLex(yylineno); exit(0);}

%%
