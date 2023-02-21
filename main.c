#include "header.h"

// Exit status code video https://www.youtube.com/watch?v=DiNmwwQWl0g
int main(){
    int pid = getpid();
    int lastStatus = 0;

    // From Exploration: Signal Handling API - Ignore ^C (SIG_IGN)
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
        // Get command line input
        char* cmd_buffer = malloc(sizeof(char) * BUFFER_SIZE);
        getcmd(cmd_buffer);
        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

        char* cmdLine = expandInput(cmd_buffer, pid);
        char* cmdCopy = malloc(sizeof(char) * BUFFER_SIZE);
        strcpy(cmdCopy, cmdLine);
        free(cmd_buffer);

        // Check if input a blank, comment, echo, or ecit command
        if(skipCmd(cmdLine) || isEcho(cmdLine)) continue;
        if(checkExit(cmdLine)) break;
        fflush(stdout);

        // If built i nfunction, run. Else run runCmd
        if(isBuiltIn(cmdLine, &lastStatus)){
            fflush(stdout);
            continue;
        } 
        else {
            free(cmdLine);
            runCmd(cmdCopy, &lastStatus);
        }

        fflush(stdout);
        // free(cmdLine);
        free(cmdCopy);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Retrieves cmd line input and puts it in a buffer
 * 
 * @param cmdLine Char string to store input
 */
void getcmd(char *cmdLine){
    printf(": ");
    fflush(stdout);
    size_t bufsize = BUFFER_SIZE;
    getline(&cmdLine, &bufsize, stdin);
}

char * expandInput(char *buffer, int parentPid){
    char *new = malloc(sizeof(char) * BUFFER_SIZE);
    char pidStr[15];
    sprintf(pidStr, "%d", parentPid);

    for(int i = 0; i <= strlen(buffer) - 1; i++){
        if (buffer[i] == '$' && buffer[i+1] == '$')
        {
            strncat(new, pidStr, strlen(pidStr));
            i++;
        } else {
            char cpyChar[2] = "\0";
            cpyChar[0] = buffer[i];
            strncat(new, cpyChar, strlen(cpyChar));
        }
        
    }

    return new;
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
        fflush(stdout);
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
 * @param cmdLine char array to check
 * @return int 1 if true, else 0 
 */
int isEcho(char *cmdLine){
    // Check if command line is an echo command
    if(strncmp(CMD_ECHO, cmdLine, strlen(CMD_ECHO)) == 0){
        // Echo message if one is present, otherwise new line
        if(strlen(cmdLine) > 5){
            printf("%s\n", &cmdLine[5]);
            fflush(stdout);
        } else {
            printf("\n");
            fflush(stdout);
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

    // Loops and prints all arguments
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
  * @brief Checks if command is built in, and runs if so
  * 
  * @param cmdLine char array of params to build command
  * @param status int pointer to status var
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
        // print new CWD
        printf("CWD is %s\n", getcwd(dir, BUFFER_SIZE));
        fflush(stdout);
        free(dir);
        free(cmd);
        return 1;
    // Check if cmd is a status command
    } else if (strcmp(cmd->cmd,"status") == 0){
        printf("exit value %d\n", *status);
        fflush(stdout);
        free(cmd);
        return 1;
    }
    return 0;
}

/**
 * @brief Redirects input and output streams
 * 
 * @param cmd Command data struct containing params for stream redirects
 */
void setIOStreams(struct command *cmd){
    // If there is a specified input stream
    if (cmd->in == 1){
        int file = open(cmd->input, O_RDONLY);
        if(file == -1){
            printf("Cannot open %s for input\n", cmd->input);
            fflush(stdout);
            exit(1);
        } else {
            // Assign input file to input stream
            dup2(file, STDIN_FILENO);
            close(file);
        }
    }
    // If there is a specified output stream
    if (cmd->out == 1){
        int file = open(cmd->output, O_WRONLY | O_TRUNC | O_CREAT, 0777);
        if(file == -1){
            printf("Cannot write to %s for output\n", cmd->output);
            fflush(stdout);
            exit(1);
        } else {
            // Assign input file to output stream
            dup2(file, STDOUT_FILENO);
            close(file);
        }
    }
}

/**
 * @brief Runs and handles non built-in commands
 * 
 * @param cmdLine char array which contains data to mkae command
 * @param lastStatus int for childExitStatus
 */
void runCmd(char *cmdLine, int *lastStatus){
    // Code from Exploration: Process API - Monitoring Child Processes
    struct command *cmd = createCmd(cmdLine);
    pid_t spawnpid = -5;

    // Fork structure example from https://github.com/brentirwin/cs344-smallsh/blob/master/smallsh.c

    spawnpid = fork();
    
    switch (spawnpid){
        // If fork() fails
        case -1:
            perror("fork() failed!\n");
            exit(1);
            break;
        // If process is child
        case 0:
            // Redirect input files
            setIOStreams(cmd);

            // Create array for exec() elements
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

            // Run exec() process, and handle errors if it fails
            execvp(execArgs[0], execArgs);
            perror("exexvp");
            fflush(stdout);
            // *lastStatus = 1;
            exit(1);
            break;
        // Parent function
        default:
            // If child is a background process, do not wait for process exit and display childPID
            if(cmd->background){
                pid_t runpid = waitpid(spawnpid, lastStatus, WNOHANG);
                printf("background pid is %d\n", spawnpid);
                fflush(stdout);
            } else {
                // If forground process, wait for compleation
                pid_t runpid = waitpid(spawnpid, lastStatus, 0);
            }
            
            // Check for terminated processes, and what terminated the,
            while ((spawnpid = waitpid(-1, lastStatus, WNOHANG)) > 0) {
                printf("child %d terminated\n", spawnpid);
                fflush(stdout);
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
}