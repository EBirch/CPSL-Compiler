all: lex.out

lex.yy.c: CPSL.lex
	flex -f CPSL.lex

CPSL.tab.c: CPSL.y
	bison -vd CPSL.y

lex.out: lex.yy.c CPSL.tab.c symboltable.cpp symboltable.hpp
	g++ -std=c++11 -g lex.yy.c CPSL.tab.c symboltable.cpp -o lex.out

clean:
	rm lex.yy.c CPSL.tab.h CPSL.tab.c lex.out
	make
