#include "symboltable.hpp"
extern bool verbose;

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
,location(std::string("__stringConstLabel"+std::to_string(SymbolTable::getInstance()->labels++)))
{};

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
  SymbolTable::getInstance()->offset.back()+=type.size;
};

void Var::print(){
  std::cout<<"Var "<<name<<" of type "<<type->name<<", location:"<<location<<std::endl;
};

Function::Function(std::string name, Type returnType, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined):Symbol(name)
,defined(defined)
,funcType(function)
,typeList(typeList)
,location("__"+name)
,returnType(std::make_shared<Type>(returnType)){
  this->name=name;
};

Function::Function(std::string name, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined):Symbol(name)
,defined(defined)
,funcType(procedure)
,typeList(typeList)
,location("__"+name){
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
  std::cout<<")"<<((funcType==Function::function)?("->"+returnType->name):(""))<<", location:"<<location<<std::endl;
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
{
  offset.resize(2);
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
