%{
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <fstream>
#include "symboltable.hpp"
#define YYERROR_VERBOSE 1

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern int lineNum;
extern char *yytext;
std::shared_ptr<SymbolTable> SymbolTable::instance;
std::fstream emit;
bool verbose=false;
void yyerror(const char *str);
%}

%union {
  int intVal;
  char *strVal;
  std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> *typeList;
  std::vector<std::string> *identList;
  std::vector<Expression> *exprList;
  Array *arrVal;
  Record *recVal;
  Type *typeVal;
  Const *constVal;
  Expression *expr;
  void *none;
}

%token <intVal> NUM_SYM
%token <strVal> CHAR_SYM STRING_SYM IDENTIFIER_SYM

%token <none> ARRAY_SYM BEGIN_SYM CHR_SYM CONST_SYM DO_SYM DOWNTO_SYM ELSE_SYM ELSEIF_SYM END_SYM FOR_SYM FORWARD_SYM FUNCTION_SYM IF_SYM OF_SYM ORD_SYM PRED_SYM PROCEDURE_SYM READ_SYM RECORD_SYM REPEAT_SYM RETURN_SYM STOP_SYM SUCC_SYM THEN_SYM TO_SYM TYPE_SYM UNTIL_SYM VAR_SYM WHILE_SYM WRITE_SYM DOT_SYM COMMA_SYM COLON_SYM SEMICOLON_SYM LPAREN_SYM RPAREN_SYM LBRACK_SYM RBRACK_SYM

%type <identList> IdentList MoreIdents 
%type <typeList> FormalParameters MoreParams RecVars
%type <constVal> ConstPrim ConstExpression
%type <typeVal> Type SimpleType
%type <recVal> RecordType 
%type <arrVal> ArrayType
%type <expr> Expression SimpleExprPrim LValue 
%type <exprList> ExprList MoreLVals Sublval
// %type <intVal> LValue
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

program: ConstantDecl TypeDecl VarDecl ProFuncDecl Block DOT_SYM{
      SymbolTable::getInstance()->popScope();
      SymbolTable::getInstance()->popScope();
      SymbolTable::getInstance()->emitEnd();
    }
  ;
ConstantDecl:
  | CONST_SYM MoreConst IDENTIFIER_SYM EQUALS_SYM ConstExpression SEMICOLON_SYM{
      $5->name=$3;
      SymbolTable::getInstance()->addSymbol($3, *$5, true);
    }
  ;
MoreConst:
  | MoreConst IDENTIFIER_SYM EQUALS_SYM ConstExpression SEMICOLON_SYM{
      $4->name=$2;
      SymbolTable::getInstance()->addSymbol($2, *$4, true);
    }
  ;
TypeDecl:
  | TYPE_SYM MoreType IDENTIFIER_SYM EQUALS_SYM Type SEMICOLON_SYM{
      $5->name=$3; 
      auto temp=$5;
      switch($5->typeType){
        case Type::type:SymbolTable::getInstance()->addSymbol($3, *$5, true);
          break;
        case Type::array:SymbolTable::getInstance()->addSymbol($3, *(dynamic_cast<Array*>($5)), true);
          break;
        case Type::record:SymbolTable::getInstance()->addSymbol($3, *(dynamic_cast<Record*>($5)), true);
          break;
      }
    }
  ;
MoreType:
  | MoreType IDENTIFIER_SYM EQUALS_SYM Type SEMICOLON_SYM{
      $4->name=$2; 
      auto temp=$5;
      switch($4->typeType){
        case Type::type:SymbolTable::getInstance()->addSymbol($2, *$4, true);
          break;
        case Type::array:SymbolTable::getInstance()->addSymbol($2, *(dynamic_cast<Array*>($4)), true);
          break;
        case Type::record:SymbolTable::getInstance()->addSymbol($2, *(dynamic_cast<Record*>($4)), true);
          break;
      }
    }
  ;
Type: SimpleType{
      $<typeVal>$=$1;
    }
  | RecordType{
    auto temp=$1;
      $<recVal>$=$1;
    }
  | ArrayType{
      auto temp=$1;
      $<arrVal>$=$1;
    }
  ;
SimpleType: IDENTIFIER_SYM{
      SymbolTable::getInstance()->checkType($1);
      $$=dynamic_cast<Type*>(SymbolTable::getInstance()->getSymbol($1).get());
    }
  ;
RecordType: RECORD_SYM RecVars END_SYM{
      $$=new Record(*$2);
    }
  ;
RecVars: {
      $$=new std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>>();
    }
  | RecVars IdentList COLON_SYM Type SEMICOLON_SYM{
      $1->push_back(std::make_pair(*$2, std::make_shared<Type>(*$4)));
      $$=$1;
    }
  ;
ArrayType: ARRAY_SYM LBRACK_SYM ConstExpression COLON_SYM ConstExpression RBRACK_SYM OF_SYM Type{
      $$=new Array($8, *$3, *$5);
    }
  ;
IdentList: MoreIdents IDENTIFIER_SYM{
      $1->push_back($2);
      $$=$1;
    }
  ;
MoreIdents: {
      $$=new std::vector<std::string>();
    }
  | MoreIdents IDENTIFIER_SYM COMMA_SYM{
      $1->push_back($2);
      $$=$1;
    }
  ;
VarDecl:
  | VAR_SYM MoreVars IdentList COLON_SYM Type SEMICOLON_SYM{
      std::for_each($3->begin(), $3->end(), 
        [&](std::string val){
          Var var(*$5, SymbolTable::getInstance()->offset.back(), val);
          SymbolTable::getInstance()->addSymbol(val, var, true);
        });
    }
  ;
MoreVars:
  | MoreVars IdentList COLON_SYM Type SEMICOLON_SYM{
      std::for_each($2->begin(), $2->end(), 
        [&](std::string val){
          Var var(*$4, SymbolTable::getInstance()->offset.back(), val);
          SymbolTable::getInstance()->addSymbol(val, var, true);
        });
    }
  ;
ProFuncDecl:
  | ProFuncDecl ProcedureDecl
  | ProFuncDecl FunctionDecl
  ;
ProcedureDecl: PROCEDURE_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM SEMICOLON_SYM FORWARD_SYM SEMICOLON_SYM{
      Function func(std::string($2), *$4);
      SymbolTable::getInstance()->addFunction($2, func, true);
    }
  | ProcedureStart Body SEMICOLON_SYM{
      SymbolTable::getInstance()->popScope();
    }
  ;
ProcedureStart: PROCEDURE_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM SEMICOLON_SYM{
      Function func(std::string($2), *$4, true);
      SymbolTable::getInstance()->addFunction($2, func);
      SymbolTable::getInstance()->pushScope(func);
    }
  ;
FunctionDecl: FUNCTION_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM COLON_SYM Type SEMICOLON_SYM FORWARD_SYM SEMICOLON_SYM{
      Function func(std::string($2), *$7, *$4);
      SymbolTable::getInstance()->addFunction($2, func, true);
    }
  | FunctionStart Body SEMICOLON_SYM{
      SymbolTable::getInstance()->popScope();
    }
  ;
FunctionStart: FUNCTION_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM COLON_SYM Type SEMICOLON_SYM{
      Function func(std::string($2), *$7, *$4, true);
      SymbolTable::getInstance()->addFunction($2, func);
      SymbolTable::getInstance()->pushScope(func);
    }
  ;
FormalParameters: {
      $$=new std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>>();
    }
  | MoreParams IdentList COLON_SYM Type{
      $1->push_back(std::make_pair(*$2, std::make_shared<Type>(*$4)));
      $$=$1;
    }
  | MoreParams VAR_SYM IdentList COLON_SYM Type{
      $1->push_back(std::make_pair(*$3, std::make_shared<Type>(*$5)));
      $$=$1;
    }
  ;
MoreParams: {
      $$=new std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>>();
    }
  | MoreParams IdentList COLON_SYM Type SEMICOLON_SYM{
      $1->push_back(std::make_pair(*$2, std::make_shared<Type>(*$4)));
      $$=$1;
    }
  | MoreParams VAR_SYM IdentList COLON_SYM Type SEMICOLON_SYM{
      $1->push_back(std::make_pair(*$3, std::make_shared<Type>(*$5)));
      $$=$1;
    }
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
Statement: Assignment{
      SymbolTable::getInstance()->clearReg();
    } 
  | IfStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | WhileStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | RepeatStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | ForStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | StopStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | ReturnStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | ReadStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | WriteStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  | ProcedureCall{
      SymbolTable::getInstance()->clearReg();
    } 
  | NullStatement{
      SymbolTable::getInstance()->clearReg();
    } 
  ;
Assignment: LValue ASSIGN_SYM Expression{
      assign($1, $3);
    }
  ;
LValue: IDENTIFIER_SYM Sublval{
      auto temp=new Expression(std::string($1), Expression::stringType);
      $2->push_back(*temp);
      $$=getLval(*$2);
    }
  ;
Sublval:{
      $$=new std::vector<Expression>();
    }
  | DOT_SYM IDENTIFIER_SYM Sublval{
      auto temp=new Expression(std::string($2), Expression::stringType);
      $3->push_back(*temp);
      $$=$3;
    }
  | LBRACK_SYM Expression RBRACK_SYM Sublval{
      $4->push_back(*$2);
      $$=$4;
    }
  ;
Expression: SimpleExprPrim{
      $$=$1;
    }
  | Expression OR_SYM Expression{
      $$=eval($1, $3, "or");
    }
  | Expression AND_SYM Expression{
      $$=eval($1, $3, "and");
    }
  | Expression EQUALS_SYM Expression{
      $$=eval($1, $3, "seq");
    }
  | Expression NEQ_SYM Expression{
      $$=eval($1, $3, "sne");
    }
  | Expression LTE_SYM Expression{
      $$=eval($1, $3, "sle");
    }
  | Expression GTE_SYM Expression{
      $$=eval($1, $3, "sge");
    }
  | Expression LT_SYM Expression{
      $$=eval($1, $3, "slt");
    }
  | Expression GT_SYM Expression{
      $$=eval($1, $3, "sgt");
    }
  | Expression ADD_SYM Expression{
      $$=eval($1, $3, "add");
    }
  | Expression SUB_SYM Expression{
      $$=eval($1, $3, "sub");
    }
  | Expression MULT_SYM Expression{
      $$=evalSpec($1, $3, "mult");
    }
  | Expression DIV_SYM Expression{
      $$=evalSpec($1, $3, "div");
    }
  | Expression MOD_SYM Expression{
      $$=evalSpec($1, $3, "mod");
    }
  | NOT_SYM Expression{
      $$=evalUnary($2, "not");
    }
  | SUB_SYM Expression %prec UNARYMINUS_SYM{
      $$=evalUnary($2, "neg");
    }
  | LPAREN_SYM Expression RPAREN_SYM{
      $$=$2;
    }
  | IDENTIFIER_SYM LPAREN_SYM Arguments RPAREN_SYM
  | CHR_SYM LPAREN_SYM Expression RPAREN_SYM
  | ORD_SYM LPAREN_SYM Expression RPAREN_SYM
  | PRED_SYM LPAREN_SYM Expression RPAREN_SYM
  | SUCC_SYM LPAREN_SYM Expression RPAREN_SYM
  ;
SimpleExprPrim: LValue{
      $$=$1;
    }
  | NUM_SYM{
      $$=new Expression($1, Expression::intType, true);
    }
  | CHAR_SYM{
      $$=new Expression($1[1], Expression::charType, true);
    }
  | STRING_SYM{
      if(SymbolTable::getInstance()->lookup(std::string($1))){
        auto temp=*(dynamic_cast<Const*>(SymbolTable::getInstance()->getSymbol($1).get()));
        $$=new Expression(temp.location, Expression::stringType, true);
      }
      else{
        auto temp=new Const(std::string($1), std::string($1));
        SymbolTable::getInstance()->addSymbol(std::string($1), *temp, true);
        $$=new Expression(temp->location, Expression::stringType, true);
      }
    }
  ;
ConstExpression: ConstPrim{
      $$=$1;
    }
  | ConstExpression OR_SYM ConstExpression{
      $$=orOp(*$1, *$3);
    }
  | ConstExpression AND_SYM ConstExpression{
      $$=andOp(*$1, *$3);
    }
  | ConstExpression EQUALS_SYM ConstExpression{
      $$=eq(*$1, *$3);
    }
  | ConstExpression NEQ_SYM ConstExpression{
      $$=neq(*$1, *$3);
    }
  | ConstExpression LTE_SYM ConstExpression{
      $$=lte(*$1, *$3);
    }
  | ConstExpression GTE_SYM ConstExpression{
      $$=gte(*$1, *$3);
    }
  | ConstExpression LT_SYM ConstExpression{
      $$=lt(*$1, *$3);
    }
  | ConstExpression GT_SYM ConstExpression{
      $$=gt(*$1, *$3);
    }
  | ConstExpression ADD_SYM ConstExpression{
      $$=add(*$1, *$3);
    }
  | ConstExpression SUB_SYM ConstExpression{
      $$=sub(*$1, *$3);
    }
  | ConstExpression MULT_SYM ConstExpression{
      $$=mult(*$1, *$3);
    }
  | ConstExpression DIV_SYM ConstExpression{
      $$=div(*$1, *$3);
    }
  | ConstExpression MOD_SYM ConstExpression{
      $$=mod(*$1, *$3);
    }
  | NOT_SYM ConstExpression{
      $$=notOp(*$2);
    }
  | SUB_SYM ConstExpression %prec UNARYMINUS_SYM{
      $$=negative(*$2);
    }
  | LPAREN_SYM ConstExpression RPAREN_SYM{
      $$=$2;
    }
  ;  
ConstPrim: NUM_SYM{
      $$=new Const($1);
    }
  | CHAR_SYM{
      $$=new Const($1[1]);
    }
  | STRING_SYM{
      $$=new Const(std::string($1), std::string($1));
    }
  | IDENTIFIER_SYM{
      $$=new Const(std::string($1), Const::identType);
    }
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
ReadStatement: READ_SYM LPAREN_SYM MoreLVals LValue RPAREN_SYM{
      $3->push_back(*$4);
      read(*$3);
    }
  ;
MoreLVals: {
      $$=new std::vector<Expression>();
    }
  | MoreLVals LValue COMMA_SYM{
      $1->push_back(*$2);
      $$=$1;
    }
  ;
WriteStatement: WRITE_SYM LPAREN_SYM ExprList Expression RPAREN_SYM{
      $3->push_back(*$4);
      write(*$3);
    }
  ;
ExprList: {
      $$=new std::vector<Expression>();
    }
  | ExprList Expression COMMA_SYM{
      $1->push_back(*$2);
      $$=$1;
    }
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
  if(argc>=3){
    if(std::string(argv[2])=="-v"){
      verbose=true;
    }
  }
  std::string emitFile(argv[1]);
  emitFile+=".cpsl";
  emit.open(emitFile.data(), std::ios::out);
  emit<<".text"<<std::endl<<".globl __main"<<std::endl<<"__main:"<<std::endl;
  yyin=temp;
  yyparse();
  emit.close();
  std::cout<<"Compiled to "<<emitFile<<std::endl;
}

void yyerror(const char *str){
  std::cout<<"Parse error on line "<<lineNum<<": "<<str<<"\n";
  exit(-1);
}
