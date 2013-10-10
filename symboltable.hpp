#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>
extern void yyerror(const char *str);

#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

class Symbol{
  public:
    std::string name;
    Symbol(std::string name):name(name)
    {};
    virtual void print();
    virtual bool isType();
};

class Type:public Symbol{
  public:
    int size;
    Type(std::string name, int size);
    void print();
    bool isType();
};

class Const:public Symbol{
  public:
    enum ConstType{
      intType,
      charType,
      stringType,
      booleanType,
      identType
    };
    ConstType type;
    int numVal;
    char charVal;
    std::string strVal;
    bool boolVal;
    int getIntVal();
    Const(int numVal, std::string name="");
    Const(char charVal, std::string name="");
    Const(std::string strVal, std::string name="");
    Const(bool boolVal, std::string name="");
    Const(std::string name);
    void print();
};

class Var:public Symbol{
  public:
    std::shared_ptr<Type> type;
    Var(Type type, std::string name="");
    void print();
};

class Function:public Symbol{
  public:
    enum FunctionType{
      function,
      procedure
    };
    std::string name;
    std::shared_ptr<Type> returnType;
    std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList;
    bool defined;
    FunctionType funcType;
    Function(std::string name, Type returnType, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined=false);
    Function(std::string name, std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, bool defined=false);
    void print();
};

class Array:public Type{
  public:
    int lower;
    int upper;
    std::shared_ptr<Type> type;
    Array(Type* type, Const lower, Const upper, std::string name="");
    void print();
    bool isType(){
      return true;
    };
};

//TODO: typelist use std::map<std::string(member name), std::pair<std::shared_ptr<Type>, int(offset)>>
class Record:public Type{
  public:
    std::map<std::string, std::pair<std::shared_ptr<Type>, int>> layout;
    Record(std::vector<std::pair<std::vector<std::string>, std::shared_ptr<Type>>> typeList, std::string name=""):Type(name, 0)
    {};
    void print(){
      std::cout<<"TODO";
    };
    bool isType(){
      return true;
    };
};

class Simple:public Type{
  public:
    enum simpleType{
      unknown,
      integer,
      boolean,
      character,
      string,
      trueVal,
      falseVal
    };
    simpleType simType;
    std::shared_ptr<Type> type;
    Simple(simpleType simType=unknown, std::string name="");
    void print();
    bool isType(){
      return true;
    };
};

class SymbolTable{
  public:
    std::vector<std::map<std::string, std::shared_ptr<Symbol>>> tables;
    static std::shared_ptr<SymbolTable> instance;
    static std::shared_ptr<SymbolTable> getInstance();
    void pushScope(Function funcName);
    void popScope();
    void addFunction(std::string name, Function func, bool forward=false);
    void addSymbol(std::string name, Symbol sym, bool init=false);
    void addSymbol(std::string name, Var var, bool init=false);
    void addSymbol(std::string name, Simple simple, bool init=false);
    void addSymbol(std::string name, Record record, bool init=false);
    void addSymbol(std::string name, Array array, bool init=false);
    void addSymbol(std::string name, Const constant, bool init=false);
    void addSymbol(std::string name, Type type, bool init=false);
    bool lookup(std::string name);
    void checkType(std::string name);
    std::shared_ptr<Symbol> getSymbol(std::string name);
  private:
    SymbolTable();
};

#endif
