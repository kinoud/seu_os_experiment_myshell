%option noyywrap nodefault yylineno
%{
    #include <errno.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <string.h>
    #include <sys/types.h>
    #include <sys/wait.h>

    #include "parse.tab.h"
%}

%%
"|" |
"<" |
">" |
"\n"    { return yytext[0]; }
^[ ]*"cd"   { return CD; }
^[ ]*"exit" { return EXIT; }
[^ \t\n"'|<>]+ {
                char *t;
                int n=strlen(yytext);
                if((t = malloc(n + 1)) == NULL){
                    perror("Failed to allocate memory to yylval.s");
                    return 0;
                }

                for(int i=0;i<n;i++)
                    t[i]=yytext[i];
                t[n]='\0';

                yylval.s = t;
                //printf("t='%s'\n",t);
                return TOK;
            }
(\"([^\\"]|\\.)*\"|\'([^\\']|\\.)*\')   {
                                            char *t;
                                            int n=strlen(yytext)-2;
                                            if((t = malloc(n+1)) == NULL){
                                                perror("Failed to allocate memory to yylval.s");
                                                return 0;
                                            }

                                            for(int i=0;i<n;i++)
                                                t[i]=yytext[i+1];
                                            t[n]='\0';
                                            yylval.s = t;
                                            //printf("t='%s'\n",t);
                                            return TOK;
                                        }
.   ;
%%
