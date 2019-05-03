#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include "article.c"

#define CACHESIZE 3

// Função para ler um parágrafo
ssize_t readln(int fildes, char* buf){
    ssize_t total_char = 0, r;

    while((r = read(fildes, buf + total_char, 1)) > 0){
        if(buf[total_char++] == '\n') break;
    }

    return total_char;
}


int main(int argc, char** argv){

    // Abrir ficheiro dos artigos
    int articles = open("./files/ARTIGOS.bin", O_RDONLY);
    int stocks = open("./files/STOCKS.bin", O_RDWR);
    int sells = open("./files/VENDAS.bin", O_WRONLY | O_APPEND);
    
    //Estrutura para fazer o caching de preços
    Article* caching = malloc(CACHESIZE * sizeof(Article));
    int posCaching = 0, nCaching = 0;

    ssize_t res;
    char c[1024];
    char writeAux[1024];
    double price;
    int nArticles = 1;

    // Criar o fifo para comunicação
    mkfifo("./communicationFiles/server", 0666); 

    // Leitura do ficheiro ARTIGOS para descobrir o número de artigos que tem
    int stockAvailable = 0;
    
    while((res = read(articles, &c, 16)) > 15){
        nArticles++;

        // Adicionar Stock dos Artigos
        // write(stocks, &nArticles, 4);
        // write(stocks, &stockAvailable, 4);
        
    }
    // Abrir fifo para escutar pedidos
    int fifo = open("./communicationFiles/server", O_RDONLY);

    char* messageHead;
    int code;
    char* quantity;
    int clientQuantity, clientFD;
    char clientAddress[512];
    int i;
    char* stopstring;

    lseek(articles , 0, SEEK_SET);


    Article art = createArticle();
    
    while(read(articles, art, 16)){
        printf("%d %d %.2f\n", art->code, art->ref, art->price);
    }


    // Ler informação a partir do fifo e ficar à espera
    while((res = readln(fifo, c)) >= 0){
        
        // Quando não lê nada para continuar a escutar
        if(res == 0) continue;

        // Caso contrário, quando lê
        else {
            //write(1, c, res);
            
            c[res-1] = '\0';

            messageHead = strtok(c, " ");

            if(strcmp(messageHead, "m")==0){
                if(strcmp(strtok(NULL, " "), "add") == 0){
                    // Incrementar o número de artigos
                    nArticles++;

                    // Colocar o apontador do ficheiro no fim
                    lseek(stocks, 0, SEEK_END);
                    
                    // Escrever no ficheiro de stocks, o stock do novo produto
                    write(stocks, &nArticles, 4);
                    
                    stockAvailable = 0;
                    write(stocks, &stockAvailable, 4);
                } else {
                    // Buscar o código do produto
                    code = atoi(strtok(NULL, " "));

                    // Buscar o preço para que foi alterado
                    price = strtod(strtok(NULL, " "), &stopstring);

                    // Procurar se existe na cache o produto qual o preço foi alterado
                    for(i = 0; i < nCaching; i++){
                        if(caching[i]->code == code){
                            caching[i]->price = price;
                            //write(1, "Alterei na cache\n", 17);
                            break;
                        }
                    }   
                }

            } else {
                code = atoi(strtok(NULL, " "));
                quantity = strtok(NULL, " ");
            
                // Concatenar o pid com o path
                sprintf(clientAddress, "./communicationFiles/%s", messageHead);

                // Verificação da validade do código
                if(code < 1 || code >= nArticles){
                    sprintf(writeAux, "Produto com código %d não existe!\n", code);
                    clientFD = open(clientAddress, O_WRONLY);
                    write(clientFD, writeAux, strlen(writeAux));
                    close(clientFD);
                    continue;
                }

                //Procurar na cache pelo código do produto
                for(i = 0; i < nCaching; i++){
                    if(caching[i]->code == code){
                        price = caching[i]->price;
                        //write(1, "Encontrei cache\n", 16);
                        break;
                    }
                }
                
                
                //Se não estiver na cache ir buscar ao ficheiro
                if(i >= nCaching && posCaching < CACHESIZE){

                    // Buscar o preço do artigo
                    lseek(articles, (code-1) * 16, SEEK_SET);

                    // Criar o objeto artigo
                    Article aux = createArticle();
                    read(articles, aux, 16);
                
                    // Colocar o preço
                    price = aux->price;

                    //Adicionar à cache
                    caching[posCaching] = aux;

                    //Alterar a posição a preencher na cache
                    posCaching++;

                    //Alterar o número total de elementos na cache
                    if(nCaching < CACHESIZE) nCaching++;

                    // Alterar o apontador para preencher o array se estiver no maximo
                    if(posCaching == CACHESIZE) posCaching = 0;
                }

                // Buscar o stock do artigo
                lseek(stocks, ((code-1) * 8) + 4, SEEK_SET);
                read(stocks, &stockAvailable, 4);

                // Query 1 do Cliente
                if(!quantity){

                    // Preparar resposta e enviar para a primeira query
                    sprintf(writeAux, "Stock: %d; Price: %.2f\n", stockAvailable, price);
                    clientFD = open(clientAddress, O_WRONLY);
                    write(clientFD, writeAux, strlen(writeAux));
                    close(clientFD);
                } 
                // Query 2 do Cliente
                else {
                
                    // Verificar a quantidade que o cliente quer
                    clientQuantity = atoi(quantity);

                    // Verificar se a quantidade desejada pode ser retirada
                    if(clientQuantity < 0 && stockAvailable < abs(clientQuantity)){
                        
                        sprintf(writeAux, "Stock insuficiente! Temos a seguinte quantidade em stock %d!\n", stockAvailable);
        
                    } 
                    // Caso possa, ou é retirada ou adicionada, dependendo do que o utilizador quer. 
                    else {

                        stockAvailable += clientQuantity;
                    
                        //Escrever no ficheiro de stocks
                        lseek(stocks, ((code-1) * 8) + 4, SEEK_SET);
                        write(stocks, &stockAvailable, 4);
                        sprintf(writeAux, "Stock atualizado para %d!\n", stockAvailable);

                        if(clientQuantity < 0){
                        
                            // Registar a venda no ficheiro de vendas
                            write(sells, &code, 4);

                            clientQuantity = abs(clientQuantity);
                            write(sells, &clientQuantity, 4);
                        
                            price = price * abs(clientQuantity);
                            write(sells, &price, 8);
                        }
                    }

                    // Enviar a resposta ao cliente que pediu
                    clientFD = open(clientAddress, O_WRONLY);
                    write(clientFD, writeAux, strlen(writeAux));
                    close(clientFD);
                }
            }
        }
    }
}