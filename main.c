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
    struct command *cmd = malloc(sizeof(struct command));

    // Grab init command
    char *cmdptr;
    char *cmdtoken = strtok_r(cmdLine, " ", &cmdptr);
    cmd->cmd = calloc(strlen(cmdtoken) + 1, sizeof(char));
    strcpy(cmd->cmd, cmdtoken);
    cmdtoken = strtok_r(NULL, " ", &cmdptr);

    // Check for extra params
    while(cmdtoken != NULL){
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
        // Check for arguments
        } else {
            strcpy(cmd->args[cmd->numArgs], cmdtoken);
            cmd->numArgs = cmd->numArgs + 1;
        }

        cmdtoken = strtok_r(NULL, " ", &cmdptr);
    }

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
    struct command *cmd = createCmd(cmdLine);
    // Check if cmd is a cd command
    if(strcmp(cmd->cmd,"cd") == 0){
        char *dir = malloc(sizeof(char) * BUFFER_SIZE);
        // Cd to arg path if provided, else home
        if(cmd->numArgs >= 1){
            chdir(cmd->args[0]);
        } else {
            chdir(getenv("HOME"));
        }
        
        printf("CWD is %s\n", getcwd(dir, BUFFER_SIZE));
        fflush(stdout);
        free(dir);
        free(cmd);
        return 1;
    } else if (strcmp(cmd->cmd,"status") == 0){
        printf("exit value %d\n", *status);
        free(cmd);
        return 1;
    }
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
    struct command *cmd = createCmd(cmdLine);
    pid_t spawnpid = -5;

    // Fork structure example from https://github.com/brentirwin/cs344-smallsh/blob/master/smallsh.c

    spawnpid = fork();
    
    switch (spawnpid){
        case -1:
            perror("fork() failed!\n");
            exit(1);
            break;
        case 0:
            setIOStreams(cmd);
            char *execArgs[BUFFER_SIZE];
            execArgs[0] = cmd->cmd;

            if(cmd->args[0]){
                for(int i = 0; i < cmd->numArgs; i++){
                    execArgs[i + 1] = cmd->args[i];
                }
            }

            execArgs[cmd->numArgs + 1] = NULL;
            fflush(stdout);
            // *lastStatus = 0;
            execvp(execArgs[0], execArgs);
            perror("exexvp");
            fflush(stdout);
            // *lastStatus = 1;
            exit(1);
            break;
        default:
            if(cmd->background){
                pid_t runpid = waitpid(spawnpid, lastStatus, WNOHANG);
                printf("background pid is %d\n", spawnpid);
                fflush(stdout);
            } else {
                pid_t runpid = waitpid(spawnpid, lastStatus, 0);
            }
            
            while ((spawnpid = waitpid(-1, lastStatus, WNOHANG)) > 0) {
                printf("child %d terminated\n", spawnpid);
                if (WIFEXITED(lastStatus)) {
		        // If exited by status
		            printf("exit value %d\n", WEXITSTATUS(lastStatus));
	            } else {
		        // If terminated by signal
		            printf("terminated by signal %d\n", WTERMSIG(lastStatus));
	            }
                fflush(stdout);
            }
    }
        // printf("The process with pid %d is returning from main\n", getpid());
        // free(cmd);
}

// Exit status code video https://www.youtube.com/watch?v=DiNmwwQWl0g
int main(){
    int pid = getpid();
    int lastStatus = 0;


    // Ignore ^C
	struct sigaction sa_sigint = {0};
	sa_sigint.sa_handler = SIG_IGN;
	sigfillset(&sa_sigint.sa_mask);
	sa_sigint.sa_flags = 0;
	sigaction(SIGINT, &sa_sigint, NULL);

	// // Redirect ^Z to catchSIGTSTP()
	// struct sigaction sa_sigtstp = {0};
	// sa_sigtstp.sa_handler = catchSIGTSTP;
	// sigfillset(&sa_sigtstp.sa_mask);
	// sa_sigtstp.sa_flags = 0;
	// sigaction(SIGTSTP, &sa_sigtstp, NULL);


    while (1){
        char* cmdLine = malloc(sizeof(char) * BUFFER_SIZE);
        getcmd(cmdLine);
        char* cmdCopy = malloc(sizeof(char) * BUFFER_SIZE);
        strcpy(cmdCopy, cmdLine);
        cmdLine[strcspn(cmdLine, "\n")] = '\0';
        cmdCopy[strcspn(cmdCopy, "\n")] = '\0';

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
            fflush(stdout);
            runCmd(cmdCopy, &lastStatus);
        }

        fflush(stdout);
        free(cmdLine);
        free(cmdCopy);
    }

    return EXIT_SUCCESS;
}