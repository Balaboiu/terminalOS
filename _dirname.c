#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv){

    int c;
    int hflag = 0;
    while ((c = getopt(argc, argv, "h")) != -1){
        switch(c){
            case 'h':
                hflag = 1;
                break;
            default:
                perror("Something went wrong\n");
                break;

        }
    }
    if(hflag != 0){
        printf("Usage: dirname [OPTION] NAME...\n");
    }
    else {
        for (int i = 1; i < argc; i++)
        {
            char *copy = strdup(argv[i]);   //make a copy of the parameter, to compare later
            int length = strlen(argv[i]);

            if(argv[i][length] == '/')
                length--;
            for (int j = length; j > 0; j--)  //iterate from the last element to the first one
            {
                if (argv[i][j] == '/') {       //until you reach a '/'

                    argv[i][j] = '\0';         //change the character to the terminator, to truncate the string

                    break;
                }
            }

            if (strcmp(argv[i], copy) == 0) {
                printf(".\n");
            }
            else {
                printf("%s\n", argv[i]);
            }
            free(copy);
            }

        }



    return 0;
}