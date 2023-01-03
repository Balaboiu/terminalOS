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
    }
}
void parsePipes(char** parsedCommand,char parsedCommandWithPipes[20][20][20],int parsedCommandLength,int *numberPointer){
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
        *numberPointer = j;
    }
}
void executePipedCommands(char parsedCommandWithPipes[20][20][20], int numberOfPipes){
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
                executeCommands((char**)parsedCommandWithPipes[i]);
            }

        }
        else{
            for (int j = 0; j < numberOfPipes * 2; j++)
                close(pipeFD[j]);
            for(int i = 0;i<numberOfPipes;i++)
                wait(NULL);

        }
    }
}
//void removeNulls(char** parsedCommand,char** parsedCommandAfterRemoval){
//    int i = 0;
//
//    while(strcmp(parsedCommand[i],NULL) != 0){
//        strcpy(parsedCommandAfterRemoval[i],parsedCommand[i]);
//        i++;
//    }
//
//};
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
        int parsedCommandLength,*commandPointer = &parsedCommandLength;
        int numberOfPipes = 0, *numberPointer = &numberOfPipes;

        splitCommand(input,parsedCommand,commandPointer);
        parsePipes(parsedCommand,parsedCommandWithPipes,parsedCommandLength,numberPointer);

//        for(int i = 0;i<20;i++)
//            for(int j = 0;j<20;j++)
//                //if(strcmp(parsedCommandWithPipes[i][j],"")!=0)
//                 printf("p[%d][%d] = %s \n",i,j,parsedCommandWithPipes[i][j]);

        if(strcmp(parsedCommand[0],"exit") == 0)
            exit(0);
        if((pid = fork())< 0 ){
            perror("Error when creating the fork.");
        }
        if(pid == 0){
            executeCommands(parsedCommand);
            //executePipedCommands(parsedCommandWithPipes,numberOfPipes);
        }
        else{
            wait(NULL);
            continue;
    }

    return 0;

}
}







