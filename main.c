#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PIPE 1
#define REDIRECT 2
#define APPEND_REDIRECT 3
#define LAST 4
#define REVERSE_REDIRECT 5


char projectRootDirectory[1024];
int globCounter = 0;
char** commandChunk = NULL;

void inputHistory(char* str){
    char* input;
    //printf("%s ",cwd);
    input = readline("$> ");
    if(strlen(input)!=0){


    strcpy(str, input);
    add_history(str);
   }
    free(input);

}
void splitCommand(char* command, char** parsedCommand,int* parsedCommandLength) { //bug when command contains a space between brackets
    int i = 0;
    while(command != NULL){
        parsedCommand[i] = strsep(&command," "); //split up the command on spaces, and add the separated string to an array
        i++;
    }
    *parsedCommandLength = i;
}
void executeCommands(char** parsedCommand){
    char customCommandLocation[1024];
    char builtInCommandLocation[512];
    int pid;

    strcpy(customCommandLocation,projectRootDirectory); //create the path to the chosen custom command
    strcat(customCommandLocation,"/bin/");
    strcat(customCommandLocation,parsedCommand[0]);

    strcpy(builtInCommandLocation,"/bin/");             //create the path to the chosen built in command
    strcat(builtInCommandLocation,parsedCommand[0]);

    if(strcmp(parsedCommand[0],"dirname") == 0){
        execvp(customCommandLocation,parsedCommand);
        exit(1);
    }
    else if(strcmp(parsedCommand[0],"cp") == 0){
        execvp(customCommandLocation,parsedCommand);
        exit(1);
    }
    else if(strcmp(parsedCommand[0],"tee") == 0){
        execvp(customCommandLocation,parsedCommand);
        exit(1);
    }
    else{
        execvp(builtInCommandLocation,parsedCommand);
        exit(1);

    }
}
int getCommandChunk(char** parsedCommand,int parsedCommandLength){

    commandChunk = (char**)malloc(4096*sizeof(char));

    for(int i = 0;i<parsedCommandLength;i++){

        if(strcmp(parsedCommand[i],"|") == 0){
            globCounter++;
            return PIPE;
        }
        else if(strcmp(parsedCommand[i],">") == 0){
            globCounter++;
            return REDIRECT;
        }
        else if(strcmp(parsedCommand[i],">>") == 0){
            globCounter++;
            return APPEND_REDIRECT;
        }
        else if(strcmp(parsedCommand[i],"<") == 0){
            globCounter++;
            return REVERSE_REDIRECT;
        }
        else{
            commandChunk[i] = parsedCommand[i];
            globCounter++;
        }

    }
    return LAST;
}
int countPipesAndRedirects(char** parsedCommand,int parsedCommandLength) {
    int count = 0;
    for (int i = 0; i < parsedCommandLength; i++) {
        if (strcmp(parsedCommand[i], ">") == 0 || strcmp(parsedCommand[i], ">>") == 0 ||
            strcmp(parsedCommand[i], "|") == 0 ||  strcmp(parsedCommand[i], "<") == 0)  {
            count++;
        }


    }
    return count;
}
void executeCommandChains(char** parsedCommand,int parsedCommandLength,int count) {

    if (count == 0) {

        executeCommands(parsedCommand);
    } else {
        int pipeFD[count][2];
        int stat;
        int pid[count + 1];

        int prevStatus = 0;
        for (int k = 0; k < count;k++)
            pipe(pipeFD[k]);

        for (int i = 0; i <= count; i++) {

            int status = getCommandChunk(parsedCommand + globCounter, parsedCommandLength - globCounter);

            if ((pid[i] = fork()) < 0) {
                perror("Error when creating the fork.");
            }
            if (pid[i] == 0) {

                if (status == PIPE) {
                continue;

                }
                else if (status == REDIRECT) {

                    char** oldCommandChunk = NULL;
                    oldCommandChunk = (char**)malloc(4096*sizeof(char));
                    oldCommandChunk = commandChunk;
                    getCommandChunk(parsedCommand + globCounter, parsedCommandLength - globCounter);
                    int fd;
                    if((fd = open(commandChunk[0],  O_WRONLY | O_TRUNC | O_CREAT ,0664)) < 0)
                    {
                        printf("Error opening the input file\n");
                        exit(1);
                    }
                    dup2(fd,1);

                    executeCommands(oldCommandChunk);

                }
                else if (status == APPEND_REDIRECT) {

                    char** oldCommandChunk = NULL;
                    oldCommandChunk = (char**)malloc(4096*sizeof(char));
                    oldCommandChunk = commandChunk;
                    getCommandChunk(parsedCommand + globCounter, parsedCommandLength - globCounter);
                    int fd = open(commandChunk[0],  O_WRONLY | O_APPEND | O_CREAT ,0664);
                    dup2(fd,1);
                    close(fd);
                    executeCommands(oldCommandChunk);

                }
                else if (status == REVERSE_REDIRECT) {

                    char** oldCommandChunk = NULL;
                    oldCommandChunk = (char**)malloc(4096*sizeof(char));
                    oldCommandChunk = commandChunk;
                    getCommandChunk(parsedCommand + globCounter, parsedCommandLength - globCounter);
                    int fd;
                    if((fd = open(commandChunk[0],  O_RDONLY)) < 0)
                    {
                        printf("Error opening the input file\n");
                        exit(1);
                    }
                    dup2(fd,0);
                    close(fd);
                    executeCommands(oldCommandChunk);

                }


                else {

                    break;
                }


            }
            else {

                for (int j = 0; j < count; j++) {
                    close(pipeFD[j][0]);
                    close(pipeFD[j][1]);
                }
                for(int k = 0;k<count;k++)
                    waitpid(pid[k],&stat,0);
                prevStatus = status;

            }
        }
    }
}

int main(int argc, char **argv) {

    getcwd(projectRootDirectory, sizeof(projectRootDirectory)); //the initial location from where you ran the program
    char input[256];
    int pid;


    system("clear");

    for (;;) {

        char* parsedCommandWithPipes[20][20];

        inputHistory(input);

        char *parsedCommand[100] = {NULL};
        int parsedCommandLength,*commandPointer = &parsedCommandLength;

        splitCommand(input,parsedCommand,commandPointer);
        int count = countPipesAndRedirects(parsedCommand,parsedCommandLength);


        if(strcmp(parsedCommand[0],"exit") == 0)
            exit(0);
        if(strcmp(parsedCommand[0],"cd") == 0){
            chdir(parsedCommand[1]);
        }
        if((pid = fork())< 0 ){
            perror("Error when creating the fork.");
        }
        if(pid == 0){
            executeCommandChains(parsedCommand,parsedCommandLength, count);
        }
        else{
            wait(NULL);
            printf("\n");

            continue;
    }

    return 0;

}
}