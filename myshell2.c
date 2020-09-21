#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<fcntl.h>
#include<unistd.h>
#include<pwd.h>
#include"abstract_syntax_tree.h"
#include"binary_semaphore.h"

//#define DEBUG

static char user_home[100];

void myshell_greeting(){
    struct passwd *pwd = getpwuid(getuid());
    char *user = pwd->pw_name;
    char *cwd = getcwd(NULL,100);
    if(strcmp(user,"root")==0){
        strcpy(user_home,"/");
    }else{
        strcpy(user_home,"/home/");
    }
    strcat(user_home,user);
    int n = strlen(cwd);
    int m = strlen(user_home);
    if(strncmp(cwd,user_home,m)==0){
        cwd[0]='~';
        for(int i=1;i<n-(m-1);i++)
            cwd[i]=cwd[i+m-1];
        cwd[n-(m-1)] = '\0';
    }
    printf("[myshell:%s] ",cwd);

    free(cwd);
    //free(pwd);
}

void exec_bash_cmd(struct bash_cmd* cmd){
    if(cmd->type=='c'){
        //cd
        //printf("chdir %s\n",cmd->arg);
        char *dir = cmd->arg;
        if(dir == NULL||dir[0]=='~'){
            dir = user_home;
        }
        if(chdir(dir)){
            printf("change directory failed\n");
        }
        free(cmd->arg);
        free(cmd);
    }else if(cmd->type=='e'){
        //exit
        printf("goodbye\n");
        exit(0);
    }
}

struct bash_cmd *new_bash_cmd(int type, char *arg){
    struct bash_cmd *a = malloc(sizeof(struct bash_cmd));
    OUT_OF_SPACE_WARN(a);
    a->type = type;
    a->arg = arg;
    return a;
}

struct single_ast *new_single_ast1(char *t){
    struct single_ast *a = malloc(sizeof(struct single_ast));
    OUT_OF_SPACE_WARN(a);
    a->is_leaf = 1;
    a->l = NULL;
    a->r = t;
    return a;
}

struct single_ast *new_single_ast2(struct single_ast *l,char *r){
    struct single_ast *a = malloc(sizeof(struct single_ast));
    OUT_OF_SPACE_WARN(a);
    a->is_leaf = 0;
    a->l = l;
    a->r = r;
    return a;
}

struct piping_ast *new_piping_ast1(struct single_ast *r){
    struct piping_ast *a = malloc(sizeof(struct piping_ast));
    OUT_OF_SPACE_WARN(a);
    a->is_leaf = 1;
    a->l = NULL;
    a->r = r;
    return a;
}

struct piping_ast *new_piping_ast2(struct piping_ast *l,struct single_ast *r){
    struct piping_ast *a = malloc(sizeof(struct piping_ast));
    OUT_OF_SPACE_WARN(a);
    a->is_leaf = 0;
    a->l = l;
    a->r = r;
    return a;
}

struct comple_ast *new_comple_ast(struct piping_ast *pp, char *inf,char *outf){
    struct comple_ast *a = malloc(sizeof(struct comple_ast));
    OUT_OF_SPACE_WARN(a);
    a->pp = pp;
    a->inf = inf;
    a->outf = outf;
    return a;
}

void print_single_command(struct single_ast *a){
    if(!(a->is_leaf)){
        print_single_command(a->l);
        printf(" ");
    }
    printf("%s",a->r);
}

void print_piping_commands(struct piping_ast *a){
    if(!(a->is_leaf)){
        print_piping_commands(a->l);
        printf(" |\n");
    }
    printf("  ");
    print_single_command(a->r);
}

void single_ast_free(struct single_ast *a){
    if(!(a->is_leaf)){
        single_ast_free(a->l);
    }
    free(a->r);
}

void piping_ast_free(struct piping_ast *a){
    if(!(a->is_leaf)){
        piping_ast_free(a->l);
    }
    single_ast_free(a->r);
}

void comple_ast_free(struct comple_ast *a){
    piping_ast_free(a->pp);
    free(a->inf);
    free(a->outf);
}


int get_single_command_argc(struct single_ast *a){
    if(!(a->is_leaf)){
        return get_single_command_argc(a->l) + 1;
    }
    return 1;
}

void fill_single_command_argv(struct single_ast *a, char** argv, int pos){
    argv[pos] = a->r;
    if(!(a->is_leaf))
        fill_single_command_argv(a->l, argv, pos-1);
}

void exec_single_command(struct single_ast *a){
    //printf("exec_single_cmd\n");
    int argc = get_single_command_argc(a);
    char **argv = malloc((argc+1)*sizeof(char*));
    //printf("argv=%d\n",argv);
    
    OUT_OF_SPACE_WARN(argv);
    argv[argc] = 0;
    fill_single_command_argv(a, argv, argc-1);
    //printf("cmd=%s\n",argv[0]);
    //printf("argc=%d\n",argc);
    execvp(argv[0],argv);
    perror("Failed to execute command");
}


void exec_piping_commands(struct piping_ast *a, int in_handle, int out_handle){

    int fds[2];
    pid_t pid;
    if(!(a->is_leaf)){
        pipe(fds);// 0 read     1 write
        //printf("pipe [%d==%d]\n",fds[1],fds[0]);
        pid = fork();
        if(pid == (pid_t) 0){
            //child
            //sleep(dep);
            close(fds[1]);
            //printf("[%d>>%d]\n",fds[0],out_handle);
            dup2(fds[0], STDIN_FILENO);
            dup2(out_handle, STDOUT_FILENO);
            exec_single_command(a->r);
#ifdef DEBUG
            fprintf(stderr,"if this was printed, sth goes wrong\n");
#endif
            exit(0);
        }else{
            if(out_handle!=STDOUT_FILENO){
                close(out_handle);
            }
            close(fds[0]);
            exec_piping_commands(a->l,in_handle,fds[1]);
            if(out_handle==STDOUT_FILENO)waitpid(pid, NULL, 0);
        }
    }else{
        pid = fork();
        if(pid == (pid_t) 0){
            //printf("[%d>>%d]\n",in_handle,out_handle);
            //in_handle=open("./test.in",O_RDONLY);
            dup2(in_handle, STDIN_FILENO);
            dup2(out_handle, STDOUT_FILENO);
            exec_single_command(a->r);
#ifdef DEBUG
            fprintf(stderr,"if this was printed, sth goes wrong\n");
#endif
            exit(0);
        }else{
            if(out_handle!=STDOUT_FILENO){
                close(out_handle);
            }else waitpid(pid, NULL, 0);
        }
    }
}

void exec_comple_cmd(struct comple_ast *a){

    int in_handle = STDIN_FILENO;
    int out_handle = STDOUT_FILENO;

#ifdef DEBUG
    printf("-preparing commands...\n");
    printf("-input=");
#endif

    if(a->inf == NULL){
#ifdef DEBUG
        printf("stdin");
#endif
    }
    else{
#ifdef DEBUG
        printf("%s",a->inf);
#endif
        in_handle = open(a->inf,O_RDONLY);
        if(in_handle == -1){
            fprintf(stderr, "\nopen input file '%s' failed\n",a->inf);
            return;
        }
    }
#ifdef DEBUG
    printf("\n-output=");
#endif
    if(a->outf == NULL){
#ifdef DEBUG
        printf("stdout ");
#endif
    }
    else{
#ifdef DEBUG
        printf("%s ",a->outf);
#endif
        out_handle = creat(a->outf,0755);
        if(out_handle == -1){
            fprintf(stderr, "\ncreate output file '%s' failed\n",a->outf);
            return;
        }
    }

#ifdef DEBUG
    printf("\n-piping commands=\n");
    print_piping_commands(a->pp);
    printf("\n-executing commands...\n");
#endif

    //sem_exec_finish = binary_semaphore_create_default(0);

    exec_piping_commands(a->pp,in_handle,out_handle);

    //binary_semaphore_wait(sem_exec_finish);
    //binary_semaphore_deallocate(sem_exec_finish);
}

void yyerror(char *s, ...){
    va_list ap;
    va_start(ap, s);
    fprintf(stderr,"%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
}

int main(){
    myshell_greeting();
    return yyparse();
}