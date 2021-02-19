//sh360.c
//SHELL 360 source code
//Isobel Glover
//V00876157
//CSC360 Assignment 1

//libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//define constants:
#define MAX_INPUT_LINE 100
#define MAX_PROMPT_LEN 10
#define MAX_PATH_DIRS 10
#define MAX_CMD_ARGS 11  
#define MAX_PATH_LEN 100 


//define custom error codes
#define NO_PROMPT_FILE 10
#define BAD_PROCESS 11 
#define COMMAND_NOT_FOUND 12
#define OR_SYNTAX_ERROR 13
#define PP_SYNTAX_ERROR 14 
#define FILE_ERROR 15
#define LINE_TOO_LONG 16 
#define TOO_MANY_TOKENS 17


//function declarations:
int execute_command(char* args[], char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1],int dir_num);
int execute_or_command(char* args[],char* outfile, char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1],int dir_num);
int execute_pp_command(char* args[], int num_tokens, int num_commands, char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1],int dir_num,char** bad_cmd);
int find_command(char* cmd,char* cmd_with_path, char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1],int dir_num);
int parse_pathfile(char* pathfile, char* prompt, char path_list[][MAX_PATH_LEN + 1], int* dir_num);
int input_to_tokens(char input[MAX_INPUT_LINE + 1], char* tokens[MAX_CMD_ARGS],int* num_tokens,char* delims);
int check_OR_syntax(char** tokens,int num_tokens,char* outfile,char* args[]);
int check_pp_syntax(char** tokens, int num_tokens, char* args[], int* num_commands);


//MAIN
int main(int argc, char* argv[]){

    //get prompt from .sh360rc file:
    char prompt[MAX_PROMPT_LEN + 1];
    char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1];
    int dir_num_tmp;
    char* pathfile = ".sh360rc";
    int pathfile_error = parse_pathfile(pathfile,prompt,path_list,&dir_num_tmp);
    const int dir_num = dir_num_tmp; 


    //check for error
    if (pathfile_error == NO_PROMPT_FILE){
        fprintf(stderr,"FATAL: path/prompt file does not exist!\n");
        exit(NO_PROMPT_FILE);
    }


    printf("\nstarting SHELL 360...\n\n");

    //declare variables used in main loop
    char input_buffer[MAX_INPUT_LINE + 1];
    char* tokens[MAX_CMD_ARGS];
    int num_tokens = 0;
    char* delims = " \t";
    
    int or = 0;
    int pp = 0;
    char* args[MAX_CMD_ARGS + 1];
    char* cmd;
    int status;

    int input_err;
    int cd_error;


    char outfile[MAX_INPUT_LINE + 1];
    int or_syntax_error;
    int num_commands;
    int pp_syntax_error;

    //main loop (each iteration of the loop represents 1 command entered at the prompt)
    for(;;){

        //loop variables
        or = 0;
        pp = 0;
        //char* args[num_tokens+1];
        //char* cmd;
        //int status;
        
        //reset number of tokens
        num_tokens = 0;
        
        //print prompt
        fprintf(stdout,"%s",prompt);
        fflush(stdout);

        //read input line
        input_err = input_to_tokens(input_buffer, tokens, &num_tokens, delims);

        //check for input parse errors (line too long, too many tokens).
        if (input_err == LINE_TOO_LONG){
            fprintf(stderr,"Input error: line too long. Please enter a line of max %d characters.\n",MAX_INPUT_LINE);
            continue;
        }
        if (input_err == TOO_MANY_TOKENS){
            fprintf(stderr,"Input error: too many tokens. Please enter a line with a max of %d tokens.\n",MAX_CMD_ARGS);
            continue;
        }

        //check for exit command (should exit only if 'exit' is the sole command (ie, no arguments))
        if (strcmp(tokens[0],"exit") == 0){
            if (num_tokens == 1){
                fprintf(stdout,"\nexiting SHELL 360...\n\n");
                exit(EXIT_SUCCESS);
            }
            fprintf(stderr,"%s: command not found. To exit SHELL 360 use 'exit' with no arguments.\n",tokens[0]);
        }

        //check for cd command:
        if (strcmp(tokens[0], "cd") == 0){
            if (num_tokens == 2){
                cd_error = chdir(tokens[1]);
                if (cd_error == -1){
                    fprintf(stderr,"Error: cd to %s failed. Ensure directory exists.\n",tokens[1]);
                }
            }
            else {
                fprintf(stderr,"Error: cd supports only one argument.\n");
            }
            continue;
        }

        //check for an OR command:
        if (strcmp("OR",tokens[0]) == 0){
            or = 1; //OR flag

            //check OR syntax
            //outfile;
            or_syntax_error = check_OR_syntax(tokens,num_tokens,outfile,args);
            if (or_syntax_error == OR_SYNTAX_ERROR){
                fprintf(stderr,"OR syntax error. Usage: OR [cmd] -> [filename]\n");
                continue;
            }
            //execute the command
            cmd = args[0];
            status = execute_or_command(args, outfile, path_list, dir_num);

            //check for execution error specific to OR (output file error)
            if (status == FILE_ERROR){
                fprintf(stderr,"File error: cannot open %s for writing.\n",outfile);
                continue;
            }
        }

        //check for a PP command
        if (strcmp("PP",tokens[0]) == 0){
            pp = 1; //PP flag

            //check PP syntax
            //num_commands;
            pp_syntax_error = check_pp_syntax(tokens, num_tokens, args, &num_commands);
            if (pp_syntax_error == PP_SYNTAX_ERROR){
                fprintf(stderr,"PP syntax error. Usage: PP [cmd1] -> [cmd2] (-> [cmd3])\n");
                continue;
            }

            //execute PP command
            
            status = execute_pp_command(args, num_tokens, num_commands, path_list, dir_num, &cmd);

        }

        //ordinary command (not OR or PP)
        if (!(or) && !(pp)){
            //build args list (this is done in function that checks syntax for OR and PP commands
            for (int i=0; i<num_tokens; i++){
                args[i] = tokens[i];
            }
            args[num_tokens] = 0;
            cmd = args[0];

            //execute command
            status = execute_command(args,path_list,dir_num);
        }

        //check for execution errors (these apply to ordinary, OR, and PP commands)
        if (status == BAD_PROCESS){
            fprintf(stderr, "FATAL: error in process execution. Please restart shell.\n");
            exit(BAD_PROCESS);
        }
        if (status == COMMAND_NOT_FOUND){
            fprintf(stderr,"%s: command not found. Ensure command is entered correctly and that ./.sh360rc contains directory of command.\n",cmd);
            continue;
        }
    }
} //end main


//Function definitions


//Executes an ordinary (not OR or PP) command by creating a child process.
//calls find_command to check that the command exists in path_list (the list of directories from .sh360rc)
//before attempting to run the process.
//NOTE: This function is based on code adapted from appendix_b.c by Micheal Zastre
//parameters:
//args - an array of arguments to the command (including the command itself), terminating with "\0"
//path_list - an array of the directories in which to search for command executables, as read from .sh360rc
//dir_num - the number of directories in path_list
//returns:
//0 upon successful completion
//COMMAND_NOT_FOUND if the command is not found in any directory in path_list
//BAD_PROCESS if some error occurs within the process execution
int execute_command(char* args[], char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1],int dir_num){

    char cmd_with_path[MAX_PATH_LEN+MAX_INPUT_LINE+2];
    char* cmd = args[0];

    int cmd_status = find_command(cmd, cmd_with_path, path_list, dir_num);
    args[0] = cmd_with_path;

    if (cmd_status == COMMAND_NOT_FOUND){
        return COMMAND_NOT_FOUND;
    }
    int pid;
    int status;
    char* envp[] = {0};
    if ((pid=fork()) == 0){
        execve(args[0],args,envp);
        fprintf(stderr,"BAD child! Get out!\n");
        return BAD_PROCESS;
    }
    while (wait(&status) > 0){
    }
    return 0;
}


//Executes an OR command by creating a child process.
//calls find_command to check that the command exists in path_list (the list of directories from .sh360rc)
//before attempting to run the process.
//NOTE: This function is based on code adapted from appendix_c.c by Micheal Zastre
//parameters:
//args - an array of arguments to the command (including the command itself), terminating with "\0"
//outfile - the filename of the file to which the output is redirected
//path_list - an array of the directories in which to search for command executables, as read from .sh360rc
//dir_num - the number of directories in path_list
//returns:
//0 upon successful completion
//COMMAND_NOT_FOUND if the command is not found in any directory in path_list
//BAD_PROCESS if some error occurs within the process execution
//FILE_ERROR if outfile is unable to be opened for writing
int execute_or_command(char* args[],char* outfile, char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1],int dir_num){

    char cmd_with_path[MAX_PATH_LEN+MAX_INPUT_LINE+2];

    char* cmd = args[0];
    int cmd_status = find_command(cmd,cmd_with_path,path_list,dir_num);
    args[0] = cmd_with_path;
    if (cmd_status == COMMAND_NOT_FOUND){
        return COMMAND_NOT_FOUND;
    }

    int pid;
    int status;
    char* envp[] = {0};

    //now execute command.
    if ((pid=fork()) == 0){
        int fd = open(outfile,O_CREAT|O_RDWR|O_TRUNC,S_IRUSR|S_IWUSR);
        if (fd == -1){
            return FILE_ERROR;
        }
        dup2(fd,1);
        dup2(fd,2);
        execve(args[0],args,envp);
        return BAD_PROCESS;
    }
    while (wait(&status) > 0){
    }
    return 0;

}

//Executes a PP command by creating a child process for each command and creating pipes to connect the commands.
//The cases for 2 or 3 commands are considered separately due to the different pipe structure required.
//Calls find_command to check that each command exists in path_list (the list of directories from .sh360rc)
//before attempting to run the processes.
//NOTE: This function is based on code adapted from appendix_d.c by Micheal Zastre
//parameters:
//args - an array of arguments to all the commands, where the different commands are separated by
//an entry of "\0" and the array is terminated with "\0"
//num_tokens - the number of tokens in the original user input
//num_commands - the number of different commands (2 or 3)
//path_list - an array of the directories in which to search for command executables, as read from .sh360rc
//dir_num - the number of directories in path_list
//bad_cmd - a pointer to a string in which to put the first command that causes an error, if this occurs.
//returns:
//0 upon successful completion
//COMMAND_NOT_FOUND if the command is not found in any directory in path_list
//BAD_PROCESS if some error occurs within the process execution
int execute_pp_command(char* args[], int num_tokens, int num_commands, char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1],int dir_num, char** bad_cmd){
    

    //declare all args arrays to avoid errors
    char** args1;
    char** args2;
    char** args3;
    char* envp[] = {0};
    //executing pp
    //first, check number of commands and divide up args list
    args1 = &args[0];


    int next_args = 2;
    for (int i=0; i<num_tokens; i++){
        if (args[i] == 0){
            if (next_args == 2){
                args2 = &args[i+1];
                next_args  = 3;
            }
            else if (next_args == 3){
                args3 = &args[i+1];
                break;
            }
            
        }
    }


    //check command exists, add path to command.
    char cmd1_with_path[MAX_PATH_LEN+MAX_INPUT_LINE+2];
    char* cmd1 = args1[0];
    int cmd1_status = find_command(cmd1,cmd1_with_path,path_list,dir_num);
    args1[0] = cmd1_with_path;

    char cmd2_with_path[MAX_PATH_LEN+MAX_INPUT_LINE+2];
    char* cmd2 = args2[0];
    int cmd2_status = find_command(cmd2,cmd2_with_path,path_list,dir_num);
    args2[0] = cmd2_with_path;

    if (num_commands == 3){
        char cmd3_with_path[MAX_PATH_LEN+MAX_INPUT_LINE+2];
        char* cmd3 = args3[0];
        int cmd3_status = find_command(cmd3,cmd3_with_path,path_list,dir_num);
        args3[0] = cmd3_with_path;
        if (cmd3_status == COMMAND_NOT_FOUND){
            *bad_cmd = cmd3;
            return COMMAND_NOT_FOUND;
        }
    }
    if (cmd1_status == COMMAND_NOT_FOUND)
    {
        *bad_cmd = cmd1;
        return COMMAND_NOT_FOUND;
    }
    if (cmd2_status == COMMAND_NOT_FOUND){
        *bad_cmd = cmd2;
        return COMMAND_NOT_FOUND;
    }

    //Do one at a time.
    if (num_commands == 2){
        int status;
        int pid1, pid2;
        int fd[2];
        pipe(fd);

        if ((pid1 = fork()) == 0) { //child 1
            dup2(fd[1],1);
            close(fd[0]);
            execve(args1[0],args1,envp);
            return BAD_PROCESS;
        }
        
        if ((pid2 = fork()) == 0) { //child 2
            dup2(fd[0],0);
            close(fd[1]);
            execve(args2[0],args2,envp);
            return BAD_PROCESS;
        }

        close(fd[0]);
        close(fd[1]);
        waitpid(pid1,&status,0);
        waitpid(pid2,&status,0);


    }
    if (num_commands == 3){
        int status;
        int pid1, pid2, pid3;
        int fd1[2]; //pipe 1
        int fd2[2]; //pipe 2

        pipe(fd1);
        pipe(fd2); 

        if ((pid1 = fork()) == 0) { //child 1
            dup2(fd1[1],1);

            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);

            execve(args1[0],args1,envp);
            return BAD_PROCESS;
        }
        
        if ((pid2 = fork()) == 0) { //child 2
            dup2(fd1[0],0);
            dup2(fd2[1],1);

            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);

            execve(args2[0],args2,envp);
            return BAD_PROCESS;
        }

        if ((pid3 = fork()) == 0) { //child 2
            dup2(fd2[0],0);

            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);

            execve(args3[0],args3,envp);
            return BAD_PROCESS;
        }

        close(fd1[0]);
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);

        waitpid(pid1,&status,0);
        waitpid(pid2,&status,0);
        waitpid(pid3,&status,0);
    }
    return 0;
}


//Checks if a command is valid and concatenates it with its path if it is found.
//Called by each execute_command function.
//parameters:
//cmd - the command to find
//cmd_with_path - a string in which the command concatenated with its correct path will be stored
//path_list - an array of the directories in which to search for command executables, as read from .sh360rc
//dir_num - the number of directories in path_list
//returns:
//0 on successful completion
//COMMAND_NOT_FOUND if the command cannot be found in any directory in path_list
int find_command(char* cmd,char* cmd_with_path, char path_list[MAX_PATH_DIRS][MAX_PATH_LEN + 1], int dir_num){

    for (int i=0; i<dir_num; i++){
        //create concatenated string 
        strncpy(cmd_with_path,path_list[i],MAX_PATH_LEN + 1);
        strncat(cmd_with_path,"/",2); //add '/' to separate path and command
        strncat(cmd_with_path,cmd,MAX_INPUT_LINE + 1);


        //check if the file is executable
        if (access(cmd_with_path,X_OK)==0){ 
            //we have found the command
            return 0;
            //go to execute command
        }
    }
    return COMMAND_NOT_FOUND;
}



//Parses the path/prompt file, .sh360rc, gets prompt and builds path_list, the array of 
//directories in which to search for commands.
//parameters:
//pathfile: the filename which contains the prompt and path information (in our case, .sh360rc)
//prompt: string in which to store the prompt
//path_list: array in which to store the list of directories
//dir_num: pointer to an integer in which to store the number of directories in path_list
//returns:
//0 on successful completion
//NO_PROMPT_FILE if the path/prompt file cannot be found or opened.
int parse_pathfile(char* pathfile, char* prompt, char path_list[][MAX_PATH_LEN + 1], int* dir_num){
    FILE* fd = fopen(pathfile,"r");
    if (fd == NULL){
        return NO_PROMPT_FILE;
    }
    fgets(prompt,MAX_PROMPT_LEN,fd);
    // truncate newline from prompt
    if (prompt[strlen(prompt) - 1] == '\n'){
        prompt[strlen(prompt) - 1] = '\0';
    }

    char buffer[MAX_PATH_LEN + 1] = "\0";
    int path_len; 
    *dir_num = 0;

    while(fgets(buffer,MAX_PATH_LEN,fd)){
        path_len = strlen(buffer);
        //only consider a valid path if not empty (otherwise don't count it and don't add to path list)
        if (path_len > 0){
            if (buffer[path_len - 1] == '\n'){
                buffer[path_len - 1] = '\0';
            }

            strncpy(path_list[*dir_num],buffer,MAX_PATH_LEN + 1);
            *dir_num  = *dir_num + 1;
        }
    }
    fclose(fd);
    return 0;
}



//Reads and tokenizes the user input.
//NOTE: This function is partially based on code from appendix_e.c by Micheal Zastre
//parameters:
//input - a string used to store the line input by the user
//tokens - an array in which the tokens from the tokenized input are stored
//num_tokens - a pointer to an integer in which the number of tokens is stored
//delims - a string of delimiters used to tokenize the user input
//returns:
//0 upon successful completion
//LINE_TOO_LONG if the line entered by the user is too long
//TOO_MANY_TOKENS if the user entered too many tokens
int input_to_tokens(char input[MAX_INPUT_LINE + 1], char* tokens[MAX_CMD_ARGS],int* num_tokens,char* delims){
    int line_len;
    char *t;

    fgets(input,MAX_INPUT_LINE + 1,stdin);
    line_len = strlen(input);
    //remove newline from input
    if (input[line_len - 1] == '\n'){
        input[line_len - 1] = '\0';
    }
    else {
        //get rid of the rest of the too long line so it doesn't get used as input.
        do {
            fgets(input,MAX_INPUT_LINE + 1,stdin);
        }
        while(input[strlen(input) - 1] != '\n');
        return LINE_TOO_LONG;
    }

    t = strtok(input,delims);
    while (t != NULL && *num_tokens < MAX_CMD_ARGS){
        tokens[*num_tokens] = t;
        *num_tokens = *num_tokens + 1;
        t = strtok(NULL,delims);
    }
    if (t != NULL){
        //too many args
        return TOO_MANY_TOKENS;
    }

    return 0;
}


//checks that the syntax of an OR command is valid and fills the args list.
//parameters:
//tokens - pointer to the array of tokens input by the user
//num_tokens - the number of tokens in tokens
//outfile - string in which to store the output file name found in tokens
//args - array in which to store the command and its arguments
//returns:
//0 on successful completion
//OR_SYNTAX_ERROR if syntax for the OR command is incorrect.
int check_OR_syntax(char** tokens,int num_tokens,char* outfile,char* args[]){
    int num_args = 0;
    int arrow_ind = 0;
    int arrow_found = 0;
    int outfile_found = 0;
    for (int i=1; i<num_tokens;i++){
        if (arrow_found){
            strncpy(outfile,tokens[i],MAX_INPUT_LINE + 1);
            outfile_found = 1;
            break;
        }
        if (strcmp(tokens[i],"->") == 0){
            arrow_ind = i;
            arrow_found = 1;
            continue;
        }
        if (!arrow_found){
            num_args++;
            args[i-1] = tokens[i];
        }
    }
    if (arrow_ind+2 != num_tokens || !arrow_found || !outfile_found || num_args < 1){
        return OR_SYNTAX_ERROR;
    }

    args[num_args] = 0;
    return 0;
}



//checks that the syntax of a PP command is valid and fills the args list for the command.
//The args for all commands will be stored in a single list, and separated by null commands.
//parameters:
//tokens - pointer to the array of tokens input by the user
//num_tokens - the number of tokens in tokens
//args - array in which to store the command and its arguments
//num_commands - pointer to an integer in which to store the number of commands in the PP statement (2 or 3)
//returns:
//0 on successful completion
//PP_SYNTAX_ERROR if syntax for the PP command is incorrect.
int check_pp_syntax(char** tokens, int num_tokens, char* args[], int* num_commands){
    int num_commands_tmp = 1;
    int num_arrows = 0;
    int command_expected = 1;
    int syntax_error = 0;

    for (int i=1; i<num_tokens;i++){
        //go through tokens (skip first since we know it is PP)
        if (strcmp(tokens[i],"->") != 0 ){
            command_expected = 0;
            args[i-1] = tokens[i];
            continue;
        }
        else {
            if (command_expected == 1){
                //2 arrows in a row
                syntax_error = 1;
                break;
            }
            //we have found a ->
            args[i-1] = 0; //test
            num_arrows += 1;
            command_expected = 1;
            continue;
        }

    }
    args[num_tokens - 1] = 0; //test 
    //now check validity:
    if (num_arrows < 1 || command_expected == 1 || num_commands_tmp > 3){
        syntax_error = 1;
    }
    num_commands_tmp = num_arrows + 1;
    *num_commands = num_commands_tmp;
    if (syntax_error){
        return PP_SYNTAX_ERROR;
    }
    return 0;
}


