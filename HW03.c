#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* 
Name: Brandon Pham
BlazerId: vpham
Project #: 3
To compile: <instructions for compiling the program> 
just use make and make run from my makefile
To run: <instructions to run the program> 
    "./HW3 <flag> <Dir>"
        i.e "./HW3 -f" ".c 4" -S -t f"
*/ 
#define TRUE 1
#define MAX_PATH_SIZE 3000 //"Love you 3000 -Ironman"
#define FALSE 0

typedef struct sixFlags
{
    int f1; // "S"
    int f2; // "s"
    int ff2;
    int f3; // "f"
    char *ff3; 
    int deep;
    int f4; // "t"
    char *ff4;
    int f5; // "e"
    int f6; // "E"
} sixFlags;

sixFlags* init_sixFlags(sixFlags *six)
{
    six->f1 = FALSE;
    six->f2 = FALSE;
    six->ff2 = 0;
    six->f3 = FALSE;
    six->ff3 = NULL;
    six->deep = 0;
    six->f5 = FALSE;
    six->f6 = FALSE;

    return six;
}
int fileExplore(char *fileName, struct stat *stats, sixFlags flags, int tabSpaces)
{
    if (flags.f2 && !(stats->st_size <= flags.ff2))
    {
        return FALSE;
    }
    if (flags.f3 && (strstr(fileName, flags.ff3) == NULL || !(tabSpaces <= flags.deep))) 
    {
        return FALSE;
    }
    if (flags.f4 && (strcmp(flags.ff4, "f") == 0) && ((stats->st_mode & S_IFMT) != S_IFREG)) 
    {
        return FALSE;
    }
    if (flags.f4 && (strcmp(flags.ff4, "d") == 0) && ((stats->st_mode & S_IFMT) != S_IFDIR)) 
    {
        return FALSE;
    }

    return TRUE;
}
typedef void holder(char *fileName, struct stat *stats, int *count, int tabSpaces, sixFlags flags);

void newPrint(char *fileName, struct stat *stats, int *count, int tabSpaces, sixFlags flags)
{
    if (!flags.f1) 
    {
        printf("%*s[%d] %s\n", 4 * tabSpaces, "", *count, fileName); 
    }
    else 
    {
        if ((stats->st_mode & S_IFMT) == S_IFDIR) 
        {
            printf("%*s[%d] %s %d Bytes, %o, %s\n", 4 * tabSpaces, "", *count, fileName, 0, stats->st_mode & 0777, ctime(&stats->st_mtime)); 
        }
        else 
        {
            printf("%*s[%d] %s %lld Bytes, %o, %s\n", 4 * tabSpaces, "", *count, fileName, stats->st_size, stats->st_mode & 0777, ctime(&stats->st_mtime));
        } 
    }
    *(count) += 1;
}
void directTravel(char *path, int tabSpaces, sixFlags flags, holder funcPtr, char *com_e, char **separate, int numE)
{
   struct dirent *dirent;
    struct stat stats;
    DIR *parentDir;

    if (tabSpaces == 0) 
    {
        printf("Starting Directory: %s\n", path);
    }

    parentDir = opendir(path);
    if (parentDir == NULL) 
    { 
        printf ("Error opening directory '%s'\n", path); 
        exit(-1);
    }

    int count = 1; 
    while((dirent = readdir(parentDir)) != NULL)
     {

        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) 
        {
            continue;
        }

        char *filePath = (char *) malloc(MAX_PATH_SIZE);
        strcpy(filePath, path);
        strcat(filePath, "/");
        strcat(filePath, dirent->d_name);

        if (stat(filePath, &stats) == -1) 
        {
            printf("Error with stat '%s\n", filePath);
            exit(-1);
        }

        if (fileExplore(dirent->d_name, &stats, flags, tabSpaces)) 
        {
            funcPtr(dirent->d_name, &stats, &count, tabSpaces, flags);
        }

        if (dirent->d_type == DT_DIR) 
        {
            if(numE != 0){
                strcpy(separate[numE++], "-C");
                strcpy(separate[numE++], dirent->d_name);
            }
            directTravel(filePath, tabSpaces + 1, flags, funcPtr, com_e, separate, numE);
            if(numE != 0){
                strcpy(separate[numE++], "-C");
                strcpy(separate[numE++], "../");
            }
        }

        //-E
        if(flags.f6 == 1){
            separate[numE++] = dirent->d_name;
        }

        //-e
        if(flags.f5 == 1){
            int status;
            char *segment = malloc(sizeof(char) * BUFSIZ);
            int pid = fork();
            if(pid == 0){
                if (dirent->d_type == DT_DIR) 
                {
                    kill(getpid(), SIGKILL);
                }
                char *comlist[10];
                for(int i = 0; i < 50; i++)
                {
                    comlist[i] = malloc(sizeof(char)*BUFSIZ);
                }
                segment = strtok(com_e, " ");
                int num = 0;
                while(segment != NULL)
                {
                    strcpy(comlist[num++], segment);
                    segment = strtok(NULL, " ");
                }
                comlist[num++] = dirent->d_name;
                comlist[num] = NULL;
                execvp(comlist[0], comlist);
            }else{
                wait(&status);
            }
        }

        free(filePath);

    }
    if(flags.f6 == 1){
        printf("\nBelow are the files that save into the tar file\n");
        separate[numE] = NULL;
        execvp(separate[0], separate);
        separate[numE++] = dirent->d_name;
    }
    
    closedir(parentDir); 
}
int main(int argc, char **argv) 
{

    int opt;
    sixFlags flags;
    char *com_e = malloc(sizeof(char)*BUFSIZ);
    char *com_E = malloc(sizeof(char)*BUFSIZ);

    init_sixFlags(&flags);

    while ((opt = getopt(argc, argv, "Ss:f:t:e:E:")) != -1) 
    {
        switch(opt) 
        {
            case 'S':
                flags.f1 = 1;
                break;
            case 's':
                flags.f2 = 1;
                flags.ff2 = atoi(optarg);
                break;
            case 'f':
                flags.f3 = 1;
                flags.ff3 = strtok(optarg, " ");
                flags.deep = atoi(strtok(NULL, " "));
                break;
            case 't':
                flags.f4 = 1;
                flags.ff4 = optarg;
                break;
            case 'e':
                flags.f5 = 1;
                strcpy(com_e, optarg);
                break;
            case 'E':
                flags.f6 = 1;
                strcpy(com_E, optarg);
        }
    }

    // -E separate command
    char *separate[50];
    for(int i = 0; i < 50; i++)
    {
        separate[i] = malloc(sizeof(char)*BUFSIZ);
    }
    int numE = 0;
    char *copy = strtok(com_E, " ");
    while(copy != NULL){
        strcpy(separate[numE++], copy);
        copy = strtok(NULL, " ");
    }

    if (optind == argc) 
    {
        directTravel(".", 0, flags, &newPrint, com_e, separate, numE);
    }
    else 
    {
        directTravel(argv[argc - 1], 0, flags, &newPrint, com_e, separate, numE);
    }

    return 0;
}
