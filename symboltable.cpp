#include <fstream>
#include "symboltable.hpp"
extern bool verbose;
extern std::fstream emit;

Type::Type(std::string name, int size, TypeType typeType):Symbol(name)
,size(size)
,typeType(typeType)
{};

void Type::print(){
  std::cout<<"This shouldn't happen\n";
};

int Const::getIntVal(){
  if(type==intType){
    return numVal;
  }
  if(type==identType){
    auto ident=*(dynamic_cast<Const*>(SymbolTable::getInstance()->getSymbol(name).get()));
    if(ident.type==intType){
      return ident.numVal;
    }
  }
  yyerror("Array bound not an int val\n");
};

Const::Const(int numVal, std::string name):Symbol(name)
,numVal(numVal)
,type(intType)
{};

Const::Const(char charVal, std::string name):Symbol(name)
,charVal(charVal)
,type(charType)
{};

Const::Const(std::string strVal, std::string name):Symbol(name)
,strVal(strVal)
,type(stringType)
,location(std::string("__stringConstLabel"+std::to_string(SymbolTable::getInstance()->labels++))){
  SymbolTable::getInstance()->stringConsts.push_back(*this);
};

Const::Const(bool boolVal, std::string name):Symbol(name)
,boolVal(boolVal)
,type(booleanType)
{};

Const::Const(std::string name, ConstType type):Symbol(name)
,type(type)
{};

void Const::print(){
  std::cout<<"Const "<<name;
  switch(type){
    case intType: std::cout<<": "<<numVal<<std::endl; break;
    case charType: std::cout<<": "<<charVal<<std::endl; break;
    case stringType: std::cout<<": "<<strVal<<", location:"<<location<<std::endl; break;
    case booleanType: std::cout<<": "<<((boolVal)?"True":"False")<<std::endl; break;
    case identType: std::cout<<std::endl; break;
  };
};

Var::Var(Type type, int location, std::string name):Symbol(name)
,type(std::make_shared<Type>(type))
,location(location){
};

void Var::print(){
  std::cout<<"Var "<<name<<" of type "<<type->name<<", location:"<<location<<std::endl;
};

Function::Function(std::string name, Type returnType, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined):Symbol(name)
,defined(defined)
,funcType(function)
,typeList(typeList)
,location("__"+name)
,offset(0)
,returnType(std::make_shared<Type>(returnType)){
  this->name=name;
};

Function::Function(std::string name, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined):Symbol(name)
,defined(defined)
,funcType(procedure)
,typeList(typeList)
,location("__"+name)
,offset(0){
  this->name=name;
};

void Function::print(){
  std::cout<<((funcType==Function::function)?("Function: "):("Procedure: "))<<name<<"(";
  for(int i=0;i<typeList.size();++i){
    for(int j=0;j<typeList[i].first.size();++j){
      std::cout<<typeList[i].first[j];
      if(j==typeList[i].first.size()-1){
        continue;
      }
      std::cout<<", ";
    }
    std::cout<<": "<<typeList[i].second->name;
    if(i==typeList.size()-1){
        continue;
      }
    std::cout<<"; ";
  }
  std::cout<<")"<<((funcType==Function::function)?("->"+returnType->name):(""))<<", location:"<<location<<", offset:"<<offset<<std::endl;
};

Array::Array(Type *type, Const lower, Const upper, std::string name):Type(name, type->size*(upper.getIntVal()-lower.getIntVal()), Type::array)
,lower(lower.getIntVal())
,upper(upper.getIntVal())
,type(std::make_shared<Type>(*type))
{
  if(this->upper<=this->lower){
    yyerror("Invalid array bounds");
  }
  this->name="Array["+std::to_string(this->lower)+":"+std::to_string(this->upper)+"] of "+type->name;
};

void Array::print(){
  std::cout<<"Type "<<name<<": Array "<<lower<<" to "<<upper<<" of "<<type->name<<", size:"<<size<<"\n";
};

Simple::Simple(simpleType simType, std::string name):Type(name, 4)
,simType(simType){};

void Simple::print(){
  std::cout<<"Type "<<name<<" of simple type ";
  switch(simType){
    case integer: std::cout<<"integer"<<std::endl; break;
    case boolean: std::cout<<"boolean"<<std::endl; break;
    case character: std::cout<<"char"<<std::endl; break;
    case string: std::cout<<"string"<<std::endl; break;
  }
};

Record::Record(std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, std::string name):Type(name, 0, Type::record){
  int offset=0;
  std::for_each(typeList.begin(), typeList.end(),
    [&](std::pair<std::vector<std::string>, std::shared_ptr<Type>> val){
      std::for_each(val.first.begin(), val.first.end(),
        [&](std::string name){
          layout.insert(std::make_pair(name, std::make_pair(val.second, offset)));
          offset+=val.second->size;
        });
    });
  this->size=offset;
};

std::shared_ptr<SymbolTable> SymbolTable::getInstance(){
  if(!instance){
    std::shared_ptr<SymbolTable> inst(new SymbolTable());
    instance=inst;
  }
  return instance;
};

void Record::print(){
  std::cout<<"Type "<<name<<": Record {";
  std::for_each(layout.begin(), layout.end(),
    [&](std::pair<std::string, std::pair<std::shared_ptr<Type>, int>> val){
      std::cout<<val.first<<":"<<val.second.first->name<<" at offset "<<val.second.second<<"; ";
    });
  std::cout<<"}"<<std::endl;
};

void SymbolTable::pushScope(Function funcName){
  std::map<std::string, std::shared_ptr<Symbol>> temp;
  std::shared_ptr<Function> tempFunc=std::make_shared<Function>(funcName);
  tables.push_back(temp);
  offset.push_back(0);
  for(int i=0;i<tempFunc->typeList.size();++i){
    for(int j=0;j<tempFunc->typeList[i].first.size();++j){
      Var temp(*tempFunc->typeList[i].second, offset.back(), tempFunc->typeList[i].first[j]);
      SymbolTable::getInstance()->offset.back()+=4;
      addSymbol(tempFunc->typeList[i].first[j], temp, true);
    }
  }
};

void SymbolTable::popScope(){
  if(verbose){
    std::for_each(tables.back().begin(), tables.back().end(), 
      [&](std::pair<std::string, std::shared_ptr<Symbol>> val)
      {
        val.second->print();
      });
    std::cout<<std::endl<<std::endl;
  }
  tables.pop_back();
  offset.pop_back();
};

void SymbolTable::addFunction(std::string name, Function func, bool forward){
  if(tables.back().find(name)!=tables.back().end()){
    if(auto tempFunc=dynamic_cast<Function*>(getSymbol(name).get())){
      if(tempFunc->defined||forward){
        yyerror("Function already defined\n");
      }
      tempFunc->defined=true;
    }
    else{
      yyerror("Redeclaring symbol");
    }
  }
  func.offset=SymbolTable::getInstance()->offset.back();
  tables.back().insert(std::make_pair(name, std::make_shared<Function>(func)));
};

bool SymbolTable::lookup(std::string name){
  bool returnVal=false;
  std::for_each(tables.begin(), tables.end(),
    [&](std::map<std::string, std::shared_ptr<Symbol>> map){
      returnVal=returnVal||(map.find(name)!=map.end());
    });
  return returnVal;
};

std::shared_ptr<Symbol> SymbolTable::getSymbol(std::string name){
  for(int i=tables.size()-1;i>=0;--i){
    if(tables[i].find(name)!=tables[i].end()){
      auto temp=tables[i].at(name);
      return temp;
    }
  }
  yyerror("Symbol not found");
};

bool Type::isType(){
  return true;
};

bool Symbol::isType(){
  return false;
};

void Symbol::print(){
  std::cout<<"symbol\n";
};

void SymbolTable::checkType(std::string name){
  if(lookup(name)){
    if(getSymbol(name)->isType()){
      return;
    }
  }
  yyerror("Type is undefined\n");
};

SymbolTable::SymbolTable():tables()
,labels(0)
,controlLabels(0)
,ifLabels(0)
,controlStack()
,ifStack()
,stringConsts(){
  offset.resize(2);
  registers.resize(18);
  std::fill(registers.begin(), registers.end(), true);
  std::map<std::string, std::shared_ptr<Symbol>> temp, mainScope;
  temp.insert(std::make_pair("integer", std::make_shared<Simple>(Simple::integer, "integer")));
  temp.insert(std::make_pair("INTEGER", std::make_shared<Simple>(Simple::integer, "INTEGER")));
  temp.insert(std::make_pair("char", std::make_shared<Simple>(Simple::character, "char")));
  temp.insert(std::make_pair("CHAR", std::make_shared<Simple>(Simple::character, "CHAR")));
  temp.insert(std::make_pair("boolean", std::make_shared<Simple>(Simple::boolean, "boolean")));
  temp.insert(std::make_pair("BOOLEAN", std::make_shared<Simple>(Simple::boolean, "BOOLEAN")));
  temp.insert(std::make_pair("string", std::make_shared<Simple>(Simple::string, "string")));
  temp.insert(std::make_pair("STRING", std::make_shared<Simple>(Simple::string, "STRING")));
  temp.insert(std::make_pair("true", std::make_shared<Const>(true, "true")));
  temp.insert(std::make_pair("TRUE", std::make_shared<Const>(true, "TRUE")));
  temp.insert(std::make_pair("false", std::make_shared<Const>(false, "false")));
  temp.insert(std::make_pair("FALSE", std::make_shared<Const>(false, "FALSE")));
  tables.push_back(temp);
  tables.push_back(mainScope);
};

Const* negative(Const val){
  if(!checkIdent(val, Const::intType)){
    yyerror("Invalid operator on const expression");
  }
  return new Const(-val.numVal);
}

Const* notOp(Const val){
  if(!checkIdent(val, Const::booleanType)){
    yyerror("Invalid operator on const expression");
  }
  return new Const(!val.boolVal);
}

Const* mod(Const left, Const right){
  if((left.type!=Const::intType)||(!sameType(left, right))){
    yyerror("Invalid operator on const expression");
  }
  return new Const(left.numVal%right.numVal);  
};

Const* div(Const left, Const right){
  if((left.type!=Const::intType)||(!sameType(left, right))){
    yyerror("Invalid operator on const expression");
  }
  return new Const(left.numVal/right.numVal);  
};

Const* mult(Const left, Const right){
  if((left.type!=Const::intType)||(!sameType(left, right))){
    yyerror("Invalid operator on const expression");
  }
  return new Const(left.numVal*right.numVal);  
};

Const* sub(Const left, Const right){
  if((left.type!=Const::intType)||(!sameType(left, right))){
    yyerror("Invalid operator on const expression");
  }
  return new Const(left.numVal-right.numVal);  
};

Const* add(Const left, Const right){
  if((left.type!=Const::intType)||(!sameType(left, right))){
    yyerror("Invalid operator on const expression");
  }
  return new Const(left.numVal+right.numVal);  
};

Const* gt(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::intType:return new Const(left.numVal>right.numVal);
    case Const::charType:return new Const(left.charVal>right.charVal);
    case Const::stringType:return new Const(left.strVal>right.strVal);
  }
}

Const* lt(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::intType:return new Const(left.numVal<right.numVal);
    case Const::charType:return new Const(left.charVal<right.charVal);
    case Const::stringType:return new Const(left.strVal<right.strVal);
  }
}

Const* gte(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::intType:return new Const(left.numVal>=right.numVal);
    case Const::charType:return new Const(left.charVal>=right.charVal);
    case Const::stringType:return new Const(left.strVal>=right.strVal);
  }
}

Const* lte(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::intType:return new Const(left.numVal<=right.numVal);
    case Const::charType:return new Const(left.charVal<=right.charVal);
    case Const::stringType:return new Const(left.strVal<=right.strVal);
  }
}

Const* neq(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::intType:return new Const(left.numVal!=right.numVal);
    case Const::charType:return new Const(left.charVal!=right.charVal);
    case Const::stringType:return new Const(left.strVal!=right.strVal);
  }
}

Const* eq(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::intType:return new Const(left.numVal==right.numVal);
    case Const::charType:return new Const(left.charVal==right.charVal);
    case Const::stringType:return new Const(left.strVal==right.strVal);
  }
}

Const* andOp(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::booleanType:return new Const(left.boolVal&&right.boolVal);
    default: yyerror("Invalid operator on const expression");
  }
}

Const* orOp(Const left, Const right){
  if(!sameType(left, right)){
    yyerror("Operands not of same type");
  }
  switch(left.type){
    case Const::booleanType:return new Const(left.boolVal||right.boolVal);
    default: yyerror("Invalid operator on const expression");
  }
}

bool sameType(Const &left, Const &right){
  if(left.type==Const::identType){
    left=*(dynamic_cast<Const*>(SymbolTable::getInstance()->getSymbol(left.name).get()));
  }
  if(right.type==Const::identType){
    right=*(dynamic_cast<Const*>(SymbolTable::getInstance()->getSymbol(right.name).get()));
  }
  return left.type==right.type;
}

bool checkIdent(Const &val, Const::ConstType type){
  if(val.type==Const::identType){
    val=*(dynamic_cast<Const*>(SymbolTable::getInstance()->getSymbol(val.name).get()));
  }
  return val.type==type;
}

int getSize(std::string val){
  auto temp=dynamic_cast<Var*>(SymbolTable::getInstance()->getSymbol(val).get());
  if(!temp){
    yyerror("Var cast failed\n");
  }
  return temp->type->size;
}

Expression *getLval(std::vector<Expression> exprList){
  auto tempVar=dynamic_cast<Var*>(SymbolTable::getInstance()->getSymbol(exprList[0].getVal<std::string>()).get());
  if(!tempVar){
    // auto tempConst=dynamic_cast<Const*>(SymbolTable::getInstance()->getSymbol(exprList[0].getVal<std::string>()).get());
    // if(!tempConst){ 
      yyerror("Var cast failed\n");
    // }
    //TODO typechecking and get const vals

  }
  int rootLoc=tempVar->location;
  int lastLower;
  // auto lastType=dynamic_cast<Type*>(tempVar->type.get());
  auto lastType=dynamic_cast<Type*>(SymbolTable::getInstance()->getSymbol(tempVar->type->name).get());
  // if(lastType->typeType==Type::array){
  //   lastLower=(dynamic_cast<Array*>(lastType))->lower;
  //   lastType=dynamic_cast<Type*>(dynamic_cast<Array*>(lastType)->type.get());
  // }
  if(exprList.size()>1){
    bool rootInt=true;
    int locReg=SymbolTable::getInstance()->getReg();
    int tempReg=SymbolTable::getInstance()->getReg(), tempRegMult=SymbolTable::getInstance()->getReg();
    emit<<"move $"<<locReg<<", $zero"<<std::endl;
    // std::for_each(exprList.begin()+1, exprList.end(),
      // [&](Expression expr){
    for(int i=1;i<exprList.size();++i){
    if(lastType->typeType==Type::array){
      lastLower=(dynamic_cast<Array*>(SymbolTable::getInstance()->getSymbol(lastType->name).get()))->lower;
      lastType=dynamic_cast<Type*>(dynamic_cast<Array*>(SymbolTable::getInstance()->getSymbol(lastType->name).get())->type.get());
    }
        if(exprList[i].type==Expression::intType){
          if(exprList[i].lit){
            rootLoc+=((exprList[i].getVal<int>()-lastLower)*lastType->size);
          }
          else{
            emit<<"lw $"<<tempReg<<", "<<exprList[i].getVal<int>()<<"($fp)"<<std::endl;
            emit<<"addi $"<<tempReg<<", $"<<tempReg<<", "<<(-lastLower)<<std::endl;
            emit<<"li $"<<tempRegMult<<", "<<lastType->size<<std::endl;
            emit<<"mult $"<<tempReg<<", $"<<tempRegMult<<std::endl;
            emit<<"mflo $"<<tempReg<<std::endl;
            emit<<"add $"<<locReg<<", $"<<locReg<<", $"<<tempReg<<std::endl;
          }
        }
        else if(exprList[i].type==Expression::stringType){
          auto tempRec=(dynamic_cast<Record*>(lastType));
          if(tempRec->layout.find(exprList[i].getVal<std::string>())==tempRec->layout.end()){
            yyerror("Invalid lvalue expression");
          }
          auto mem=tempRec->layout.at(exprList[i].getVal<std::string>());
          rootLoc+=mem.second;
          lastType=dynamic_cast<Type*>(SymbolTable::getInstance()->getSymbol(mem.first->name).get());
        }
        else{
          yyerror("Invalid lvalue expression");
        }
      // });
      }
    }
  auto simpTemp=(dynamic_cast<Simple*>(SymbolTable::getInstance()->getSymbol(lastType->name).get()));
  if(!simpTemp){
    auto what=dynamic_cast<Array*>(SymbolTable::getInstance()->getSymbol(lastType->name).get());
    simpTemp=(dynamic_cast<Simple*>(SymbolTable::getInstance()->getSymbol(what->type->name).get()));
  }
  return new Expression(rootLoc, Expression::intType, false, (simpTemp->simType==Simple::character||simpTemp->simType==Simple::string), true);
}

int SymbolTable::getReg(){
  for(int i=0;i<registers.size();++i){
    if(registers[i]){
      registers[i]=false;
      return i+8;
    }
  }
  yyerror("Out of registers\n");
}

void SymbolTable::clearReg(){
  std::fill(registers.begin(), registers.end(), true);
}

// void SymbolTable::lockReg(int reg){
//   registers[reg-8]=false;
// }

void evalBoilerPlate(int &leftReg, int &rightReg, Expression *left, Expression* right){
  leftReg=((left->type==Expression::reg)?(left->getVal<int>()):(SymbolTable::getInstance()->getReg()));
  rightReg=((right->type==Expression::reg)?(right->getVal<int>()):(SymbolTable::getInstance()->getReg()));
  if(left->type!=Expression::reg){
    emit<<((left->lit)?("li $"+std::to_string(leftReg)+", "+std::to_string(left->getVal<int>())):("lw $")+std::to_string(leftReg)+", "+std::to_string(left->getVal<int>())+"($fp)")<<std::endl;
  }
  if(right->type!=Expression::reg){
    emit<<((right->lit)?("li $"+std::to_string(rightReg)+", "+std::to_string(right->getVal<int>())):("lw $")+std::to_string(rightReg)+", "+std::to_string(right->getVal<int>())+"($fp)")<<std::endl;
  }
}

Expression *eval(Expression *left, Expression *right, std::string op)
{
  if(left->lit&&right->lit){
    return foldExpr(left, right, op);
  }
  int leftReg, rightReg;
  int dest=SymbolTable::getInstance()->getReg();
  evalBoilerPlate(leftReg, rightReg, left, right);
  emit<<op<<" $"<<dest<<", $"<<leftReg<<", $"<<rightReg<<std::endl;
  return new Expression(dest, Expression::reg);
} 

Expression *evalSpec(Expression *left, Expression *right, std::string op){
  if(left->lit&&right->lit){
    return foldExpr(left, right, op);
  }
  int leftReg, rightReg;
  evalBoilerPlate(leftReg, rightReg, left, right);
  emit<<((op=="mod"||op=="div")?("div "):("mult "))<<"$"<<leftReg<<", $"<<rightReg<<std::endl;
  emit<<((op=="mod")?("mfhi "):("mflo "))<<"$"<<leftReg<<std::endl;//TODO dest?
  return new Expression(leftReg, Expression::reg);
}

Expression *evalUnary(Expression *expr, std::string op){
  if(expr->lit){
    return foldExprUnary(expr, op);
  }
  int reg=((expr->type==Expression::reg)?(expr->getVal<int>()):(SymbolTable::getInstance()->getReg()));
  int dest=SymbolTable::getInstance()->getReg();
  if(expr->type!=Expression::reg){
    emit<<((expr->lit)?("li $"+std::to_string(reg)+", "+std::to_string(expr->getVal<int>())):("lw $")+std::to_string(reg)+", "+std::to_string(expr->getVal<int>())+"($fp)")<<std::endl;
  }
  emit<<op<<" $"<<dest<<", $"<<reg<<std::endl;
  return new Expression(dest, Expression::reg);
}

Expression *foldExprUnary(Expression *expr, std::string op){
  if(op=="not"){
    return new Expression(!expr->getVal<bool>(), Expression::boolType, true);
  }
  if(op=="neg"){
    return new Expression(-expr->getVal<int>(), Expression::intType, true);
  }
}

Expression *foldExpr(Expression *left, Expression *right, std::string op){
  if(op=="mult"){
    return new Expression(left->getVal<int>()*right->getVal<int>(), Expression::intType, true);
  }
  if(op=="div"){
    return new Expression(left->getVal<int>()/right->getVal<int>(), Expression::intType, true);
  }
  if(op=="add"){
    return new Expression(left->getVal<int>()+right->getVal<int>(), Expression::intType, true);
  }
  if(op=="sub"){
    return new Expression(left->getVal<int>()-right->getVal<int>(), Expression::intType, true);
  }
  if(op=="mod"){
    return new Expression(left->getVal<int>()%right->getVal<int>(), Expression::intType, true);
  }
  if(op=="and"){
    return new Expression(left->getVal<bool>()&&right->getVal<bool>(), Expression::boolType, true);
  }
  if(op=="or"){
    return new Expression(left->getVal<bool>()||right->getVal<bool>(), Expression::boolType, true);
  }
  if(op=="seq"){
    return new Expression(left->getVal<int>()==right->getVal<int>(), Expression::boolType, true);
  }
  if(op=="sne"){
    return new Expression(left->getVal<int>()!=right->getVal<int>(), Expression::boolType, true);
  }
  if(op=="sge"){
    return new Expression(left->getVal<int>()>=right->getVal<int>(), Expression::boolType, true);
  }
  if(op=="sle"){
    return new Expression(left->getVal<int>()<=right->getVal<int>(), Expression::boolType, true);
  }
  if(op=="sgt"){
    return new Expression(left->getVal<int>()>right->getVal<int>(), Expression::boolType, true);
  }
  if(op=="slt"){
    return new Expression(left->getVal<int>()<right->getVal<int>(), Expression::boolType, true);
  }
}

void assign(Expression *lval, Expression *rval){
  int src=((rval->lit)?(SymbolTable::getInstance()->getReg()):(rval->getVal<int>()));
  if(rval->lit){
    emit<<"li $"<<src<<", "<<rval->getVal<int>()<<std::endl;
  }
  if(rval->ident){
    int reg=SymbolTable::getInstance()->getReg();
    emit<<"lw $"<<reg<<", "<<src<<"($fp)"<<std::endl;
    emit<<"sw $"<<reg<<", "<<lval->getVal<int>()<<"($fp)"<<std::endl;
  }
  else{
    emit<<"sw $"<<src<<", "<<lval->getVal<int>()<<"($fp)"<<std::endl;
  }
}

void write(std::vector<Expression> exprList){
  std::for_each(exprList.begin(), exprList.end(), 
    [&](Expression expr){
      if(expr.type==Expression::intType){
        if(expr.lit){
          emit<<"li $a0, "<<expr.getVal<int>()<<std::endl;
          emit<<"li $v0, 1"<<std::endl;
        }
        else if(expr.str){
          emit<<"lw $a0, "<<expr.getVal<int>()<<"($fp)"<<std::endl;
          emit<<"li $v0, 11"<<std::endl;
        }
        else{
          emit<<"lw $a0, "<<expr.getVal<int>()<<"($fp)"<<std::endl;
          emit<<"li $v0, 1"<<std::endl;
        }
      }
      if(expr.type==Expression::stringType){
        emit<<"la $a0, "<<expr.getVal<std::string>()<<std::endl;
        emit<<"li $v0, 4"<<std::endl;
      }
      if(expr.type==Expression::charType){
        emit<<"li $a0, "<<expr.getVal<int>()<<std::endl;
        emit<<"li $v0, 11"<<std::endl;
      }
      if(expr.type==Expression::reg){
        emit<<"move $a0, $"<<expr.getVal<int>()<<std::endl;
        emit<<"li $v0, 1"<<std::endl;
      }
      emit<<"syscall"<<std::endl;
    });
  // emit<<"la $a0, __newline"<<std::endl<<"li $v0, 4"<<std::endl<<"syscall"<<std::endl;
}

void SymbolTable::emitEnd(){
  emit<<"li $v0, 10"<<std::endl<<"syscall"<<std::endl;
  emit<<".data"<<std::endl<<"__newline: .asciiz \"\\n\""<<std::endl;
  if(stringConsts.size()>0){
    std::for_each(stringConsts.begin(), stringConsts.end(),
      [&](Const strConst){
        emit<<strConst.location<<": .asciiz "<<strConst.strVal<<std::endl;
      });
  }
}

void read(std::vector<Expression> exprList){
  std::for_each(exprList.begin(), exprList.end(), 
    [&](Expression expr){
      if(expr.str){
        emit<<"li $v0, 12"<<std::endl<<"syscall"<<std::endl<<"sw $v0, "<<expr.getVal<int>()<<"($fp)"<<std::endl;
      }
      else{
        emit<<"li $v0, 5"<<std::endl<<"syscall"<<std::endl<<"sw $v0, "<<expr.getVal<int>()<<"($fp)"<<std::endl;
      }
    });
}

void ifBegin(){
  SymbolTable::getInstance()->ifStack.push_back(0);
  SymbolTable::getInstance()->controlStack.push_back(SymbolTable::getInstance()->controlLabels++);
}

void controlBegin(){
  int labelCount=SymbolTable::getInstance()->controlLabels++;
  SymbolTable::getInstance()->controlStack.push_back(labelCount);
  emit<<"__controlStmt"<<labelCount<<": ";
}

void controlCheck(Expression *cond, bool val){
  int labelCount=SymbolTable::getInstance()->controlStack.back();
  int reg=((cond->type==Expression::reg)?(cond->getVal<int>()):(SymbolTable::getInstance()->getReg()));
  if(cond->type!=Expression::reg){
    emit<<((cond->lit)?("li $"+std::to_string(reg)+", "+std::to_string(cond->getVal<int>())):("lw $")+std::to_string(reg)+", "+std::to_string(cond->getVal<int>())+"($fp)")<<std::endl;
  }
  if(!val){
    emit<<"bne $"<<reg<<", $zero, __controlStmtAfter"<<labelCount<<std::endl;
  }
  else{
    emit<<"beq $"<<reg<<", $zero, __controlStmtAfter"<<labelCount<<std::endl;
  }
}

void repeatCheck(Expression *cond){
  int labelCount=SymbolTable::getInstance()->controlStack.back();
  int reg=((cond->type==Expression::reg)?(cond->getVal<int>()):(SymbolTable::getInstance()->getReg()));
  if(cond->type!=Expression::reg){
    emit<<((cond->lit)?("li $"+std::to_string(reg)+", "+std::to_string(cond->getVal<int>())):("lw $")+std::to_string(reg)+", "+std::to_string(cond->getVal<int>())+"($fp)")<<std::endl;
  }
  emit<<"beq $"<<reg<<", $zero, __controlStmt"<<labelCount<<std::endl;
  SymbolTable::getInstance()->controlStack.pop_back();
}

void controlEnd(){
  emit<<"j __controlStmt"<<SymbolTable::getInstance()->controlStack.back()<<std::endl;
  emit<<"__controlStmtAfter"<<SymbolTable::getInstance()->controlStack.back()<<": "<<std::endl;
  SymbolTable::getInstance()->controlStack.pop_back();
}

void ifBranch(Expression *cond){
  int ifCount=SymbolTable::getInstance()->ifStack.back();
  int controlCount=SymbolTable::getInstance()->controlStack.back();
  int reg=((cond->type==Expression::reg)?(cond->getVal<int>()):(SymbolTable::getInstance()->getReg()));
  if(cond->type!=Expression::reg){
    emit<<((cond->lit)?("li $"+std::to_string(reg)+", "+std::to_string(cond->getVal<int>())):("lw $")+std::to_string(reg)+", "+std::to_string(cond->getVal<int>())+"($fp)")<<std::endl;
  }
  emit<<"beq $"<<reg<<", $zero, __control"<<controlCount<<"IfStmt"<<ifCount<<std::endl;
}

void ifBranchEnd(){
  emit<<"j __controlStmtAfter"<<SymbolTable::getInstance()->controlStack.back()<<std::endl;
}

void endIf(){
  int ifCount=SymbolTable::getInstance()->ifStack.back();
  int controlCount=SymbolTable::getInstance()->controlStack.back();
  emit<<"__control"<<controlCount<<"IfStmt"<<ifCount<<": "<<std::endl;
  emit<<"__controlStmtAfter"<<controlCount<<": "<<std::endl;
  SymbolTable::getInstance()->controlStack.pop_back();
  SymbolTable::getInstance()->ifStack.pop_back();
}

void labelIfBranch(){
  int ifCount=SymbolTable::getInstance()->ifStack.back()++;
  int controlCount=SymbolTable::getInstance()->controlStack.back();
  emit<<"__control"<<controlCount<<"IfStmt"<<ifCount<<": "<<std::endl;
}

Expression *doFunc(std::string ident, std::vector<Expression> args){
  std::string label;
  std::vector<std::pair<int, int>> backupVars;
  if(!SymbolTable::getInstance()->lookup(ident)){
    yyerror("Procedure not defined\n");
  }
  auto tempFunc=dynamic_cast<Function*>(SymbolTable::getInstance()->getSymbol(ident).get());
  if(tempFunc){
    label=tempFunc->location;
  }
  else{
    yyerror("Function cast error");
  }
  int stackSpace=-8;
  for(int i=0;i<SymbolTable::getInstance()->registers.size();++i){
    if(!SymbolTable::getInstance()->registers[i]){
      SymbolTable::getInstance()->spillStack.push_back(std::make_pair(i+8, -stackSpace));
      stackSpace-=4;
    }
  }
  std::for_each(SymbolTable::getInstance()->tables.back().begin(), SymbolTable::getInstance()->tables.back().end(),
    [&](std::pair<std::string, std::shared_ptr<Symbol>> sym){
      auto temp=dynamic_cast<Var*>(sym.second.get());
      if(!temp){
        return;
      }
      backupVars.push_back(std::make_pair(temp->location, -stackSpace));
      stackSpace-=4;
    });
  int argMove=SymbolTable::getInstance()->getReg();
  emit<<"addi $sp, $sp, "<<stackSpace<<std::endl;
  emit<<"sw $ra, 0($sp)"<<std::endl;
  emit<<"sw $fp, 4($sp)"<<std::endl;
  for(int i=0;i<backupVars.size();++i){
    emit<<"lw $"<<argMove<<", "<<backupVars[i].first<<"($fp)"<<std::endl;
    emit<<"sw $"<<argMove<<", "<<backupVars[i].second<<"($sp)"<<std::endl;
  }
  emit<<"move $fp, $gp"<<std::endl;
  emit<<"addi $fp, $fp, "<<tempFunc->offset<<std::endl;
  int stackRecover=8;
  for(int i=0;i<args.size();++i){
    if(args[i].type==Expression::intType){
      if(args[i].lit){
        emit<<"li $"<<argMove<<", "<<args[i].getVal<int>()<<std::endl;
      }
      else{
        emit<<"lw $"<<argMove<<", "<<(args[i].getVal<int>()-tempFunc->offset)<<"($fp)"<<std::endl;
      }
    }
    if(args[i].type==Expression::reg){
      emit<<"move $"<<argMove<<", $"<<args[i].getVal<int>()<<std::endl;
    }
    emit<<"sw $"<<argMove<<", "<<(i*4)<<"($fp)"<<std::endl;
  }
  for(int i=0;i<SymbolTable::getInstance()->spillStack.size();++i){
    emit<<"sw $"<<SymbolTable::getInstance()->spillStack[i].first<<", "<<SymbolTable::getInstance()->spillStack[i].second<<"($sp)"<<std::endl;
  }
  emit<<"jal "<<label<<std::endl;
  emit<<"lw $ra, 0($sp)"<<std::endl;
  emit<<"lw $fp, 4($sp)"<<std::endl;
  int tempReg=SymbolTable::getInstance()->getReg();
  for(int i=0;i<backupVars.size();++i){
    emit<<"lw $"<<tempReg<<", "<<backupVars[i].second<<"($sp)"<<std::endl;
    emit<<"sw $"<<tempReg<<", "<<backupVars[i].first<<"($fp)"<<std::endl;
  }
  while(SymbolTable::getInstance()->spillStack.size()>0){
    emit<<"lw $"<<SymbolTable::getInstance()->spillStack.back().first<<", "<<SymbolTable::getInstance()->spillStack.back().second<<"($sp)"<<std::endl;
    SymbolTable::getInstance()->spillStack.pop_back();
  }
  emit<<"addi $sp, $sp, "<<(-stackSpace)<<std::endl;
  int reg=SymbolTable::getInstance()->getReg();
  if(tempFunc->funcType==Function::function){
    emit<<"move $"<<reg<<", $v0"<<std::endl;
  }
  return new Expression(reg, Expression::reg);
}

void doReturn(Expression *retVal){
  if(retVal->type==Expression::reg){
    emit<<"move $v0, $"<<retVal->getVal<int>()<<std::endl;
    emit<<"jr $ra"<<std::endl;
    return;
  }
  if(retVal->lit){
    emit<<"li $v0, "<<retVal->getVal<int>()<<std::endl;
  }
  else{
    emit<<"lw $v0, "<<retVal->getVal<int>()<<"($fp)"<<std::endl;
  }
  emit<<"jr $ra"<<std::endl;
}
