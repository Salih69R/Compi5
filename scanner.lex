%{ 
/* Declarations section */

	#include "hw3_output.hpp"
	#include "structs.hpp"
	#include "parser.tab.hpp"
	#include<iostream>
    using namespace std;
	using namespace output;
	

	
	
%} 

%option yylineno 
%option noyywrap 	

whitespace										([\t\n\r ])

%%

void											{
													return VOID;
												}
int												{
													return INT;
												}
byte											{
													return BYTE;
												}
b												{
													return B;
												}
bool											{
													return BOOL;
												}
enum											{
													return ENUM;
												}
and 											{
													return AND;
												}
or												{
													return OR;
												}
not												{
													return NOT;
												}
true											{
													return TRUE;
												}
false 											{
													return FALSE;
												}
return											{
													return RETURN;
												}
if												{
													return IF;
												}
else											{
													return ELSE;
												}
while											{
													return WHILE;
												}
break											{
													return BREAK;
												}
continue 										{
													return CONTINUE;
												}
\;												{
													return SC;
												}
\,												{
													return COMMA;
												}
\(												{
													return LPAREN;
												}
\)												{
													return RPAREN;
												}
\{												{
													return LBRACE;
												}
\}												{
													return RBRACE;
												}
\=												{
													return ASSIGN;
												}
"<="|">="|"<"|">"								{
													return RELATIONOP;
												}	
"=="|"!="										{
													return EQUALOP;
												}
\+|\-											{
													return ADDOPS;
												}
\*|\/											{
													return MULTOPS;
												}
[a-zA-Z][a-zA-Z0-9]*							{ 
													yylval = new Node(ID_t, yytext,yytext);
													return ID; 
												} 
0|[1-9][0-9]*									{
													yylval = new Node(NUM_t,yytext,yytext);
													return NUM; 
												} 
["](([^\n\r\"\\]|\\[rnt"\\])+)["]				{
													yylval = new Node(STRING_t,yytext,yytext);
													return STRING;
												}
"//"[^\r\n]*[ \r|\n|\r\n]?						;
{whitespace}									;
.												{ errorLex(yylineno); exit(0);}

%%

