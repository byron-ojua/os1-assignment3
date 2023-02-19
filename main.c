#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define BUFFER_SIZE 256
#define CMD_EXIT "exit\n"

/**
 * @brief Retrieves cmd line input and puts it in a buffer
 * 
 * @param cmd Char string to put cmd in
 */
void getcmd(char *cmd){
    printf(": ");
    size_t bufsize = BUFFER_SIZE;
    getline(&cmd, &bufsize, stdin);
}


/**
 * @brief Checks if cmd input is exit
 * 
 * @param cmd char array to check
 * @return int 1 if true, else 0
 */
int checkExit(char *cmd){
    if(strcmp(CMD_EXIT, cmd) == 0){
        return 1;
    }

    return 0;
}


/**
 * @brief Checks is command should be skipped by checking if input is '\n' or '#'
 * 
 * @param cmd char array to check
 * @return int 1 is line should be skipped, else 0
 */
int skipCmd(char *cmd){
    if(strcmp("\n", cmd) == 0 || cmd[0] == '#'){
        return 1;
    }

    return 0;
}

void processCmd(char *cmd){
    printf("%s", cmd);
}


int main(){
    char* cmd = malloc(sizeof(char) * BUFFER_SIZE);
    while (1 == 1){
        getcmd(cmd);
        if(skipCmd(cmd)) continue;
        if(checkExit(cmd)) break;

        processCmd(cmd);
    }

    return EXIT_SUCCESS;
}