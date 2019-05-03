#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

// lseek(articles , 0, SEEK_SET);

// res = 1;

// while(read(articles, &code, 4) && read(articles, &namePosition, 4) && read(articles, &price, 8)){
                
//     printf("%d %d %.2f\n", code, namePosition, price);
                
// }

// Função para ler um parágrafo
ssize_t readln(int fildes, char* buf){
    ssize_t total_char = 0, r;

    while((r = read(fildes, buf + total_char, 1)) > 0){
        if(buf[total_char++] == '\n') break;
    }

    return total_char;
}

int main(int argc, char**argv){
    
    // Abrir os ficheiros necessários
    int articles = open("./files/ARTIGOS.bin", O_RDWR);
    int strings = open("./files/STRINGS.txt", O_RDWR);
    int server = open("./communicationFiles/server", O_WRONLY);

    ssize_t res;
    char c[1024];
    int nArticles = 1;

    // Leitura do ficheiro ARTIGOS para descobrir o número de artigos que tem
    while((res = read(articles, &c, 16)) > 15){
        
        nArticles++;
    
    }

    int code;
    char* name;
    double price = 0.0;
    char *token, *stopstring;
    char writeAux[1024];
    pid_t son;

    int status, namePosition;

    // Leitura do input por parte do utilizador
    while((res = read(0, &c, 512)) > 0){

        c[res-1] = '\0';
        
        token = strtok(c, " ");
        
        // Caso a instrução recebida comece por i (Inserir Novo Artigo)
        if(strcmp("i", token) == 0){
            lseek(articles, 0, SEEK_END);
            // Receção do nome do artigo
            token = strtok(NULL, " ");
            name = strdup(token);

            // Criação de um processo filho para tratar do nome;
            if((son = fork())== 0){
                lseek(strings, 0, SEEK_SET);
                char* articleName;
                int ref = 1;
                char aux[1024];
                
                // Percorrer os nomes
                while((res = readln(strings, c)) > 0){
                    c[res-1] = '\0';
                    
                    articleName = strtok(c, " ");
                    articleName = strtok(NULL, " ");
                    
                    // Verificar se o nome já existe, se sim, retorna o indice onde está
                    if(strcmp(articleName, name)==0){
                        _exit(ref);
                        break;
                    }
                    ref++;
                }

                // Caso o nome não exista criar o novo
                sprintf(aux, "%d %s\n", ref, name);
                write(strings, &aux, strlen(aux));
                _exit(ref);

            }

            // Receção do preço do artigo
            token = strtok(NULL, " ");
            price = strtod(token, &stopstring);

            // Esperar pela morte do filho
            son = wait(&status);
            namePosition = WEXITSTATUS(status);

            // Escrever os dados
            write(articles, &nArticles, 4);
            write(articles, &namePosition, 4);
            write(articles, &price, 8);

            // Escrever código para o ecrâ
            sprintf(writeAux, "Produto com nome %d preço %.2f inserido com código %d;\n", namePosition, price, nArticles++);
            write(0, writeAux, strlen(writeAux));

            // Enviar mensagem ao servidor que novo item foi adicionado
            write(server, "m add\n", 6);


        }
        
        // Alterar um nome recebendo o código
        if(strcmp("n", token) == 0){

            // Receção do código do artigo
            token = strtok(NULL, " ");
            code = atoi(token);

            // Receção do novo nome do artigo
            token = strtok(NULL, " ");
            name = strdup(token);

            // Verificação da validade do código
            if(code < 1 || code >= nArticles){
                sprintf(writeAux, "Produto com código %d não existe!\n", code);
                write(0, writeAux, strlen(writeAux));
                continue;
            }

            // Criação do filho para lidar com o ficheiro STRINGS e nome;
            if((son = fork())== 0){
                lseek(strings, 0, SEEK_SET);
                char* articleName;
                int ref = 1;
                char aux[1024];
                
                // Percorrer os nomes
                while((res = readln(strings, c)) > 0){
                    c[res-1] = '\0';
                    
                    articleName = strtok(c, " ");
                    articleName = strtok(NULL, " ");
                    
                    // Verificar se o nome já existe, se sim, retorna o indice onde está
                    if(strcmp(articleName, name)==0){
                        _exit(ref);
                        break;
                    }
                    ref++;
                }

                // Caso o nome não exista criar o novo
                sprintf(aux, "%d %s\n", ref, name);
                write(strings, &aux, strlen(aux));
                _exit(ref);

            }

            // Esperar pela morte do filho
            son = wait(&status);
            namePosition = WEXITSTATUS(status);

            // Alterar posição de escrita no ficheiro e escrever nova referência para o nome;
            lseek(articles, ((code-1) * 16) + 4, SEEK_SET);
            write(articles, &namePosition, 4);

            write(0, "Sucess!\n", 8);
            

        }


        // Alterar um preço recebendo um código de produto
        if(strcmp("p", token) == 0){

            // Receção do código do artigo
            token = strtok(NULL, " ");
            code = atoi(token);

            // Verificação da validade do código
            if(code < 1 || code >= nArticles){
                sprintf(writeAux, "Produto com código %d não existe!\n", code);
                write(0, writeAux, strlen(writeAux));
                continue;
            }

            // Receção do preço do artigo
            token = strtok(NULL, " ");
            price = strtod(token, &stopstring);

            // Alterar posição de escrita no ficheiro e escrever o novo preço;
            lseek(articles, ((code-1) * 16) + 8, SEEK_SET);
            write(articles, &price, 8);
            write(0, "Sucess!\n", 8);

            // Enviar mensagem ao servidor que novo item foi adicionado
            sprintf(writeAux, "m change %d %.2f\n", code, price);
            write(server, writeAux, strlen(writeAux));
        }

    }

    // Fechar fds
    close(articles);
    close(strings);
    
}

