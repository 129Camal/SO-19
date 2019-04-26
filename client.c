#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include "dynamicDoubleArray.c"

// Função para ler um parágrafo
ssize_t readln(int fildes, char* buf){
    ssize_t total_char = 0, r;

    while((r = read(fildes, buf + total_char, 1)) > 0){
        if(buf[total_char++] == '\n') break;
    }

    return total_char;
}

int main(int argc, char** argv){
    
    int server = open("./communicationFiles/server", O_WRONLY);
    ssize_t res;
    pid_t pid;
    char address[512];
    
    sprintf(address, "./communicationFiles/%d", getpid());
    mkfifo(address, 0666); 

    if((pid = fork()) == 0){
        
        char c[1024];
        char writeAux[1024];
           
        ssize_t r; 

        int mailbox = open(address, O_RDONLY);

        while((r = readln(mailbox, c)) >= 0){
            write(1, &c, r);
        }

        _exit(0);
    }

    char c[1024];
    char writeAux[1024];


    while((res = read(0, &c, 512)) > 0){
        c[res] = '\0';
        sprintf(writeAux, "c %d %s", getpid(), c);
        write(server, writeAux, strlen(writeAux));
    }
}