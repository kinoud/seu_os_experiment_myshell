%{
    #include "abstract_syntax_tree.h"
    extern void myshell_greeting();
%}

%union {
    struct single_ast *single_a;
    struct piping_ast *piping_a;
    struct comple_ast *comple_a;
    char *s;
    int *t;
}

%token <s> TOK
%token CD EXIT

%type <single_a> single_cmd 
%type <piping_a> piping_cmd
%type <comple_a> comple_cmd
%type <t> bash_cmd

%start exec_list
%%
exec_list   : /*空*/
            | exec_list comple_cmd '\n' { exec_comple_cmd($2); comple_ast_free($2); myshell_greeting(); }
            | exec_list '\n' { myshell_greeting(); }
            | exec_list bash_cmd '\n' { exec_bash_cmd($2);  myshell_greeting(); }
            | exec_list error '\n' {yyerrok; myshell_greeting(); }
            ;

bash_cmd    : CD TOK { $$ = new_bash_cmd('c', $2); }
            | CD { $$ = new_bash_cmd('c', NULL); }
            | EXIT { $$ = new_bash_cmd('e', NULL); }
            ;

single_cmd  : single_cmd TOK { $$ = new_single_ast2($1, $2); }
            | TOK { $$ = new_single_ast1($1); }
            ;

piping_cmd  : piping_cmd '|' single_cmd { $$ = new_piping_ast2($1, $3); }
            | single_cmd { $$ = new_piping_ast1($1); }
            ;

comple_cmd  : piping_cmd { $$ = new_comple_ast($1, NULL, NULL); }
            | piping_cmd '>' TOK '<' TOK { $$ = new_comple_ast($1, $5, $3); }
            | piping_cmd '<' TOK '>' TOK { $$ = new_comple_ast($1, $3, $5); }
            | piping_cmd '>' TOK { $$ = new_comple_ast($1, NULL, $3); }
            | piping_cmd '<' TOK { $$ = new_comple_ast($1, $3, NULL); }
            ;

%%
