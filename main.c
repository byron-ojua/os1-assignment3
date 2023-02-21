#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>

#define BUFFER_SIZE 2048
#define CMD_EXIT "exit"
#define CMD_ECHO "echo"


/**
 * @brief Basic structure for holding command parameters
 * 
 */
struct command{
    char *cmd;
    int background;
    int in;
    char *input;
    int out;
    char *output;
    char args[512][BUFFER_SIZE];
    int numArgs;
};

/**
 * @brief Retrieves cmd line input and puts it in a buffer
 * 
 * @param cmd Char string to put cmd in
 */
void getcmd(char *cmdLine){
    printf(": ");
    fflush(stdout);
    size_t bufsize = BUFFER_SIZE;
    getline(&cmdLine, &bufsize, stdin);
    // printf("%s", cmdLine);
}

/**
 * @brief Checks if cmd input is exit
 * 
 * @param cmdLine char array to check
 * @return int 1 if true, else 0
 */
int checkExit(char *cmdLine){
    //Check if command string is == 'exit'
    if(strcmp(CMD_EXIT, cmdLine) == 0){
        printf("\n");
        return 1;
    }

    return 0;
}

/**
 * @brief Checks is command should be skipped by checking if input is '\0' or '#'
 * 
 * @param cmdLine char array to check
 * @return int 1 is line should be skipped, else 0
 */
int skipCmd(char *cmdLine){
    if(cmdLine[0] == '\0' || cmdLine[0] == '#'){
        return 1;
    }

    return 0;
}

/**
 * @brief Checks if the command is am echo command, and echos if so
 * 
 * @param cmdLine 
 * @return int 
 */
int isEcho(char *cmdLine){
    // Check if command line is an echo command
    if(strncmp(CMD_ECHO, cmdLine, strlen(CMD_ECHO)) == 0){
        // Echo message if one is present, otherwise new line
        if(strlen(cmdLine) > 5){
            printf("%s\n", &cmdLine[5]);
        } else {
            printf("\n");
        }

        return 1;
    }
    return 0;
}

/**
 * @brief Prints out the properties of a command
 * 
 * @param cmd Command structure data
 */
void printCmd(struct command *cmd){
    printf("CMD: %s\n", cmd->cmd);
    printf("Output: %s\n", cmd->output);
    printf("Input: %s\n", cmd->input);
    printf("Background: %d\n", cmd->background);
    printf("Args: [");

    for (int i = 0; i < cmd->numArgs; i++){
        printf("%s", cmd->args[i]);
        if(i != cmd->numArgs - 1){
            printf(", ");
        }
    }
    printf("]\n");
    fflush(stdout);
}

/**
 * @brief Processes the inputted command
 * 
 * @param cmd Char array containing command
 */
struct command *createCmd(char *cmdLine){
    // char* cmdCopy = malloc(sizeof(char) * BUFFER_SIZE);

    // strcpy(cmdCopy, cmdLine);
    // printf("cmd in make is %s\n", cmdCopy);
    // printf("%s\n", cmdLine);
    struct command *cmd = malloc(sizeof(struct command));

    // Grab init command
    char *cmdptr;
    char *cmdtoken = strtok_r(cmdLine, " ", &cmdptr);
    // printf("%s\n", cmdLine);
    cmd->cmd = calloc(strlen(cmdtoken) + 1, sizeof(char));
    strcpy(cmd->cmd, cmdtoken);

    cmdtoken = strtok_r(NULL, " ", &cmdptr);
    // printf("%s\n", cmdLine);

    // Check for extra params
    while(cmdtoken != NULL){
        // printf("%s\n", cmdLine);
        //Check for running in background
        if (cmdtoken[0] == '&'){
            cmd->background = 1;
        // Check for output file
        } else if (cmdtoken[0] == '>'){
            cmdtoken = strtok_r(NULL, " ", &cmdptr);
            cmd->output = calloc(strlen(cmdtoken) + 1, sizeof(char));
            strcpy(cmd->output, cmdtoken);
            cmd->out = 1;
        // Check for input file
        } else if (cmdtoken[0] == '<'){
            cmdtoken = strtok_r(NULL, " ", &cmdptr);
            cmd->input = calloc(strlen(cmdtoken) + 1, sizeof(char));
            strcpy(cmd->input, cmdtoken);
            cmd->in = 1;
            // printf("%s\n", cmdtoken);
        // Check for arguments
        } else {
            strcpy(cmd->args[cmd->numArgs], cmdtoken);
            cmd->numArgs = cmd->numArgs + 1;
        }

        cmdtoken = strtok_r(NULL, " ", &cmdptr);
    }

    // printCmd(cmd);
    // printf("%s\n", cmdLine);
    // printf("I made it this far\n");
    // free(cmdCopy);
    return cmd;
}

/**
 * @brief Prints the status value of the last non-built in command
 * 
 * @param last 
 * @return int 
 */
int isStatus(int last){
    printf("%d", last);
    return 1;
}

/**
 * @brief Checks if command is built in, and runs if so
 * 
 * @param cmd Command structure data
 * @return int 1 is built in, else 0
 */
int isBuiltIn(char *cmdLine, int *status){
    // // printf("This is built in with %s\n", cmdLine);
    // struct command *cmd = createCmd(cmdLine);
    // // printf("%s\n", cmd->cmd);
    // // printf("I made it back to isBuilt in\n");
    // // printf("%s\n", cmdLine);
    // // printCmd(cmd);
    // // Check if cmd is a cd command
    // if(strcmp(cmd->cmd,"cd") == 0){
    //     char *dir = malloc(sizeof(char) * BUFFER_SIZE);
    //     // printf("CWD is %s\n", getcwd(dir, BUFFER_SIZE));
    //     // Cd to arg path if provided, else home
    //     if(cmd->numArgs >= 1){
    //         chdir(cmd->args[0]);
    //     } else {
    //         chdir(getenv("HOME"));
    //     }
        
    //     printf("CWD is %s\n", getcwd(dir, BUFFER_SIZE));
    //     fflush(stdout);
    //     free(dir);
    //     free(cmd);
    //     return 1;
    // } else if (strcmp(cmd->cmd,"status") == 0){
    //     // printf("exit value %d\n", *status);
    //     free(cmd);
    //     return 1;
    // }
    return 0;
}

void setIOStreams(struct command *cmd){
    if (cmd->in == 1){
        int file = open(cmd->input, O_RDONLY);
        if(file == -1){
            printf("Cannot open %s for input\n", cmd->input);
            fflush(stdout);
            exit(1);
        } else {
            dup2(file, STDIN_FILENO);
            close(file);
        }
    }
    if (cmd->out == 1){
        int file = open(cmd->output, O_WRONLY | O_TRUNC | O_CREAT, 0777);
        if(file == -1){
            printf("Cannot write to %s for output\n", cmd->output);
            fflush(stdout);
            exit(1);
        } else {
            dup2(file, STDOUT_FILENO);
            close(file);
        }
    }
}

void runCmd(char *cmdLine, int *lastStatus){
    // Code from Exploration: Process API - Monitoring Child Processes
    printf("cmdLine is %s\n", cmdLine);
    // char *copy = malloc(sizeof(char) * BUFFER_SIZE);
    // strcpy(copy, cmdLine);


    printf("I am in run\n");
    fflush(stdout);
    struct command *cmd = createCmd(cmdLine);
    // pid_t spawnpid = -5;


    // printCmd(cmd);
    // printf("I have made cmd\n");
    // printCmd(cmd);
    // Fork example from https://github.com/brentirwin/cs344-smallsh/blob/master/smallsh.c


    // printf("I am going to fork\n");
    // spawnpid = fork();
    
    // switch (spawnpid){
    //     case -1:
    //         perror("fork() failed!\n");
    //         exit(1);
    //         break;
    //     case 0:
    //         setIOStreams(cmd);
    //         char *execArgs[BUFFER_SIZE];
    //         execArgs[0] = cmd->cmd;

    //         printf("I am going to add arguments\n");
    //         fflush(stdout);
    //         if(cmd->args[0]){
    //             for(int i = 0; i < cmd->numArgs; i++){
    //                 execArgs[i + 1] = cmd->args[i];
    //             }
    //         }
    //         printf("I am going to add null\n");
    //         fflush(stdout);
    //         execArgs[cmd->numArgs + 1] = NULL;
    //         printf("I added null\n");
    //         fflush(stdout);
    //         printCmd(cmd);
    //         // printf("%s\n", execArgs[0]);
    //         // printf("%s\n", execArgs[1]);
    //         // fflush(stdout);

    //         execvp(execArgs[0], execArgs);
    //         printf("I should not be here\n");
    //         perror("exexvp");
    //         fflush(stdout);
    //         exit(1);

    //         break;
    //     default:
    //         if(cmd->background){
    //             pid_t runpid = waitpid(spawnpid, lastStatus, WNOHANG);
    //             printf("background pid is %d\n", spawnpid);
    //             fflush(stdout);
    //         } else {
    //             pid_t runpid = waitpid(spawnpid, lastStatus, 0);
    //         }
            




            
    //         // while ((spawnPid = waitpid(-1, childExitStatus, WNOHANG)) > 0) {
    //         //     printf("child %d terminated\n", spawnPid);
    //         //     printExitStatus(*childExitStatus);
    //         //     fflush(stdout);
    //         // }
    // }
        // printf("The process with pid %d is returning from main\n", getpid());
        // free(cmd);
}

// Exit status code video https://www.youtube.com/watch?v=DiNmwwQWl0g
int main(){
    int pid = getpid();
    int lastStatus = 0;

    while (1){
        char* cmdLine = malloc(sizeof(char) * BUFFER_SIZE);
        getcmd(cmdLine);
        cmdLine[strcspn(cmdLine, "\n")] = '\0';

        // printf("%s\n", cmdLine);

        if(skipCmd(cmdLine) || isEcho(cmdLine)) continue;
        if(checkExit(cmdLine)) break;
        fflush(stdout);

        // printf("%s\n", cmdLine);
        int builtIn = isBuiltIn(cmdLine, &lastStatus);
        // printf("Testing built in %d\n", builtIn);

        fflush(stdout);
        if(builtIn){
            // printf("I am inside builtIn if\n");
            fflush(stdout);
            continue;
        } 
        else {
            // printf("I am back in main for runCmd\n");
            // fflush(stdout);
            // printf("cmdLine before run is %s\n", cmdLine);
            fflush(stdout);
            // printf("%d\n", lastStatus);
            runCmd(cmdLine, &lastStatus);
        }

        // printf("%s\n", cmdLine);
        // printf("Is it the free?\n");
        // fflush(stdout);
        // free(cmdLine);
    }

    // // Ignore ^C
	// struct sigaction sa_sigint = {0};
	// sa_sigint.sa_handler = SIG_IGN;
	// sigfillset(&sa_sigint.sa_mask);
	// sa_sigint.sa_flags = 0;
	// sigaction(SIGINT, &sa_sigint, NULL);

	// // Redirect ^Z to catchSIGTSTP()
	// struct sigaction sa_sigtstp = {0};
	// sa_sigtstp.sa_handler = catchSIGTSTP;
	// sigfillset(&sa_sigtstp.sa_mask);
	// sa_sigtstp.sa_flags = 0;
	// sigaction(SIGTSTP, &sa_sigtstp, NULL);
    printf("I am leaving now\n");
    return EXIT_SUCCESS;
}