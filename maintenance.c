#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include "dynamicArray.c"

// Função para ler um parágrafo
ssize_t readln(int fildes, char* buf){
    ssize_t total_char = 0, r;

    while((r = read(fildes, buf + total_char, 1)) > 0){
        if(buf[total_char++] == '\n') break;
    }

    return total_char;
}

int main(int argc, char** argv){
    
    // Abrir os ficheiros necessários
    int articles = open("./txtFiles/ARTIGOS.txt", O_RDWR);
    int strings = open("./txtFiles/STRINGS.txt", O_RDWR);
    int requests = open(argv[1], O_RDONLY);

    // Inicialização dos Arrays Dinâmicos para guardar a informação 
    array arrayArticles = init_array(100);
    array arrayStrings = init_array(100);

    
    ssize_t res;
    char c[1024];

    // Leitura do ficheiro ARTIGOS e armazenamento na estrutura correspondente
    while((res = readln(articles, &c)) > 0){
        c[res-1] = '\0';
        array_insert(arrayArticles, strdup(c));
    }

    close(articles);

    // Leitura do ficheiro STRINGS e armazenamento na estrutura correspondente
    char* articleName;
    while((res = readln(strings, &c)) > 0){
        c[res-1] = '\0';
        articleName = strtok(c, " ");
        articleName = strtok(NULL, " ");
        array_insert(arrayStrings, strdup(articleName));
    }

    close(strings);

    int code;
    char* name;
    double price = 0.0;
    char *token, *stopstring;
    char writeAux[1024];
    
    // Leitura do ficheiro REQUESTS que contêm os comandos que queremos executar
    while((res = readln(requests, &c)) > 0){
        c[res-1] = '\0';
        
        token = strtok(c, " ");

        // Caso a instrução recebida comece por i (Inserir Novo Artigo)
        if(strcmp("i", token) == 0){
            // Receção do nome do artigo
            token = strtok(NULL, " ");
            name = strdup(token);

            // Verificação da não existência de um nome igual na lista de nomes
            if(exist(arrayStrings, name) != -1){

                sprintf(writeAux, "Product with name %s, cannot be added. Product already exists!\n", name);
                write(1, writeAux, strlen(writeAux));
                continue;
            
            } else {
                
                // Adicionar o novo nome de artigos à estrutura de nomes
                array_insert(arrayStrings, strdup(name));

            }

            // Receção do preço do artigo
            token = strtok(NULL, " ");
            price = strtod(token, &stopstring);

            // Adicionar o artigo à estrutura
            sprintf(writeAux, "%d %d %.2f", getPosition(arrayArticles) + 1, getPosition(arrayStrings), price);
            array_insert(arrayArticles, strdup(writeAux));
        }


        // Alterar um nome recebendo o código
        if(strcmp("n", token) == 0){

            // Receção do código do artigo
            token = strtok(NULL, " ");
            code = atoi(token);

            // Receção do novo nome do artigo
            token = strtok(NULL, " ");
            name = strdup(token);
            
            // Verificar se o produto existe
            if(code < getPosition(arrayArticles) && code > 0){
                
                // Ir buscar o artigo que está na posição do código
                char* element = getFromPosition(arrayArticles, code);
                
                // Desmembrar o artigo
                token = strtok(element, " ");
                
                // Este token contêm o id do nome atual
                token = strtok(NULL, " ");

                // Verificar se o novo nome existe
                if(exist(arrayStrings, name) == -1){
                    
                    // Inserir o novo nome na estrutura de nomes
                    addToPosition(arrayStrings, atoi(token), strdup(name));
                    
                    // Construir e adicionar o novo artigo à estrutura dos artigos
                    sprintf(writeAux, "%d %d %s", code, atoi(token), strtok(NULL, " "));
                    addToPosition(arrayArticles, code, strdup(writeAux));

                
                } else {
                    // Mensagem de erro se o novo nome do artigo já existe
                    sprintf(writeAux, "It's impossible to change the name of product with code %d! Name already in use!\n", code);
                    write(1, writeAux, strlen(writeAux));
                }

            } else {
                // Mensagem de erro se o artigo não existir
                sprintf(writeAux, "It's impossible to change the name of product with code %d! Product doesn't exist!\n", code);
                write(1, writeAux, strlen(writeAux));
            }
        }


        // Alterar um preço recebendo um código
        if(strcmp("p", token) == 0){

            // Receção do código do artigo
            token = strtok(NULL, " ");
            code = atoi(token);

            // Receção do preço do artigo
            token = strtok(NULL, " ");
            price = strtod(token, &stopstring);
            //printf("Price: %.2f; Code: %d\n", price, code);

            if(code < getPosition(arrayArticles) && code > 0){
                    
                // Ir buscar o artigo que está na posição do código
                char* element = getFromPosition(arrayArticles, code);
                    
                // Desmembrar o artigo
                token = strtok(element, " ");
            
                // Construir e adicionar o novo artigo à estrutura dos artigos
                sprintf(writeAux, "%d %s %.2f", code, strtok(NULL, " "), price);
                addToPosition(arrayArticles, code, strdup(writeAux));

            } else {
                // Mensagem de erro se o artigo não existir
                sprintf(writeAux, "It's impossible to change the price of product with code %d! Product doesn't exist!\n", code);
                write(1, writeAux, strlen(writeAux));
            }
        }


    }

    // Após verificar todos os comandos
    // Voltar a abrir os ficheiros para apagar tudo e preparar para escrever
    articles = open("./txtFiles/ARTIGOS.txt", O_WRONLY |  O_TRUNC);
    strings = open("./txtFiles/STRINGS.txt", O_WRONLY | O_TRUNC);
    

    // Buscar o informação sobre os artigos
    int pos = getPosition(arrayArticles);
    char** data = getArray(arrayArticles);

    // Imprimir artigos
    for(int i = 0; i < pos; i++){
        sprintf(writeAux, "%s\n", data[i]);
        write(articles, writeAux, strlen(writeAux));
    }

    // Fechar array de artigos
    close(articles);

    // Buscar informação sobre os nomes
    pos = getPosition(arrayStrings);
    data = getArray(arrayStrings);

    // Imprimir os nomes
    for(int i = 0; i < pos; i++){
        sprintf(writeAux, "%d %s\n", i+1, data[i]);
        write(strings, writeAux, strlen(writeAux));
    }

    // Fechar fd
    close(strings);
}

