#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include "cache.c"

#define CACHESIZE 5

// Função para ler um parágrafo
ssize_t readln(int fildes, char* buf){
    ssize_t total_char = 0, r;

    while((r = read(fildes, buf + total_char, 1)) > 0){
        if(buf[total_char++] == '\n') break;
    }

    return total_char;
}

// Função que trata do pedido da manutenção a informar que adicionou um novo produto
void addArticle(int stocks, int nArticles){
    // Colocar o apontador do ficheiro no fim
    lseek(stocks, 0, SEEK_END);
                    
    // Escrever no ficheiro de stocks, o stock do novo produto
    write(stocks, &nArticles, 4);
                    
    int stockAvailable = 0;
    write(stocks, &stockAvailable, 4);
}

// Função que trata do pedido da manutenção a informar a alteração de um preço
void changeArticle(char* request, Cache caching){
    char* stopstring;

    // Buscar o código do produto
    int code = atoi(strtok(request, " "));

    // Buscar o preço para que foi alterado
    double price = strtod(strtok(NULL, " "), &stopstring);
    
    // Procurar se existe na cache o produto qual o preço foi alterado
    changeCache(caching, code, price);
}

// Função para escrever de volta para o cliente que entrou em contacto
void writeToClient(char* writeAux, char* clientAddress){
    int clientFD = open(clientAddress, O_WRONLY);
    write(clientFD, writeAux, strlen(writeAux));
    close(clientFD);
}

// Função para fazer a segunda query do cliente para adicionar/remover stock
void clientSumStock(int sells, int stocks, int quantity, int stockAvailable,int code, double price, char* clientAddress){
    char writeAux[1024];

    // Verificar se a quantidade desejada pode ser retirada
    if(quantity < 0 && stockAvailable < abs(quantity)){
        
        sprintf(writeAux, "Stock insuficiente! Temos a seguinte quantidade em stock %d!\n", stockAvailable);
        
    }
    // Caso possa, ou é retirada ou adicionada, dependendo do que o utilizador quer. 
    else {

    stockAvailable += quantity;
                    
    //Escrever no ficheiro de stocks
    lseek(stocks, ((code-1) * 8) + 4, SEEK_SET);
    write(stocks, &stockAvailable, 4);
    sprintf(writeAux, "Stock atualizado para %d!\n", stockAvailable);

        if(quantity < 0){
                        
            // Registar a venda no ficheiro de vendas
            write(sells, &code, 4);

            quantity = abs(quantity);
            write(sells, &quantity, 4);
                        
            price = price * abs(quantity);
            write(sells, &price, 8);
        }
    } 

    writeToClient(writeAux, clientAddress);
}


int main(int argc, char** argv){
    
    // Abrir ficheiros que o servidor precisa de aceder
    int articles = open("./files/ARTIGOS", O_RDONLY);
    int stocks = open("./files/STOCKS", O_RDWR);
    int sells = open("./files/VENDAS", O_WRONLY | O_APPEND);

    // Estrutura para fazer o caching de preços
    Cache caching = createCache(CACHESIZE);
    
    // Printar os artigos que têm e conta-los
    Article art = createArticle();
    int nArticles = 1;
    int stockAvailable = 0;

    while(read(articles, art, 16)){
        nArticles++;
        printf("%d %d %.2f\n", art->code, art->ref, art->price);
        
        // Tirar comentário para adicionar stocks se servidor foi iniciado pela primeira vez
        // write(stocks, &nArticles, 4);
        // write(stocks, &stockAvailable, 4);
    
    }

    // Criação e abertura de fifo que é a caixa de correio do servidor
    mkfifo("./communicationFiles/server", 0666); 
    int fifo = open("./communicationFiles/server", O_RDONLY);

    ssize_t res;
    char c[1024];
    char clientAddress[512];
    char writeAux[1024];
    char* messageHead;
    char* action;
    char* messageTail;
    int code;
    char* quantity;
    double price;

    // Receção de pedidos pelo fifo
    while((res = readln(fifo, c)) >= 0){
        
        // Caso não tenha lido nada
        if(res == 0) continue;
        // Caso tenha lido algum comando ou mensagem
        else{
            //write(1, c, res);
            // Tirar o \n que está no final
            c[res-1] = '\0';
            
            // Verificar de onde vem a mensagem
            messageHead = strtok(c, " ");

            // Verificar se vem da manutenção
            if(strcmp(messageHead, "m") == 0){
                action = strtok(NULL, " ");

                if(strcmp(action, "add") == 0){
                    nArticles++;
                    addArticle(stocks, nArticles); 
                    continue;  
                }
                if(strcmp(action, "change") == 0){
                    messageTail = strtok(NULL, "");
                    changeArticle(messageTail, caching);
                    continue;
                }
                if(strcmp(action, "a") == 0){
                    printf("Disponivel brevemente!\n");
                }
            } 
            // Se não é da manutenção, é do cliente de vendas
            else {
                code = atoi(strtok(NULL, " "));
                quantity = strtok(NULL, " ");

                // Concatenar o pid com o path
                sprintf(clientAddress, "./communicationFiles/%s", messageHead);

                // Verificação da validade do código
                if(code < 1 || code >= nArticles){
                    sprintf(writeAux, "Produto com código %d não existe!\n", code);
                    writeToClient(writeAux, clientAddress);
                    continue;
                }

                // Procurar artigo na cache
                price = searchCache(caching, code);

                // Se não estiver na cache adiciona-lo à cache
                if(price < 0) price = addToCache(caching, code, articles, CACHESIZE);

                // Buscar o stock do artigo
                lseek(stocks, ((code-1) * 8) + 4, SEEK_SET);
                read(stocks, &stockAvailable, 4);

                // Query 1 do Cliente
                if(!quantity){
                    sprintf(writeAux, "Stock: %d; Price: %.2f\n", stockAvailable, price);
                    writeToClient(writeAux, clientAddress);
                    
                } else {
                    clientSumStock(sells, stocks, atoi(quantity), stockAvailable, code, price, clientAddress);
                }
            }
        }
    }
    close(articles);
    close(stocks);
    close(sells);
    close(fifo);
    freeCache(caching);

}