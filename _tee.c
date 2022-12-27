#include <getopt.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int optarg;
    int aflag = 0, hflag = 0;
    while ((optarg = getopt(argc, argv, "a")) != -1) {
        switch (optarg) {
            case 'a':
                aflag = 1;
                break;
            case 'h':
                hflag = 1;
                break;
            default:
                printf("oops");
                break;
        }

    }
    if(hflag != 0){
        printf("Usage: tee [OPTION]... [FILE]...");
    }
    else if(aflag != 0) { printf("-a\n");}
    else{
    printf("%s",argv[2]);

    }
}
