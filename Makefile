myshell2:	parse.l parse.y abstract_syntax_tree.h
			bison -d parse.y
			flex -oparse.lex.c parse.l 
			gcc -Wall -o myshell2 parse.tab.c parse.lex.c myshell2.c