#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// Start the Shell
void init_shell() {
    clear();
    char *user = getenv("USER");
    printf("\n| ------------------------------------------------------------------- |");
    printf("\n|                                                                     |");
    printf("\n| ____________________ Welcome to Cauchy's Shell ____________________ |");
    printf("\n|                                                                     |");
    printf("\n|                           Have a good day!                          |");
    printf("\n|                                                                     |");
    printf("\n|                       ***********************                       |");
    printf("\n|                                                                     |");
    printf("\n| ------------------------------------------------------------------- |\n");
    printf("\n__________________________  User is : @%s  ________________________\n",user);
    printf("\n***********************************************************************\n\n\n");
    sleep(1);
}

// Function to take input and store to str
int take_input(char* str) {
    char* buff;
    buff = readline("\033[0;32m\n>>> \033[0m");
    
    if (strlen(buff) != 0) { 
        add_history(buff);
        strcpy(str, buff);
        return 0;
    } else {
        return 1;
    }
}

// Function to print current directory
void current_dir() { 
    char cur_dir[1024];
    getcwd(cur_dir, sizeof(cur_dir));
    printf("\n\033[0;32mCur_Dir:\033[0m \033[0;32m%s\033[0m\n", cur_dir);
}

// Function where the system command is executed
void exec_syscmd(char *parsed[]) {
    pid_t process_pid = fork();

    if (process_pid >= 0) {
        if (process_pid == 0) {
            if (execvp(parsed[0], parsed) < 0) {
                printf("\nError: could not execute command...");
            }
            exit(0);
        }
        else {
            wait(NULL);
            return;
        }
    } 
    else {
        printf("fork() is failed\n");
        return;
    }
}

// Function where the piped system commands is executed
void exec_pipecmd(char **cmd1, char **cmd2) {
    int fds[2];
    int status;
    pid_t pid1, pid2;

    if (pipe(fds) < 0) {
        printf("pipe() unsuccessfully\n");
        return;
    }

    pid1 = fork();
    if (pid1 < 0) {
        printf("\nCould not fork pid1");
        return;
    }

    if (pid1 == 0) {
        // Child 1 executing ...
        // Write at the write end
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);

        if (execvp(cmd1[0], cmd1) < 0) {
            printf("\nCould not execute command 1 ..."); 
            exit(0); 
        }
    }
    else {
        // Parent executing
        pid2 = fork();
        if (pid2 < 0) {
            printf("\nCould not fork pid2");
            return;
        }

        // Child 2 executing ...
        // Read at the read end
        if (pid2 == 0) {
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);

            if (execvp(cmd2[0], cmd2) < 0) {
                printf("\nCould not execute command 2 ..."); 
                exit(0); 
            }
        }
        else {
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
        }
    }
}

// Function for check pipe and separate string
int parse_pipe(char *str, char **strpipe) {
    for (int i = 0; i < 2; i++) {
        strpipe[i] = strsep(&str, "|");

        if (strpipe[i] == NULL) {
            break;
        }
    }

    if (strpipe[1] == NULL) {
        // returns zero if no pipe is found
        return 0;
    }
    else { 
        return 1; 
    } 
}

// Function for parsing command words
int parse_space(char *str, char **parsed) {
    for (int i = 0; i < 100; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL) {
            break;
        }
        if (strlen(parsed[i]) == 0) {
            i--;
        }
    }
}

// Function for process input command 1 and command 2
int process_string(char *str, char **cmd1, char **cmd2) {
    int check = 0;
    char *strpipe[2];

    check = parse_pipe(str, strpipe);

    if (check) {
        // if it is including a pipe
        parse_space(strpipe[0], cmd1);
        parse_space(strpipe[1], cmd2);
    }
    else {
        // if it is a simple command
        parse_space(str, cmd1);
    }

    if (strcmp(cmd1[0], "cd")  == 0) {
        chdir(cmd1[1]);
        return 0;
    }
    else {
        return check + 1;
    }
}

// Clear memory command
void clear_mem(char **cmd) {
    for (int i = 0; i < 100; i++) {
        cmd[i] = NULL;
    }
}

int main() {
    char input[1000];
    char *cmd1[100];
    char *cmd2[100];

    int flag = 0;
    init_shell();

    while(1) {
        current_dir();
        take_input(input);

        flag = process_string(input, cmd1, cmd2);

        if (flag == 1) {
            exec_syscmd(cmd1);
        }
        else if (flag == 2) {
            exec_pipecmd(cmd1, cmd2);
        }

        clear_mem(cmd1);
        clear_mem(cmd2);
    }

    return 0;
}