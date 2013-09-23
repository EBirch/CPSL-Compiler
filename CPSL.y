%{
#include <iostream>
#include <string>
#include <cstdio>
#define YYERROR_VERBOSE 1

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern int lineNum;
void yyerror(const char *str);
%}

%union {
  int intVal;
  std::string *strVal;
}

%token <intVal> DECIMAL_SYM ZERO_SYM
%token <strVal> CHAR_SYM STRING_SYM OCTAL_SYM HEX_SYM IDENTIFIER_SYM

%token ARRAY_SYM BEGIN_SYM CHR_SYM CONST_SYM DO_SYM DOWNTO_SYM ELSE_SYM ELSEIF_SYM END_SYM FOR_SYM FORWARD_SYM FUNCTION_SYM IF_SYM OF_SYM ORD_SYM PRED_SYM PROCEDURE_SYM READ_SYM RECORD_SYM REPEAT_SYM RETURN_SYM STOP_SYM SUCC_SYM THEN_SYM TO_SYM TYPE_SYM UNTIL_SYM VAR_SYM WHILE_SYM WRITE_SYM DOT_SYM COMMA_SYM COLON_SYM SEMICOLON_SYM LPAREN_SYM RPAREN_SYM LBRACK_SYM RBRACK_SYM

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

program: ConstantDecl TypeDecl VarDecl ProFuncDecl Block DOT_SYM
  ;
ConstantDecl:
  | CONST_SYM MoreConst
  ;
MoreConst:
  | IDENTIFIER_SYM EQUALS_SYM ConstExpression SEMICOLON_SYM MoreConst
  ;
TypeDecl:
  | TYPE_SYM MoreType
  ;
MoreType:
  | IDENTIFIER_SYM EQUALS_SYM Type SEMICOLON_SYM MoreType
  ;
Type: SimpleType
  | RecordType
  | ArrayType
  ;
SimpleType: IDENTIFIER_SYM
  ;
RecordType: RECORD_SYM Vars END_SYM
  ;
Vars:
  | IdentList COLON_SYM Type SEMICOLON_SYM Vars
  ;
ArrayType: ARRAY_SYM LBRACK_SYM ConstExpression COLON_SYM ConstExpression RBRACK_SYM OF_SYM Type
  ;
IdentList: IDENTIFIER_SYM MoreIdents
  ;
MoreIdents:
  | COMMA_SYM IDENTIFIER_SYM
  ;
VarDecl:
  | VAR_SYM IdentList COLON_SYM Type SEMICOLON_SYM Vars
  ;
ProFuncDecl:
  | ProFuncDecl ProcedureDecl
  | ProFuncDecl FunctionDecl
  ;
ProcedureDecl: PROCEDURE_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM SEMICOLON_SYM FORWARD_SYM SEMICOLON_SYM
  | PROCEDURE_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM SEMICOLON_SYM Body SEMICOLON_SYM
  ;
FunctionDecl: FUNCTION_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM COLON_SYM Type SEMICOLON_SYM FORWARD_SYM SEMICOLON_SYM
  | FUNCTION_SYM IDENTIFIER_SYM LPAREN_SYM FormalParameters RPAREN_SYM COLON_SYM Type SEMICOLON_SYM Body SEMICOLON_SYM
  ;
FormalParameters:
  | IdentList COLON_SYM Type MoreParams
  | VAR_SYM IdentList COLON_SYM Type MoreParams
  ;
MoreParams:
  | SEMICOLON_SYM IdentList COLON_SYM Type MoreParams
  | SEMICOLON_SYM VAR_SYM IdentList COLON_SYM Type MoreParams
  ;
Body: ConstantDecl TypeDecl VarDecl Block
  ;
Block: BEGIN_SYM StatementSequence END_SYM
  ;
StatementSequence: Statement MoreStatements
  ;
MoreStatements:
  | SEMICOLON_SYM Statement MoreStatements
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
LValue: IDENTIFIER_SYM Sublval
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
  | DECIMAL_SYM
  | OCTAL_SYM
  | HEX_SYM
  | ZERO_SYM
  | CHAR_SYM
  | STRING_SYM
  ;
ConstExpression: ConstPrim
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
  | NOT_SYM ConstExpression
  | SUB_SYM ConstExpression %prec UNARYMINUS_SYM
  | LPAREN_SYM ConstExpression RPAREN_SYM
  ;  
ConstPrim: DECIMAL_SYM
  | OCTAL_SYM
  | HEX_SYM
  | ZERO_SYM
  | CHAR_SYM
  | STRING_SYM
  | IDENTIFIER_SYM
  ;
Arguments: 
  | Expression MoreArgs
  ;
MoreArgs:
  | COMMA_SYM Expression MoreArgs
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
ReadStatement: READ_SYM LPAREN_SYM LValue MoreLVals RPAREN_SYM
  ;
MoreLVals: 
  | COMMA_SYM LValue MoreLVals
  ;
WriteStatement: WRITE_SYM LPAREN_SYM Expression MoreArgs RPAREN_SYM
  ;
ProcedureCall: IDENTIFIER_SYM LPAREN_SYM Expression MoreArgs RPAREN_SYM
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
}
