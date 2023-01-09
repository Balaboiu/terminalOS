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

    strcpy(customCommandLocation,projectRootDirectory); //create the path to the chosen custom command
    strcat(customCommandLocation,"/bin/");
    strcat(customCommandLocation,parsedCommand[0]);

    strcpy(builtInCommandLocation,"/bin/");             //create the path to the chosen built in command
    strcat(builtInCommandLocation,parsedCommand[0]);

    if(strcmp(parsedCommand[0],"dirname") == 0){
        execv(customCommandLocation,parsedCommand);
    }
    else if(strcmp(parsedCommand[0],"cp") == 0){
        execv(customCommandLocation,parsedCommand);
    }
    else if(strcmp(parsedCommand[0],"tee") == 0){
        execv(customCommandLocation,parsedCommand);
    }
    else{
        execv(builtInCommandLocation,parsedCommand);
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
/*void executePipedCommands(char* parsedCommandWithPipes[20][20], int numberOfPipes){
    int pipeFD[2*numberOfPipes];
    int pid;
    for(int i  = 0; i < 2*numberOfPipes; i += 2)
        pipe(&pipeFD[i]);

    for(int i = 0; i < numberOfPipes+1;i++) {

        if((pid = fork() )< 0){
            perror("Fork failed.");
        }
        else if (pid == 0) {
            printf("Count: %d\n",i);
            if (i == 0) {     //the first command in the chain

                dup2(pipeFD[1], 1); // output the command to the pipe, not to standard output

                for (int j = 0; j < numberOfPipes * 2; j++)

                    close(pipeFD[j]);
                printf("test 1: %s\n",parsedCommandWithPipes[0]);
                executeCommands((char**)parsedCommandWithPipes[i]);
            }
            else if (i == numberOfPipes) {    //the last command in the chain
                dup2(pipeFD[(numberOfPipes * 2) - 2], 0); //get the input from the last pipe, not from standard input

                for (int j = 0; j < numberOfPipes * 2; j++)
                    close(pipeFD[j]);
                printf("test 2: %s\n",parsedCommandWithPipes[i]);
                executeCommands((char**)parsedCommandWithPipes[i]);
            }
            else{
                dup2(pipeFD[(i-1)*2],0); // change the input to the previous pipe
                dup2(pipeFD[(i*2)+1],1); // change the output to the next pipe

                for (int j = 0; j < numberOfPipes * 2; j++)
                    close(pipeFD[j]);
                printf("test 3: %s\n",parsedCommandWithPipes[i]);
                executeCommands(parsedCommandWithPipes);
            }

        }
        else{
            for (int j = 0; j < numberOfPipes * 2; j++)
                close(pipeFD[j]);
            for(int i = 0;i<numberOfPipes;i++)
                wait(NULL);

        }
    }
}*/

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
        int pipeFD[count * 2];

        for (int i = 0; i < count * 2; i += 2)
            pipe(pipeFD + i);
        int prevStatus = 0;

        for (int i = 0; i <= count; i++) {

            int status = getCommandChunk(parsedCommand + globCounter, parsedCommandLength - globCounter);
            //printf("prevStatus: %d status: %d\n",prevStatus,status);
            int pid;
            pipe(pipeFD);
            if ((pid = fork()) < 0) {
                perror("Error when creating the fork.");
            }
            if (pid == 0) {
                if (status == PIPE) {
                    dup2(pipeFD[1], 1);
                    //                    close(pipeFD[0]);
                    //                    close(pipeFD[1]);
                    for (int j = 0; j < count * 2; j++)
                        close(pipeFD[j]);
                    executeCommands(commandChunk);

                    //free(commandChunk);
                }
                    // continue;
                else if ((status == PIPE || status == REDIRECT) && prevStatus == PIPE) {
                    /// middle pipe
                    dup2(pipeFD[(i-1)*2],0);
                    dup2(pipeFD[(i*2)+1],1);
                    for (int j = 0; j < count * 2; j++)
                        close(pipeFD[j]);
                    executeCommands(commandChunk);

                }
                else if (status == LAST && prevStatus == PIPE) {
                    /// end pipe
                    dup2(pipeFD[(2 * count) - 2], 0);
                    //                    close(pipeFD[0]);
                    //                    close(pipeFD[1]);
                    for (int j = 0; j < count * 2; j++)
                        close(pipeFD[j]);
                    executeCommands(commandChunk);


                }
                else if(status == REDIRECT || status == APPEND_REDIRECT){
                    dup2(pipeFD[1],1);
                    for (int j = 0; j < count * 2; j++)
                    close(pipeFD[j]);

                }
                else if (status == LAST && prevStatus == REDIRECT) {
                    /// redirect
                   int fd = open(commandChunk[0],  O_WRONLY | O_CREAT ,0664);
                   dup2(fd,0);
                    for (int j = 0; j < count * 2; j++)
                        close(pipeFD[j]);




                }
                else if (status == LAST && prevStatus == APPEND_REDIRECT) {
                    /// append redirect
                    int fd = open(commandChunk[0],  O_WRONLY | O_APPEND | O_CREAT ,0664);
                    dup2(fd,0);
                    for (int j = 0; j < count * 2; j++)
                        close(pipeFD[j]);
                }
                else {
                    printf("fuck\n");
                    break;
                }
            }
            else{
                for(int j = 0; j < count*2;j++)
                    close(pipeFD[j]);
                wait(NULL);
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







