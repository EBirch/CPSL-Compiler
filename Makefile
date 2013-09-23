all: lex.out

lex.yy.c: CPSL.lex
	flex -f CPSL.lex

CPSL.tab.c: CPSL.y
	bison -d CPSL.y

lex.out: lex.yy.c CPSL.tab.c
	g++ lex.yy.c CPSL.tab.c -o lex.out

clean:
	rm lex.yy.c CPSL.tab.h CPSL.tab.c lex.out
	make