#include<stdio.h>
#define OUT_OF_SPACE_WARN(a) if(!a){yyerror("out of space");exit(0);}

extern int yylineno;
void yyerror(char *s, ...);

struct single_ast{
    int is_leaf;
    struct single_ast *l;
    char *r;
};

struct piping_ast{
    int is_leaf;
    struct piping_ast *l;
    struct single_ast *r;
};

struct comple_ast{
    struct piping_ast *pp;
    char *inf;
    char *outf;
};

struct bash_cmd{
    int type;
    char *arg;
};

void exec_bash_cmd(struct bash_cmd* cmd);

struct bash_cmd *new_bash_cmd(int type, char *arg);

struct single_ast *new_single_ast1(char *t);

struct single_ast *new_single_ast2(struct single_ast *l,char *r);

struct piping_ast *new_piping_ast1(struct single_ast *r);

struct piping_ast *new_piping_ast2(struct piping_ast *l,struct single_ast *r);

struct comple_ast *new_comple_ast(struct piping_ast *pp, char *inf,char *outf);

void print_single_command(struct single_ast *a);

void print_piping_commands(struct piping_ast *a);

void exec_comple_cmd(struct comple_ast *a);

void comple_ast_free(struct comple_ast *a);


