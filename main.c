#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/wait.h>

char projectRootDirectory[1024];


int inputHistory(char* str){
    char* input;
    //printf("%s ",cwd);
    input = readline("$> ");
    if(strlen(input)==0){
        return 0;
    }
    strcpy(str, input);
    add_history(str);
    free(input);
    return 1;
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
void splitCommand(char* command, char** parsedCommand,int* parsedCommandLength) {
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
    }
}
void parsePipes(char** parsedCommand,char parsedCommandWithPipes[20][20][20],int parsedCommandLength){
    int j = 0;  //function to parse the '|' symbols in the command array, placing each command on a different row of a matrix

    for(int i = 0;i<parsedCommandLength;i++){
        int k;
        if(strcmp(parsedCommand[i],"|") == 0){
            k = 0;
            j++;
        }
        else{
            strcpy(parsedCommandWithPipes[j][k],parsedCommand[i]);
            k++;
        }

    }
}
int main(int argc, char **argv) {

    getcwd(projectRootDirectory, sizeof(projectRootDirectory)); //the initial location from where you ran the program
    char input[256];
    int pid;
    addToPATH();
    //char ** commandOutputBuffer;
    system("clear");

    for (;;) {
        //char cwd[1024];
        char parsedCommandWithPipes[20][20][20] = {NULL};
        //getcwd(cwd,sizeof(cwd));
        inputHistory(input);
        char *parsedCommand[100] = {NULL};
        int parsedCommandLength;
        int *commandPointer = &parsedCommandLength;

        splitCommand(input,parsedCommand,commandPointer);
        parsePipes(parsedCommand,parsedCommandWithPipes,parsedCommandLength);
//        for(int i = 0;i<20;i++)
//            for(int j = 0;j<20;j++)
//                //if(strcmp(parsedCommandWithPipes[i][j],"")!=0)
//                 printf("p[%d][%d] = %s \n",i,j,parsedCommandWithPipes[i][j]);
//       // printf("%d",parsedCommandLength);
        if(strcmp(parsedCommand[0],"exit") == 0)
            exit(0);
        if((pid = fork())< 0 ){
            perror("Error when creating the fork.");
        }
        if(pid == 0){
            executeCommands(parsedCommand);
        }
        else{
            wait(NULL);
            continue;
    }

    return 0;

}
}







