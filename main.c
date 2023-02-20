#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

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
    char *input;
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
        // Check for input file
        } else if (cmdtoken[0] == '<'){
            cmdtoken = strtok_r(NULL, " ", &cmdptr);
            cmd->input = calloc(strlen(cmdtoken) + 1, sizeof(char));
            strcpy(cmd->input, cmdtoken);
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
int isBuiltIn(struct command *cmd, int *status){
    // Check if cmd is a cd command
    if(strcmp(cmd->cmd,"cd") == 0){
        char *dir = malloc(sizeof(char) * BUFFER_SIZE);
        // printf("CWD is %s\n", getcwd(dir, BUFFER_SIZE));
        // Cd to arg path if provided, else home
        if(cmd->numArgs >= 1){
            chdir(cmd->args[0]);
        } else {
            chdir(getenv("HOME"));
        }
        
        printf("CWD is %s\n", getcwd(dir, BUFFER_SIZE));
        free(dir);
    } else if (strcmp(cmd->cmd,"status") == 0){

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
}

void runCmd(struct command *cmd){
    char *execArgs[BUFFER_SIZE];
    execArgs[0] = cmd->cmd;

    for(int i = 0; i < cmd->numArgs; i++){
        execArgs[i + 1] = cmd->args[i];
    }
    execArgs[cmd->numArgs + 1] = NULL;

    execvp(execArgs[0], execArgs);
    perror("exexv");
    exit(EXIT_FAILURE);
}

int main(){
    char* cmdLine = malloc(sizeof(char) * BUFFER_SIZE);
    int *lastStatus = 0;

    while (1){
        getcmd(cmdLine);
        cmdLine[strcspn(cmdLine, "\n")] = '\0';

        if(skipCmd(cmdLine) || isEcho(cmdLine)) continue;
        if(checkExit(cmdLine)) break;

        fflush(stdout);

        struct command *cmd = createCmd(cmdLine);
        // printCmd(cmd);
        if(isBuiltIn(cmd, lastStatus)) continue;
        
        runCmd(cmd);

        free(cmd);
    }

    free(cmdLine);
    return EXIT_SUCCESS;
}