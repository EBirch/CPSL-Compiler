#include "symboltable.hpp"

Type::Type(std::string name, int size):Symbol(name)
,size(size)
{};

void Type::print(){
  // std::cout<<"Type "<<name<<", size: "<<size<<std::endl;
  std::cout<<"AUGH NO WHY\n";
};

int Const::getIntVal()
{
  //TODO throw error if not right type
  return numVal;
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
{};

Const::Const(bool boolVal, std::string name):Symbol(name)
,boolVal(boolVal)
,type(booleanType)
{};

Const::Const(std::string name):Symbol(name)
,type(identType)
{};

void Const::print(){
  std::cout<<"Const "<<name;
  switch(type){
    case intType: std::cout<<": "<<numVal<<std::endl; break;
    case charType: std::cout<<": "<<charVal<<std::endl; break;
    case stringType: std::cout<<": "<<strVal<<std::endl; break;
    case booleanType: std::cout<<": "<<((boolVal)?"True":"False")<<std::endl; break;
    case identType: std::cout<<std::endl; break;
  };
};

Var::Var(Type type, std::string name):Symbol(name)
,type(std::make_shared<Type>(type))
{};

void Var::print(){
  std::cout<<"Var "<<name<<" of type "<<type->name<<std::endl;
};

Function::Function(std::string name, Type returnType, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined):Symbol(name)
,defined(defined)
,funcType(function)
,typeList(typeList)
{};

Function::Function(std::string name, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined):Symbol(name)
,defined(defined)
,funcType(procedure)
,typeList(typeList)
{};

void Function::print(){//TODO
  std::cout<<name<<std::endl;
};

Array::Array(Type *type, Const lower, Const upper, std::string name):Type(name, type->size*(upper.getIntVal()-lower.getIntVal()))
,lower(lower.getIntVal())
,upper(upper.getIntVal())
,type(std::make_shared<Type>(*type))
{
  this->name="Array["+std::to_string(this->lower)+":"+std::to_string(this->upper)+"] of "+type->name;
};

void Array::print(){
  std::cout<<"Array "<<lower<<" to "<<upper<<" of "<<type->name<<" size:"<<size<<"\n";
};

Simple::Simple(simpleType simType, std::string name):Type(name, 4)//string size?
,simType(simType){
  // if(SymbolTable::getInstance()->lookUp(name)){
  //   yyerror("Type already defined");
  // }
};

void Simple::print(){
  std::cout<<"Type "<<name<<" of simple type ";
  switch(simType){
    case integer: std::cout<<"integer"<<std::endl; break;
    case boolean: std::cout<<"boolean"<<std::endl; break;
    case character: std::cout<<"char"<<std::endl; break;
    case string: std::cout<<"string"<<std::endl; break;
    case trueVal: std::cout<<""<<std::endl; break;//TODO get rid of these
    case falseVal: std::cout<<""<<std::endl; break;
    case unknown: std::cout<<"unknown"<<std::endl; break;
  }
};

std::shared_ptr<SymbolTable> SymbolTable::getInstance(){
  if(!instance){
    std::shared_ptr<SymbolTable> inst(new SymbolTable());
    instance=inst;
  }
  return instance;
};

void SymbolTable::pushScope(Function funcName){
  //check for forward declaration
  std::map<std::string, std::shared_ptr<Symbol>> temp;
  // auto mid=tables.back().at(funcName);
  // std::cout<<mid->name;
  // auto comeon=dynamic_cast<Function*>(mid.get());
  // auto whatev=std::static_pointer_cast<Function>(mid);
  // std::shared_ptr<Function> tempFunc(std::dynamic_pointer_cast<Function>(tables.back().at(funcName)));
  std::shared_ptr<Function> tempFunc=std::make_shared<Function>(funcName);
  // std::shared_ptr<Function> tempFunc(dynamic_cast<Function*>(tables.back().at(funcName).get()));
  // tempFunc->print();
  // std::for_each(tempFunc->typeList.begin(), tempFunc->typeList.end(),
  //   [&](std::pair<std::vector<std::string>, std::shared_ptr<Type>> val){
  //     std::for_each(val.first.begin(), val.first.end(),
  //       [&](std::string ident){
  //         temp.insert(std::pair<std::string, std::shared_ptr<Symbol>>(ident, val.second));
  //       });
  //   });
  // std::cout<<tables.size()<<std::endl;
  // std::cout<<tempFunc->typeList.size()<<std::endl;
  for(int i=0;i<tempFunc->typeList.size();++i){
    // std::cout<<tempFunc->typeList[i].first.size()<<std::endl;
    for(int j=0;j<tempFunc->typeList[i].first.size();++j){
      // std::cout<<tempFunc->typeList[i].first[j]<<std::endl;
      temp.insert(std::pair<std::string, std::shared_ptr<Symbol>>(tempFunc->typeList[i].first[j], tempFunc->typeList[i].second));
    }
  }
  tables.push_back(temp);
};

void SymbolTable::popScope(){
  // std::cout<<tables.back().size()<<std::endl;
  // for(int i=0;i<tables.back().size();++i{
  //   tables.back()[i].second->print();
  // }
  std::for_each(tables.back().begin(), tables.back().end(), 
    [&](std::pair<std::string, std::shared_ptr<Symbol>> val)
    {
      // std::cout<<val.first<<std::endl;
      // std::cout<<val.first;
      val.second->print();
    });
  std::cout<<std::endl<<std::endl;
  tables.pop_back();
};

void SymbolTable::addFunction(std::string name, Function func, bool forward){
  if(tables.back().find(name)!=tables.back().end()){
    if(func.defined||forward){
      yyerror("Function already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Symbol>(func)));
};

void SymbolTable::addSymbol(std::string name, Symbol sym, bool init){
  if(tables.back().find(name)!=tables.back().end()){
    if(init){
      std::cout<<name<<std::endl;
      yyerror("Symbol already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Symbol>(sym)));
};

void SymbolTable::addSymbol(std::string name, Var var, bool init){
  if(tables.back().find(name)!=tables.back().end()){
    if(init){
      std::cout<<name<<std::endl;
      yyerror("Var already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Var>(var)));
};

void SymbolTable::addSymbol(std::string name, Simple simple, bool init){
  if(tables.back().find(name)!=tables.back().end()){
    if(init){
      std::cout<<name<<std::endl;
      yyerror("Simple type already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Simple>(simple)));
};

void SymbolTable::addSymbol(std::string name, Record record, bool init){
  if(tables.back().find(name)!=tables.back().end()){
    if(init){
      std::cout<<name<<std::endl;
      yyerror("Record already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Record>(record)));
};

void SymbolTable::addSymbol(std::string name, Array array, bool init){
  if(tables.back().find(name)!=tables.back().end()){
    if(init){
      std::cout<<name<<std::endl;
      yyerror("Array already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Array>(array)));
};

void SymbolTable::addSymbol(std::string name, Const constant, bool init){
  if(tables.back().find(name)!=tables.back().end()){
    if(init){
      std::cout<<name<<std::endl;
      yyerror("Const already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Const>(constant)));
};

void SymbolTable::addSymbol(std::string name, Type type, bool init){
  if(tables.back().find(name)!=tables.back().end()){
    if(init){
      std::cout<<name<<std::endl;
      yyerror("Type already defined\n");
    }
  }
  tables.back().insert(std::make_pair(name, std::make_shared<Type>(type)));
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
{
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
