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
    if(strncmp(CMD_ECHO, cmdLine, strlen(CMD_ECHO)) == 0){
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

    char *cmdptr;
    char *cmdtoken = strtok_r(cmdLine, " ", &cmdptr);
    cmd->cmd = calloc(strlen(cmdtoken) + 1, sizeof(char));
    strcpy(cmd->cmd, cmdtoken);

    cmdtoken = strtok_r(NULL, " ", &cmdptr);

    while(cmdtoken != NULL){
        // printf("%s\n", cmdtoken);
        if (cmdtoken[0] == '&'){
            cmd->background = 1;
        } else if (cmdtoken[0] == '>'){
            cmdtoken = strtok_r(NULL, " ", &cmdptr);
            cmd->output = calloc(strlen(cmdtoken) + 1, sizeof(char));
            // printf("out is %s\n", cmdtoken);
            strcpy(cmd->output, cmdtoken);
        } else if (cmdtoken[0] == '<'){
            cmdtoken = strtok_r(NULL, " ", &cmdptr);
            cmd->input = calloc(strlen(cmdtoken) + 1, sizeof(char));
            // printf("out is %s\n", cmdtoken);
            strcpy(cmd->input, cmdtoken);
        } else {
            strcpy(cmd->args[cmd->numArgs], cmdtoken);
            cmd->numArgs = cmd->numArgs + 1;
        }
        // printf("reading\n");
        cmdtoken = strtok_r(NULL, " ", &cmdptr);
    }

    return cmd;
}

/**
 * @brief Checks if command is built in, and runs if so
 * 
 * @param cmd Command structure data
 * @return int 1 is built in, else 0
 */
int isBuiltIn(struct command *cmd){
    if(strcmp(cmd->cmd,"cd") == 0){
        char *dir = malloc(sizeof(char) * BUFFER_SIZE);
        if(cmd->numArgs >= 1){
            chdir(cmd->args[0]);
        } else {
            chdir("/home/");
        }
        
        printf("CWD is %s\n", getcwd(dir, BUFFER_SIZE));
        free(dir);
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

int main(){
    char* cmdLine = malloc(sizeof(char) * BUFFER_SIZE);

    while (1){
        getcmd(cmdLine);
        cmdLine[strcspn(cmdLine, "\n")] = '\0';

        
        if(skipCmd(cmdLine)) continue;
        if(checkExit(cmdLine)) break;
        if(isEcho(cmdLine)) continue;
        fflush(stdout);

        struct command *cmd = createCmd(cmdLine);
        // printCmd(cmd);
        if(isBuiltIn(cmd)) continue;
        
        // printf("Creating cmd\n");
        // free(cmd);
    }

    free(cmdLine);
    return EXIT_SUCCESS;
}