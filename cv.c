#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// Função para ler um parágrafo
ssize_t readln(int fildes, char* buf){
    ssize_t total_char = 0, r;

    while((r = read(fildes, buf + total_char, 1)) > 0){
        if(buf[total_char++] == '\n') break;
    }

    return total_char;
}

int main(int argc, char** argv){
    
    // Abrir o canal de comunicação com o servidor
    int server = open("./communicationFiles/server", O_WRONLY);
    
    ssize_t res;
    pid_t pid;
    char address[512];
    
    // Criar o canal de comunicação para ouvir do servidor
    sprintf(address, "./communicationFiles/%d", getpid());
    mkfifo(address, 0666); 

    // Fazer um processo filho para lidar com a informação recebida do servidor
    if((pid = fork()) == 0){
        
        char c[1024];
           
        ssize_t r; 

        // Abrir o canal de comunicação para ouvir do servidor
        int mailbox = open(address, O_RDONLY);

        // Ouvir e escrever o que o servidor devolve
        while((r = readln(mailbox, c)) >= 0){
            write(1, &c, r);
        }

        _exit(0);
    }

    char c[1024];
    char writeAux[1024];

    // Ouvir o que o utilizador quer perguntar ao servidor
    while((res = read(0, &c, 512)) > 0){
        c[res] = '\0';
        sprintf(writeAux, "%d %s", getpid(), c);
        write(server, writeAux, strlen(writeAux));
    }

    // No final, apagar o ficheiro fifo que foi criado para o servidor comunicar com o cliente
    execlp("rm", "rm", address, NULL);
}