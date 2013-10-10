%option noyywrap
%{
	#include <iostream>
  #include <stdlib.h>
  #include <vector>
  #include "symboltable.hpp"
	#include "CPSL.tab.h"
  #define YY_DECL extern "C" int yylex()
	int lineNum=1;
  bool debug=false;
%}
%%
(array)|(ARRAY) {if(debug){std::cout<<"ARRAY_SYM\n";}return(ARRAY_SYM);}
(begin)|(BEGIN)	{if(debug){std::cout<<"BEGIN_SYM\n";}return(BEGIN_SYM);}
(chr)|(CHR)	{if(debug){std::cout<<"CHR_SYM\n";}return(CHR_SYM);}
(const)|(CONST)	{if(debug){std::cout<<"CONST_SYM\n";}return(CONST_SYM);}
(do)|(DO)	{if(debug){std::cout<<"DO_SYM\n";}return(DO_SYM);}
(downto)|(DOWNTO)	{if(debug){std::cout<<"DOWNTO_SYM\n";}return(DOWNTO_SYM);}
(else)|(ELSE)	{if(debug){std::cout<<"ELSE_SYM\n";}return(ELSE_SYM);}
(elseif)|(ELSEIF)	{if(debug){std::cout<<"ELSEIF_SYM\n";}return(ELSEIF_SYM);}
(end)|(END)	{if(debug){std::cout<<"END_SYM\n";}return(END_SYM);}
(for)|(FOR)	{if(debug){std::cout<<"FOR_SYM\n";}return(FOR_SYM);}
(forward)|(FORWARD)	{if(debug){std::cout<<"FORWARD_SYM\n";}return(FORWARD_SYM);}
(function)|(FUNCTION)	{if(debug){std::cout<<"FUNCTION_SYM\n";}return(FUNCTION_SYM);}
(if)|(IF)	{if(debug){std::cout<<"IF_SYM\n";}return(IF_SYM);}
(of)|(OF)	{if(debug){std::cout<<"OF_SYM\n";}return(OF_SYM);}
(ord)|(ORD)	{if(debug){std::cout<<"ORD_SYM\n";}return(ORD_SYM);}
(pred)|(PRED)	{if(debug){std::cout<<"PRED_SYM\n";}return(PRED_SYM);}
(procedure)|(PROCEDURE)	{if(debug){std::cout<<"PROCEDURE_SYM\n";}return(PROCEDURE_SYM);}
(read)|(READ)	{if(debug){std::cout<<"READ_SYM\n";}return(READ_SYM);}
(record)|(RECORD)	{if(debug){std::cout<<"RECORD_SYM\n";}return(RECORD_SYM);}
(repeat)|(REPEAT)	{if(debug){std::cout<<"REPEAT_SYM\n";}return(REPEAT_SYM);}
(return)|(RETURN)	{if(debug){std::cout<<"RETURN_SYM\n";}return(RETURN_SYM);}
(stop)|(STOP)	{if(debug){std::cout<<"STOP_SYM\n";}return(STOP_SYM);}
(succ)|(SUCC)	{if(debug){std::cout<<"SUCC_SYM\n";}return(SUCC_SYM);}
(then)|(THEN)	{if(debug){std::cout<<"THEN_SYM\n";}return(THEN_SYM);}
(to)|(TO)	{if(debug){std::cout<<"TO_SYM\n";}return(TO_SYM);}
(type)|(TYPE)	{if(debug){std::cout<<"TYPE_SYM\n";}return(TYPE_SYM);}
(until)|(UNTIL)	{if(debug){std::cout<<"UNTIL_SYM\n";}return(UNTIL_SYM);}
(var)|(VAR)	{if(debug){std::cout<<"VAR_SYM\n";}return(VAR_SYM);}
(while)|(WHILE)	{if(debug){std::cout<<"WHILE_SYM\n";}return(WHILE_SYM);}
(write)|(WRITE)	{if(debug){std::cout<<"WRITE_SYM\n";}return(WRITE_SYM);}
[a-zA-Z][a-zA-Z0-9_]* {if(debug){std::cout<<"IDENTIFIER_SYM\n";}yylval.strVal=strdup(yytext);return(IDENTIFIER_SYM);}
"\+" {if(debug){std::cout<<"ADD_SYM\n";}return(ADD_SYM);}
"-" {if(debug){std::cout<<"SUB_SYM\n";}return(SUB_SYM);}
"\*" {if(debug){std::cout<<"MULT_SYM\n";}return(MULT_SYM);}
"/" {if(debug){std::cout<<"DIV_SYM\n";}return(DIV_SYM);}
"&" {if(debug){std::cout<<"AND_SYM\n";}return(AND_SYM);}
"\|" {if(debug){std::cout<<"OR_SYM\n";}return(OR_SYM);}
"~" {if(debug){std::cout<<"NOT_SYM\n";}return(NOT_SYM);}
"=" {if(debug){std::cout<<"EQUALS_SYM\n";}return(EQUALS_SYM);}
"<>" {if(debug){std::cout<<"NEQ_SYM\n";}return(NEQ_SYM);}
"<" {if(debug){std::cout<<"LT_SYM\n";}return(LT_SYM);}
"<=" {if(debug){std::cout<<"LTE_SYM\n";}return(LTE_SYM);}
">" {if(debug){std::cout<<"GT_SYM\n";}return(GT_SYM);}
">=" {if(debug){std::cout<<"GTE_SYM\n";}return(GTE_SYM);}
"\." {if(debug){std::cout<<"DOT_SYM\n";}return(DOT_SYM);}
"," {if(debug){std::cout<<"COMMA_SYM\n";}return(COMMA_SYM);}
":" {if(debug){std::cout<<"COLON_SYM\n";}return(COLON_SYM);}
";" {if(debug){std::cout<<"SEMICOLON_SYM\n";}return(SEMICOLON_SYM);}
"\(" {if(debug){std::cout<<"LPAREN_SYM\n";}return(LPAREN_SYM);}
"\)" {if(debug){std::cout<<"RPAREN_SYM\n";}return(RPAREN_SYM);}
"[" {if(debug){std::cout<<"LBRACK_SYM\n";}return(LBRACK_SYM);}
"]" {if(debug){std::cout<<"RBRACK_SYM\n";}return(RBRACK_SYM);}
":=" {if(debug){std::cout<<"ASSIGN_SYM\n";}return(ASSIGN_SYM);}
"%" {if(debug){std::cout<<"MOD_SYM\n";}return(MOD_SYM);}
0[1-7][0-7]*	{if(debug){std::cout<<"NUM_SYM\n";}yylval.intVal=strtol(yytext, &yytext, 8);return(NUM_SYM);}
[1-9][0-9]*	{if(debug){std::cout<<"NUM_SYM\n";}yylval.intVal=atoi(yytext);return(NUM_SYM);}
0x[0-9a-fA-F]+	{if(debug){std::cout<<"NUM_SYM\n";}yylval.intVal=strtol(yytext, &yytext, 16);return(NUM_SYM);}
0 {if(debug){std::cout<<"NUM_SYM\n";}yylval.intVal=0;return(NUM_SYM);}
'\\?[ -~]'	{if(debug){std::cout<<"CHAR_SYM\n";}yylval.strVal=strdup(yytext);return(CHAR_SYM);}
\"[ -!#-~]*\"	{if(debug){std::cout<<"STRING_SYM\n";}yylval.strVal=strdup(yytext);return(STRING_SYM);}
\$[^\r\n]*	{}
[ \t]	{}
[\n\r]	{++lineNum;}
.	{std::cout<<"Unrecognized text on line"<<lineNum<<": "<<yytext<<std::endl;return -1;}
%%
