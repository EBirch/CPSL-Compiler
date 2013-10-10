%{
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include "symboltable.hpp"
#define YYERROR_VERBOSE 1

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern int lineNum;
extern char *yytext;
std::shared_ptr<SymbolTable> SymbolTable::instance;
void yyerror(const char *str);
%}

%union {
  int intVal;
  char *strVal;
  std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> *typeList;
  std::vector<std::string> *identList;
  Array *arrVal;
  Record *recVal;
  Type *typeVal;
  Const *constVal;
  void *none;
}

%token <intVal> NUM_SYM
%token <strVal> CHAR_SYM STRING_SYM IDENTIFIER_SYM

%token <none> ARRAY_SYM BEGIN_SYM CHR_SYM CONST_SYM DO_SYM DOWNTO_SYM ELSE_SYM ELSEIF_SYM END_SYM FOR_SYM FORWARD_SYM FUNCTION_SYM IF_SYM OF_SYM ORD_SYM PRED_SYM PROCEDURE_SYM READ_SYM RECORD_SYM REPEAT_SYM RETURN_SYM STOP_SYM SUCC_SYM THEN_SYM TO_SYM TYPE_SYM UNTIL_SYM VAR_SYM WHILE_SYM WRITE_SYM DOT_SYM COMMA_SYM COLON_SYM SEMICOLON_SYM LPAREN_SYM RPAREN_SYM LBRACK_SYM RBRACK_SYM

%type <intVal> Expression SimpleExprPrim
%type <identList> IdentList MoreIdents
%type <typeList> FormalParameters MoreParams RecVars
%type <constVal> ConstPrim ConstExpression
%type <typeVal> Type SimpleType
%type <recVal> RecordType 
%type <arrVal> ArrayType
// %type <none> program ConstantDecl MoreConst TypeDecl MoreType SimpleType RecordType ArrayType ProFuncDecl ProcedureDecl FunctionDecl Body Block StatementSequence MoreStatements Statement Assignment LValue Sublval Arguments MoreArgs IfStatement ElseIfs Else WhileStatement RepeatStatement ForStatement StopStatement ReturnStatement ReadStatement MoreLVals WriteStatement ProcedureCall NullStatement

%left OR_SYM
%left AND_SYM

%right NOT_SYM 

%nonassoc EQUALS_SYM NEQ_SYM LT_SYM LTE_SYM GT_SYM GTE_SYM

%left ADD_SYM SUB_SYM
%left MULT_SYM DIV_SYM MOD_SYM

%left ASSIGN_SYM

%right UNARYMINUS_SYM

%start program

%%

program: ConstantDecl TypeDecl VarDecl ProFuncDecl Block DOT_SYM{SymbolTable::getInstance()->popScope();SymbolTable::getInstance()->popScope();}
  ;
ConstantDecl:
  | CONST_SYM MoreConst IDENTIFIER_SYM EQUALS_SYM ConstExpression SEMICOLON_SYM{$5->name=$3; SymbolTable::getInstance()->addSymbol($3, *$5, true);}
  ;
MoreConst:
  | MoreConst IDENTIFIER_SYM EQUALS_SYM ConstExpression SEMICOLON_SYM {$4->name=$2; SymbolTable::getInstance()->addSymbol($2, *$4, true);}
  ;
TypeDecl:
  | TYPE_SYM MoreType IDENTIFIER_SYM EQUALS_SYM Type SEMICOLON_SYM{
    $5->name=$3; 
    auto temp=$5;
    SymbolTable::getInstance()->addSymbol($3, *temp, true); }
  ;
MoreType:
  | MoreType IDENTIFIER_SYM EQUALS_SYM Type SEMICOLON_SYM{$4->name=$2; 
    SymbolTable::getInstance()->addSymbol($2, *$4, true); }
  ;
Type: SimpleType{$<typeVal>$=$1;}
  | RecordType{$<recVal>$=$1;}
  | ArrayType{
    auto temp=$1;
    $<arrVal>$=$1;}
  ;
SimpleType: IDENTIFIER_SYM{SymbolTable::getInstance()->checkType($1); $$=dynamic_cast<Type*>(SymbolTable::getInstance()->getSymbol($1).get());}
  ;
RecordType: RECORD_SYM RecVars END_SYM{$$=new Record(*$2);}
  ;
RecVars: {$$=new std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>>();}
  | RecVars IdentList COLON_SYM Type SEMICOLON_SYM{$1->push_back(std::make_pair(*$2, std::make_shared<Type>(*$4))); $$=$1;}
  ;
ArrayType: ARRAY_SYM LBRACK_SYM ConstExpression COLON_SYM ConstExpression RBRACK_SYM OF_SYM Type{$$=new Array($8, *$3, *$5);}
  ;
IdentList: MoreIdents IDENTIFIER_SYM{$1->push_back($2); $$=$1;}
  ;
MoreIdents: {$$=new std::vector<std::string>();}
  | MoreIdents IDENTIFIER_SYM COMMA_SYM{$1->push_back($2); $$=$1;}
  ;
VarDecl:
  | VAR_SYM MoreVars IdentList COLON_SYM Type SEMICOLON_SYM{std::for_each($3->begin(), $3->end(), [&](std::string val){Var var(*$5, val); SymbolTable::getInstance()->addSymbol(val, var, true);});}
  ;
MoreVars:
  | MoreVars IdentList COLON_SYM Type SEMICOLON_SYM{std::for_each($2->begin(), $2->end(), [&](std::string val){Var var(*$4, val); SymbolTable::getInstance()->addSymbol(val, var, true);});}
  ;
ProFuncDecl:
  | ProFuncDecl ProcedureDecl
  | ProFuncDecl FunctionDecl
  ;
ProcedureDecl: PROCEDURE_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM SEMICOLON_SYM FORWARD_SYM SEMICOLON_SYM{Function func(std::string($2), *$4); SymbolTable::getInstance()->addFunction($2, func, true);}
  | ProcedureStart Body SEMICOLON_SYM{SymbolTable::getInstance()->popScope();}
  ;
ProcedureStart: PROCEDURE_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM SEMICOLON_SYM{Function func(std::string($2), *$4, true); SymbolTable::getInstance()->addFunction($2, func); SymbolTable::getInstance()->pushScope(func);}
  ;
FunctionDecl: FUNCTION_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM COLON_SYM Type SEMICOLON_SYM FORWARD_SYM SEMICOLON_SYM{Function func(std::string($2), *$7, *$4); SymbolTable::getInstance()->addFunction($2, func, true);}
  | FunctionStart Body SEMICOLON_SYM{SymbolTable::getInstance()->popScope();}
  ;
FunctionStart: FUNCTION_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM COLON_SYM Type SEMICOLON_SYM{Function func(std::string($2), *$4, true); SymbolTable::getInstance()->addFunction($2, func); SymbolTable::getInstance()->pushScope(func);}
  ;
FormalParameters: {$$=new std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>>();}
  | IdentList COLON_SYM Type MoreParams{$4->push_back(std::make_pair(*$1, std::make_shared<Type>(*$3))); $$=$4;}
  | VAR_SYM IdentList COLON_SYM Type MoreParams{$5->push_back(std::make_pair(*$2, std::make_shared<Type>(*$4))); $$=$5;}
  ;
MoreParams: {$$=new std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>>();}
  | SEMICOLON_SYM IdentList COLON_SYM Type MoreParams{$5->push_back(std::make_pair(*$2, std::make_shared<Type>(*$4))); $$=$5;}
  | SEMICOLON_SYM VAR_SYM IdentList COLON_SYM Type MoreParams{$6->push_back(std::make_pair(*$3, std::make_shared<Type>(*$5))); $$=$6;}
  ;
Body: ConstantDecl TypeDecl VarDecl Block
  ;
Block: BEGIN_SYM StatementSequence END_SYM
  ;
StatementSequence: MoreStatements Statement
  ;
MoreStatements:
  | MoreStatements Statement SEMICOLON_SYM
  ;
Statement: Assignment
  | IfStatement
  | WhileStatement
  | RepeatStatement
  | ForStatement
  | StopStatement
  | ReturnStatement
  | ReadStatement
  | WriteStatement
  | ProcedureCall
  | NullStatement
  ;
Assignment: LValue ASSIGN_SYM Expression
  ;
LValue: IDENTIFIER_SYM Sublval //TODO: deal with lvals?
  ;
Sublval:
  | DOT_SYM IDENTIFIER_SYM Sublval
  | LBRACK_SYM Expression RBRACK_SYM Sublval
  ;
Expression: SimpleExprPrim
  | Expression OR_SYM Expression
  | Expression AND_SYM Expression
  | Expression EQUALS_SYM Expression
  | Expression NEQ_SYM Expression
  | Expression LTE_SYM Expression
  | Expression GTE_SYM Expression
  | Expression LT_SYM Expression
  | Expression GT_SYM Expression
  | Expression ADD_SYM Expression
  | Expression SUB_SYM Expression
  | Expression MULT_SYM Expression
  | Expression DIV_SYM Expression
  | Expression MOD_SYM Expression
  | NOT_SYM Expression
  | SUB_SYM Expression %prec UNARYMINUS_SYM
  | LPAREN_SYM Expression RPAREN_SYM
  | IDENTIFIER_SYM LPAREN_SYM Arguments RPAREN_SYM
  | CHR_SYM LPAREN_SYM Expression RPAREN_SYM
  | ORD_SYM LPAREN_SYM Expression RPAREN_SYM
  | PRED_SYM LPAREN_SYM Expression RPAREN_SYM
  | SUCC_SYM LPAREN_SYM Expression RPAREN_SYM
  ;
SimpleExprPrim: LValue
  | NUM_SYM
  | CHAR_SYM
  | STRING_SYM
  ;
ConstExpression: ConstPrim{$$=$1;}
  | ConstExpression OR_SYM ConstExpression
  | ConstExpression AND_SYM ConstExpression
  | ConstExpression EQUALS_SYM ConstExpression
  | ConstExpression NEQ_SYM ConstExpression
  | ConstExpression LTE_SYM ConstExpression
  | ConstExpression GTE_SYM ConstExpression
  | ConstExpression LT_SYM ConstExpression
  | ConstExpression GT_SYM ConstExpression
  | ConstExpression ADD_SYM ConstExpression
  | ConstExpression SUB_SYM ConstExpression
  | ConstExpression MULT_SYM ConstExpression
  | ConstExpression DIV_SYM ConstExpression
  | ConstExpression MOD_SYM ConstExpression
  | NOT_SYM ConstExpression//{$$=!$2;}
  | SUB_SYM ConstExpression %prec UNARYMINUS_SYM//{$$=-$2;}
  | LPAREN_SYM ConstExpression RPAREN_SYM{$$=$2;}
  ;  
ConstPrim: NUM_SYM{$$=new Const($1);}
  | CHAR_SYM{$$=new Const(*$1);}
  | STRING_SYM{$$=new Const($1);}
  | IDENTIFIER_SYM{$$=new Const($1);}
  ;
Arguments: 
  | MoreArgs Expression
  ;
MoreArgs:
  | MoreArgs Expression COMMA_SYM
  ;
IfStatement: IF_SYM Expression THEN_SYM StatementSequence ElseIfs Else END_SYM
  ;
ElseIfs: 
  | ELSEIF_SYM Expression THEN_SYM StatementSequence ElseIfs
  ;
Else:
  | ELSE_SYM StatementSequence
  ;
WhileStatement: WHILE_SYM Expression DO_SYM StatementSequence END_SYM
  ;
RepeatStatement: REPEAT_SYM StatementSequence UNTIL_SYM Expression
  ;
ForStatement: FOR_SYM IDENTIFIER_SYM ASSIGN_SYM Expression TO_SYM Expression DO_SYM StatementSequence END_SYM
  | FOR_SYM IDENTIFIER_SYM ASSIGN_SYM Expression DOWNTO_SYM Expression DO_SYM StatementSequence END_SYM
  ;
StopStatement: STOP_SYM
  ;
ReturnStatement: RETURN_SYM Expression
  | RETURN_SYM
  ;
ReadStatement: READ_SYM LPAREN_SYM MoreLVals LValue RPAREN_SYM
  ;
MoreLVals: 
  | MoreLVals LValue COMMA_SYM
  ;
WriteStatement: WRITE_SYM LPAREN_SYM MoreArgs Expression RPAREN_SYM
  ;
ProcedureCall: IDENTIFIER_SYM LPAREN_SYM MoreArgs Expression RPAREN_SYM
  | IDENTIFIER_SYM LPAREN_SYM RPAREN_SYM
  ;
NullStatement:
  ;

%%

int main(int argc, char **argv){
  if(argc<2){
    std::cout<<"Need file to parse\n";
    return -1;
  }
  FILE *temp=fopen(argv[1], "r");
  if(!temp){
    std::cout<<"Error opening file\n";
    return -1;
  }
  yyin=temp;
  yyparse();
}

void yyerror(const char *str){
  std::cout<<"Bison error on line "<<lineNum<<": "<<str<<"\n";
  exit(-1);
}
