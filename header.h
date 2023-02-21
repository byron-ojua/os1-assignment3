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
    char *cmd;
    int background;
    int in;
    char *input;
    int out;
    char *output;
    char args[512][BUFFER_SIZE];
    int numArgs;
};

void getcmd(char *cmdLine);
int checkExit(char *cmdLine);
int skipCmd(char *cmdLine);
int isEcho(char *cmdLine);
void printCmd(struct command *cmd);
struct command *createCmd(char *cmdLine);
int isBuiltIn(char *cmdLine, int *status);
void setIOStreams(struct command *cmd);
void runCmd(char *cmdLine, int *lastStatus);

#endif // !HEADER_H