build:
	flex phase1_3.l
	yacc -v -d phase1_3.y
	gcc y.tab.c -o run.out

clean:
	rm -f y.tab.c y.tab.h y.output lex.yy.c a.out run.out

all: clean build