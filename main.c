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
void addToPATH(){
    char currentPath[1024] = "PATH="; //we need to add the commands to the PATH to be able to execute them
    char* env = getenv("PATH");

    strcat(env,":");
    strcat(env,projectRootDirectory);
    strcat(env,"/bin");
    strcat(currentPath,env);
    putenv(currentPath);
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
    //addToPATH();
    strcpy(customCommandLocation,projectRootDirectory); //create the path to the chosen custom command
    strcat(customCommandLocation,"/bin/");
    strcat(customCommandLocation,parsedCommand[0]);

    strcpy(builtInCommandLocation,"/bin/");             //create the path to the chosen built in command
    strcat(builtInCommandLocation,parsedCommand[0]);

    if(strcmp(parsedCommand[0],"dirname") == 0){
        execvp(customCommandLocation,parsedCommand);
    }
    else if(strcmp(parsedCommand[0],"cp") == 0){
        execvp(customCommandLocation,parsedCommand);
    }
    else if(strcmp(parsedCommand[0],"tee") == 0){
        execvp(customCommandLocation,parsedCommand);
    }
    else{
        execvp(builtInCommandLocation,parsedCommand);
       // system(parsedCommand);
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
        else{
            commandChunk[i] = parsedCommand[i];
            //memcpy(commandChunk[i],parsedCommand[i],sizeof(*parsedCommand[i]));
            //printf("%s\n",commandChunk[i]);

            globCounter++;
        }

    }
    return LAST;
}


int countPipesAndRedirects(char** parsedCommand,int parsedCommandLength) {
    int count = 0;
    for (int i = 0; i < parsedCommandLength; i++) {
        if (strcmp(parsedCommand[i], ">") == 0 || strcmp(parsedCommand[i], ">>") == 0 ||
            strcmp(parsedCommand[i], "|") == 0) {
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
        for (int i = 0; i < count;i++)
            pipe(pipeFD[i]);

        for (int i = 0; i <= count; i++) {

            int status = getCommandChunk(parsedCommand + globCounter, parsedCommandLength - globCounter);
            //printf("prevStatus: %d status: %d\n",prevStatus,status);


            //pipe<(pipeFD);
            if ((pid[i] = fork()) < 0) {
                perror("Error when creating the fork.");
            }
            if (pid[i] == 0) {
                if (status == PIPE && i == 0) {
                    dup2(pipeFD[0][1], 1);
                    for (int j = 0; j < count; j++) {
                        close(pipeFD[j][0]);
                        close(pipeFD[j][1]);
                    }
                    executeCommands(commandChunk);

                }
                else if ((status == PIPE || status == REDIRECT) && prevStatus == PIPE) {
                    /// middle pipe
                    dup2(pipeFD[i - 1][0], 0);
                    dup2(pipeFD[i][1], 1);
                    for (int j = 0; j < count; j++) {
                        close(pipeFD[j][0]);
                        close(pipeFD[j][1]);
                    }

                    executeCommands(commandChunk);

                }
                else if (status == LAST && prevStatus == PIPE) {
                    /// end pipe
                    dup2(pipeFD[i - 1][0], 0);
                    for (int j = 0; j < count; j++) {
                        close(pipeFD[j][0]);
                        close(pipeFD[j][1]);
                    }
                    executeCommands(commandChunk);


                }
                else if (status == REDIRECT || status == APPEND_REDIRECT) {
                   dup2(pipeFD[i-1][0],1);
                    for (int j = 0; j < count; j++) {
                        close(pipeFD[j][0]);
                        close(pipeFD[j][1]);
                    }

                }
                else if (status == LAST && prevStatus == REDIRECT) {
                    /// redirect
                    int fd = open(commandChunk[0],  O_WRONLY | O_CREAT ,0664);
//
                    dup2(fd,0);


                }
                else if (status == LAST && prevStatus == APPEND_REDIRECT) {
                    /// append redirect
//                    int fd = open(commandChunk[0],  O_WRONLY | O_APPEND | O_CREAT ,0664);
//                    dup2(fd,0);
//                    for (int j = 0; j < count * 2; j++)
//                        close(pipeFD[j]);

                }
                else {
                    printf("fuck\n");
                    break;
                }


            }
            else {
                for (int j = 0; j < count; j++) {
                    close(pipeFD[j][0]);
                    close(pipeFD[j][1]);
                }
                for(int i = 0;i<count;i++)
                    waitpid(pid[i],&stat,0);
                prevStatus = status;
            }
        }
    }
}

int main(int argc, char **argv) {

    getcwd(projectRootDirectory, sizeof(projectRootDirectory)); //the initial location from where you ran the program
    char input[256];
    int pid;
    addToPATH();

    system("clear");

    for (;;) {

        char* parsedCommandWithPipes[20][20];

        inputHistory(input);

        char *parsedCommand[100] = {NULL};
        int parsedCommandLength,*commandPointer = &parsedCommandLength;

        splitCommand(input,parsedCommand,commandPointer);
        int count = countPipesAndRedirects(parsedCommand,parsedCommandLength);

//        printf("%d \n",test);

        if(strcmp(parsedCommand[0],"exit") == 0)
            exit(0);
        if((pid = fork())< 0 ){
            perror("Error when creating the fork.");
        }
        if(pid == 0){
            executeCommandChains(parsedCommand,parsedCommandLength, count);
//                        for(int j = 0;j<parsedCommandLength;j++)
//                printf("%s ",commandChunk[j]);
           // free(commandChunk);

//            executeCommands(parsedCommandWithPipes[0]);

        }
        else{
            wait(NULL);
            printf("\n");

            continue;
    }

    return 0;

}
}







