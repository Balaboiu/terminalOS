#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <fcntl.h>
#include <stdlib.h>

#define SIZE 512
char currentWorkingDirectory[1024];
char* fileDescriptors[2];

void getFileDescriptors(int argc,char** argv,char** fileDescriptors){
    int fdCount = 0;

    for(int i = 1; i<argc;i++){
        for(int j = 0; j < strlen(argv[i]);j++){
            if(argv[i][j] == '-')
                break;
            else if(j == strlen(argv[i])-1){
                fileDescriptors[fdCount] = argv[i];
                fdCount++;
            }
    }

}
};
int cp(){
    int fd1,fd2, n, length;
    char* buffer;


    if((fd1 = open(fileDescriptors[0], O_RDONLY)) < 0)
    {
        printf("Error opening the input file\n");
        return 2;
    }
    if((fd2 = open(fileDescriptors[1],  O_WRONLY | O_CREAT ,0664)) < 0)
    {
        printf("Error opening the output file\n");
        return 2;
    }
    length = lseek (fd1, 0, SEEK_END) - lseek(fd1, 0, SEEK_SET);
    buffer = (char *)malloc(length * sizeof(char));

    if(buffer == NULL){
        printf("Malloc failed.");
        return 3;
    }
    while( (n = read(fd1, buffer, length)) > 0)
    {
        buffer[n] = '\0';
        write(fd2,buffer,length);
        //printf("%s\n",buffer);
    }
    if( n < 0)
    {
        printf("Error reading the file\n");
        return 4;
    }
    free(buffer);
    close(fd1);
    close(fd2);

}
int main(int argc, char **argv){
//    getcwd(currentWorkingDirectory,sizeof(currentWorkingDirectory));
//    strcat(currentWorkingDirectory,"/");

    getFileDescriptors(argc,argv,fileDescriptors);

    int optarg;
    int iflag = 0, rflag = 0, tflag = 0,vflag = 0,hflag = 0;
    while((optarg = getopt(argc,argv,"irtvh")) != -1){
        switch(optarg){
            case 'i':
                iflag = 1;
                break;
            case 'r':
                rflag = 1;
                break;
            case 't':
                tflag = 1;
                break;
            case 'v':
                vflag = 1;
                break;
            case 'h':
                hflag = 1;
                break;
            default:
                printf("oops");
                break;
        }
    }
    if(hflag == 1){
        printf("HELP\n");
        return 0;
    }
    else if(iflag == 1){
        printf("cp: overwrite '%s'?",fileDescriptors[1]);
        getchar();
        cp();
    }
    else if(vflag == 1){
        printf("'%s' -> '%s'\n",fileDescriptors[0],fileDescriptors[1]);
        cp();
    }
    else if(iflag == 0 && rflag == 0 && tflag == 0 && vflag == 0 && hflag == 0){

        cp();
    }
}