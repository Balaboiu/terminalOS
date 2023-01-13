#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFSIZE 4096

char* fileDescriptors[50];
int counter = 0;
char *buffer;

int getFileDescriptors(int argc,char** argv){
    int fdCount = 0;

    for(int i = 1; i<argc;i++){
        if(strcmp(argv[i],"<") == 0){
            i++;
            break;
            }
        for(int j = 0; j < strlen(argv[i]);j++){
            if(argv[i][j] == '-')
                break;

            else if(j == strlen(argv[i])-1){
                fileDescriptors[fdCount] = argv[i];
                fdCount++;
            }

        }

    }
    return fdCount;
};
int createInputBuffer(){
    int bsize;
    int i =0;
    buffer = malloc(BUFSIZE);
    char* content[BUFSIZE + 1];
    while ((bsize = fread(content, 1, BUFSIZE, stdin))){
        buffer = realloc(buffer,2*bsize);
        if(i==0){
        memcpy(buffer,content,bsize);
        i++;
        }
        else{
            memcpy(buffer+BUFSIZE,content,bsize);
        }

    };

}
void tee_a(char* fileDescriptor){

    FILE *fp = fopen(fileDescriptor, "a+");

    if (fp == 0)
        printf("Failed to open file.\n");

    int bsize = strlen(buffer);

    fwrite(buffer, bsize, 1, fp);
        if(counter == 0)
        {
            fwrite(buffer, bsize, 1, stdout);
            printf("\n");
        }


    fclose(fp);
}


void tee(char* fileDescriptor){


    FILE *fp = fopen(fileDescriptor, "w");

    if (fp == 0)
        printf("Failed to open file.\n");

    int bsize = strlen(buffer);

    fwrite(buffer, bsize, 1, fp);
    if(counter == 0)
    {
        fwrite(buffer, bsize, 1, stdout);
        printf("\n");
    }


    fclose(fp);
    counter++;
}

int main(int argc, char **argv) {
    int fdCount = getFileDescriptors(argc,argv);
    int c;
    int aflag = 0,hflag = 0;
    int length,n;

    while ((c = getopt(argc, argv, "ha")) != -1){
        switch(c){
            case 'a':
                aflag = 1;
                break;
            case 'h':
                hflag = 1;
                break;
            default:
                break;
        }

    }

    if(hflag == 1){
        printf("Usage: tee [OPTION]... [FILE]... < [INPUT_FILE]\n");
        return 0;
    }
    else if(aflag == 1){
        createInputBuffer();
        for(int i = 0; i < fdCount; i++)
            tee_a(fileDescriptors[i]);
    }

    else{
        createInputBuffer();
        for(int i = 0; i < fdCount; i++)
            tee(fileDescriptors[i]);
    }
//free(buffer);

}
