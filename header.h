#ifndef HEADER_H
#define HEADER_H

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

struct command{
    char cmd[BUFFER_SIZE];
    int background;
    int in;
    char input[BUFFER_SIZE];
    int out;
    char output[BUFFER_SIZE];
    char args[512][BUFFER_SIZE];
    int numArgs;
};

void getcmd(char *cmdLine);
int checkExit(char *cmdLine);
int isEcho(char *cmdLine);
void printCmd(struct command *cmd);
struct command *createCmd(char *cmdLine);
int isBuiltIn(char *cmdLine, int *status);
void setIOStreams(struct command *cmd);
void runCmd(char *cmdLine, int *lastStatus);
void expandInput(char *buffer, char *cmdLine, int parentPid);
void catchSIGTSTP(int signo);

#endif // !HEADER_H