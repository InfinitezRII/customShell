#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 
#include<signal.h>
#include<sys/stat.h>

#define MAXCOM 1000
#define MAXLIST 100

#define clear() printf("\033[H\033[J") 

//Startup Screen
void init_shell() 
{ 
    clear();
    printf("\n");
    printf("===================================================================\n");
    printf("===================================================================\n");
    printf("\n");
    printf("***** *   * ***** ***** ***** *****   ***** *   * ***** *     *    \n");
	printf("*     *   * *       *   *   * * * *   *     *   * *     *     *    \n");
    printf("*     *   * *****   *   *   * * * *   ***** ***** ***** *     *    \n");
    printf("*     *   *     *   *   *   * * * *       * *   * *     *     *    \n");
    printf("***** ***** *****   *   ***** * * *   ***** *   * ***** ***** *****\n");
    printf("\n");
    printf("===================================================================\n");
    printf("===================================================================\n");
    printf("\n");
    printf("                        Use it with Caution!!                      \n");
    printf("\n");
    char* username = getenv("USER");
    printf("                        USER is: @%s\n", username); 
    sleep(3); 
    clear(); 
}

int input(char* str) 
{ 
    char* buf; 
  
    buf = readline("\n>>> "); 
    if (strlen(buf) != 0) { 
        add_history(buf); 
        strcpy(str, buf); 
        return 0; 
    } else { 
        return 1; 
    } 
}

void sigint_handler(int sig)
 {
   printf("\nType exit to close the shell!\n");
 }

void printDirAndUser() 
{ 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd));
    char* user = getenv("USER");
    printf("\nUSER is: @%s ", user); 
    printf("Dir: %s", cwd);
}

void execArgs(char** parsed) 
{ 
    pid_t pid = fork();  
  
    if (pid == -1) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (pid == 0) { 
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command.."); 
        } 
        exit(0); 
    } else { 
        wait(NULL);  
        return; 
    } 
} 
  

void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
    int pipefd[2];  
    pid_t p1, p2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (p1 == 0) { 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
  
        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 
    } else { 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 
  
void Help() 
{ 
    puts("\n<<<Prototype Custom Shell>>>"
        "\nPrototype By Nuttakit Jaritrum"
        "\n-Use the shell at your own risk..."
        "\n-Use with Cautions"
        "\nList of Commands supported:"
        "\n>cd"
        "\n>cent > make directory and cd at the same time"
        "\n>dout > exit directory and delete at the same time"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell(Maybe.....)"
        "\n>pipe handling"
        "\n>improper space handling"); 
  
    return; 
} 
  
int myCmd(char** parsed) 
{ 
    int NoOfMyCmds = 6, i, switchMyArg = 0; 
    char* myCmds[NoOfMyCmds]; 
    char* username; 
  
    myCmds[0] = "exit"; 
    myCmds[1] = "cd"; 
    myCmds[2] = "help"; 
    myCmds[3] = "hello"; 
    myCmds[4] = "cent";
    myCmds[5] = "dout";  
  
    for (i = 0; i < NoOfMyCmds; i++) { 
        if (strcmp(parsed[0], myCmds[i]) == 0) { 
            switchMyArg = i + 1; 
            break; 
        } 
    } 
  
    switch (switchMyArg) { 
    case 1: 
        printf("\nLeave Custom Shell\n"); 
        exit(0); 
    case 2: 
        chdir(parsed[1]); 
        return 1; 
    case 3: 
        Help(); 
        return 1; 
    case 4: 
        username = getenv("USER"); 
        printf("\nHi %s.\nThis is my Custom Shell "
            "If you want to know more let Use help command"
            "\nCareful when use this shell\n", 
            username); 
        return 1; 
    case 5: 
    	mkdir(parsed[1],0777);
        chdir(parsed[1]); 
        return 1; 
    case 6: 
    	chdir("..");
    	rmdir(parsed[1]); 
        return 1; 
    default: 
        break; 
    } 
  
    return 0; 
} 
  
int parsePipe(char* str, char** strpiped) 
{ 
    int i; 
    for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } 
  
    if (strpiped[1] == NULL) 
        return 0; 
    else { 
        return 1; 
    } 
} 
  
void parseSpace(char* str, char** parsed) 
{ 
    int i; 
  
    for (i = 0; i < MAXLIST; i++) { 
        parsed[i] = strsep(&str, " "); 
  
        if (parsed[i] == NULL) 
            break; 
        if (strlen(parsed[i]) == 0) 
            i--; 
    } 
} 
  
int processString(char* str, char** parsed, char** parsedpipe) 
{ 
  
    char* strpiped[2]; 
    int piped = 0; 
  
    piped = parsePipe(str, strpiped); 
  
    if (piped) { 
        parseSpace(strpiped[0], parsed); 
        parseSpace(strpiped[1], parsedpipe); 
  
    } else { 
  
        parseSpace(str, parsed); 
    } 
  
    if (myCmd(parsed)) 
        return 0; 
    else
        return 1 + piped; 
} 
  
int main() 
{ 
    char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
    char* parsedArgsPiped[MAXLIST]; 
    int execFlag = 0; 
    init_shell(); 
  	signal(SIGINT, sigint_handler);
    while (1) { 
        printDirAndUser(); 
        if (input(inputString)) 
            continue; 
        execFlag = processString(inputString, 
        parsedArgs, parsedArgsPiped); 
        if (execFlag == 1) 
            execArgs(parsedArgs); 
  
        if (execFlag == 2) 
            execArgsPiped(parsedArgs, parsedArgsPiped); 
    } 
    return 0; 
}
