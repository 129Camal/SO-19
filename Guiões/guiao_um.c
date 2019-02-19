#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

//Exercicio 1 - Ficheiro de 10Mb
void tenMB(char* filename){
    int i, fd;
    
    //0666
    fd = open(filename, O_CREAT|O_RDWR, 0666);

    for(i=0; i < 1024*1024*10; i+=10){
        write(fd, "aaaaaaaaaa", 10);
    }
}

//Exercicio 2 - MyCat (caracter a caracter)
void mycat_v1(){
    char c;

    while(read(0, &c,1) == 1){
        write(1, &c, 1); 
    }
}

//Exercicio 3 - MyCat (N bytes)
void mycat_v2(size_t nbytes){
    char c[nbytes];
    ssize_t res;

    while((res = read(0, &c, nbytes)) > 0){
        write(1, &c, res); 
    }
}

/*Exercicio 4 

Basicamente, ao aumentar o tamanho estamos a diminuir o número de acesso à memória logo, 
o tempo de execução será muito menor.

*/

//Exercicio 5 - Implementação da readln (caracter a caracter)
ssize_t readln(int fildes, void* buf, size_t nbyte){
    ssize_t total_char = 0;

    while(read(fildes, buf+total_char, nbyte) == 1 && ((char*)buf)[total_char] != '\n'){
        total_char++;
    }
    
    if(((char*)buf)[total_char] == '\n') total_char++;

    return total_char;
}

//Exercicio 6 - nl (linhas recebidas do stdin ou ficheiro)
void nl(char** argv){
    char s[1024];
    int nlines = 1;
    int fd = 0;

    if(argv[2]) fd = open(argv[2], O_RDONLY);
    
    while(readln(fd, s, 1) > 0){
        printf("\t %d %s\n", nlines++, s);
    }

    close(fd);
}

//Exercicio 8 - mycat (ficheiros como argumentos)
void mycat_v3(char** argv){
    char c[1024];
    int i, fd = 0;
    ssize_t res;
    for(i = 2; argv[i]; i++){
        fd = open(argv[i], O_RDWR);
        while((res = readln(fd, c, 1)) > 0){
            write(1, c, res);
        }
        close(fd);
        write(1, "\n", 1);
    }
    while((res = read(0, c, 1024)) > 0){
        write(1, c, res); 
    }
}

//Exercicio 9 - myhead
//PERGUNTA AO PROF SE PODEMOS UTILIZAR DE VEZ EM QUANDO O PRINTF PORQUE O WRITE ÀS VEZES É BASTANTE LIXO
void myhead(char** argv){
    char c[1024];
    int fd = 0, i;
    ssize_t res;

    if(argv[2] && !argv[3]){
        fd = open(argv[2], O_RDWR);
        while((res = readln(fd, c, 1)) > 0){
            write(1, c, res);
        }
        write(1, "\n", 1);
        close(fd);
    }else{
        for(i = 2; argv[i]; i++){
            write(1, "==> ", 4);
            write(1, argv[i], strlen(argv[i]));
            write(1, " <==\n", 5);
            fd = open(argv[i], O_RDWR);
            while((res = readln(fd, c, 1)) > 0){
                write(1, c, res);
            }
            write(1, "\n", 1);
            close(fd);
        } 
    }
}

//Exercicio 10 - myGrep
/*void myGrep(char** argv){
    int i;

    for(i = 3; argv[i]; i++){
        fd = open(argv[i], O_RDWR);
        while((res = readln(fd, c, 1)) > 0){
            write(1, c, res);
        }
        close(fd);
        write(1, "\n", 1);
    }
}
*/


int main(int argc, char** argv){

    if(strcmp(argv[1], "1")==0) tenMB(argv[2]);
    if(strcmp(argv[1], "2")==0) mycat_v1();
    if(strcmp(argv[1], "3")==0) mycat_v2(atoi(argv[2]));
    if(strcmp(argv[1], "5")==0){
        int fd = open("teste.txt", O_RDWR);
        char s[1024];
        printf("%zd\n", readln(fd, s, 1));
        close(fd);
    }
    if(strcmp(argv[1], "6")==0) nl(argv);
    if(strcmp(argv[1], "8")==0) mycat_v3(argv);
    if(strcmp(argv[1], "9")==0) myhead(argv);
    return 0;
}