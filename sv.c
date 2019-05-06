#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>
#include "cache.c"
#include "sell.c"

#define CACHESIZE 5
#define AGSIZE 3

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
int clientSumStock(int sells, int stocks, int quantity, int stockAvailable, int code, double price, char* clientAddress, int numberSells){
    char writeAux[1024];
    Sell sell = createSell();

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

        // Registar a venda no ficheiro Vendas
        if(quantity < 0){

            quantity = abs(quantity);
            price = price * abs(quantity);

            sell->code = code;
            sell->quantity = quantity;
            sell->price = price;
            
            write(sells, sell, 16);

            numberSells++;
        }
    } 

    writeToClient(writeAux, clientAddress);
    return numberSells;
}

//Função para agregação final
void agreggateFinal(int fd){
    pid_t son;
    time_t rawtime;
    struct tm *info;
    char buffer[80];
    char writeAux[1024];

    time( &rawtime );

    info = localtime( &rawtime );

    strftime(buffer, 80, "./agregationFiles/%Y-%m-%dT%X", info);

    int resultFile = open(buffer, O_RDWR | O_APPEND | O_CREAT, 0666);
    
    int pIN[2];
    int pOUT[2];

    if(pipe(pIN) < 0 || pipe(pOUT) < 0){
        exit(1);
    }

    if((son = fork())==0){
        // Tratar do pipe de entrada FILHO
        close(pIN[1]);
        dup2(pIN[0], 0);
        close(pIN[0]);

        // Tratar do pipe de saída FILHO
        close(pOUT[0]);
        dup2(pOUT[1], 1);
        close(pOUT[1]);

        // Execução do agregador
        char *args[]= {"./ag", NULL}; 
        execv(args[0], args);

    } else {
        
        // Tratar do pipe de entrada, fechando entrada
        close(pIN[0]);

        // Tratar do pipe de saida, fechando a escrita
        close(pOUT[1]);

        Sell sell = createSell();

        lseek(fd, 0, SEEK_SET);

        while(read(fd, sell, 16)){
            
            write(pIN[1], sell, 16);
        
        }
        
        // Fechar quando se acaba de enviar 
        close(pIN[1]);

        // Receber conjunto agregado
        while(read(pOUT[0], sell, 16)){

            sprintf(writeAux, "%d %d %.2f\n", sell->code, sell->quantity, sell->price);
            write(resultFile, writeAux, strlen(writeAux));
        
        }

        freeSell(sell);
    }

    if((son = fork())==0){
        execlp("rm", "rm", "./agregationFiles/aux", NULL);
    }
}


// Função para agregar as vendas concorrentemente
int agreggateConcorrent(int sells, int numberSells, int lastAgregation){
    
    pid_t son;
    int toAdd = numberSells - lastAgregation;
    int numberOfAgreggators;

    if(toAdd <= AGSIZE){
        numberOfAgreggators = 1;
    } 
    else {
        numberOfAgreggators = toAdd / AGSIZE;
    }
    int toAddEachAgreggator = toAdd / numberOfAgreggators;
    int r = 1;

    //printf("%d %d %d\n", toAdd, numberOfAgreggators, toAddEachAgreggator);
    
    int fd[2 * numberOfAgreggators][2];
    
    Sell sell = createSell();

    lseek(sells, lastAgregation * 16, SEEK_SET);

    for(int i = 0; i < numberOfAgreggators; i++){
        
        pipe(fd[r++]);
        pipe(fd[r++]);

        if((son = fork())== 0){

            // Tratar do pipe de entrada FILHO
            close(fd[r-2][1]);
            dup2(fd[r-2][0], 0);
            close(fd[r-2][0]);


            // Tratar do pipe de saída FILHO
            close(fd[r-1][0]);
            dup2(fd[r-1][1], 1);
            close(fd[r-1][1]);
            

            // Execução do agregador
            char *args[]= {"./ag", NULL}; 
            execv(args[0], args);
        
        } else {
            
            // Tratar do pipe de entrada, fechando entrada
            close(fd[r-2][0]);
            
            if(i == numberOfAgreggators - 1){
                    
                while(read(sells, sell, 16)){
                    //printf("ENVIANDO %d -> Code: %d, Quantity: %d, Price: %.2f\n", r-2, sell->code, sell->quantity, sell->price);
                    write(fd[r-2][1] , sell, 16);
                    lastAgregation++;
                }
                
            } else {
                for(int j = 0; j < toAddEachAgreggator; j++){
                    read(sells, sell, 16);
                    write(fd[r-2][1] , sell, 16);
                    //printf("ENVIANDO %d -> Code: %d, Quantity: %d, Price: %.2f\n", r-2, sell->code, sell->quantity, sell->price);
                    lastAgregation++;
                    }
                }
            
            // Fechar quando se acaba de enviar
            close(fd[r-2][1]);

             // Fechar parte de escrita do pipe que o filho vai escrever
            close(fd[r-1][1]);

        }    
    }

    r = 2;
    ssize_t res;
    Sell sell_aux = createSell();

    int fdaux = open("./agregationFiles/aux", O_CREAT | O_RDWR | O_TRUNC, 0666);

    for(int i = 0; i < numberOfAgreggators; i++){
        
        while((res = read(fd[r][0], sell_aux, 16)) > 0){
            write(fdaux, sell_aux, 16);
            //printf("PAI RECEBEU %d -> Code: %d, Quantity: %d, Price: %.2f\n", r, sell_aux->code, sell_aux->quantity, sell_aux->price);
        } 
        r += 2;
    }
    
    // Agregação final
    agreggateFinal(fdaux);
    
    return lastAgregation;
}

// Função para agregar as vendas
int agreggate(int sells, int numberSells, int lastAgregation){
    pid_t son;
    time_t rawtime;
    struct tm *info;
    char buffer[80];
    char writeAux[1024];

    time( &rawtime );

    info = localtime( &rawtime );

    strftime(buffer, 80, "./agregationFiles/%Y-%m-%dT%X", info);

    int resultFile = open(buffer, O_RDWR | O_APPEND | O_CREAT, 0666);
    
    int pIN[2];
    int pOUT[2];

    if(pipe(pIN) < 0 || pipe(pOUT) < 0){
        exit(1);
    }

    if((son = fork())==0){
        // Tratar do pipe de entrada FILHO
        close(pIN[1]);
        dup2(pIN[0], 0);
        close(pIN[0]);

        // Tratar do pipe de saída FILHO
        close(pOUT[0]);
        dup2(pOUT[1], 1);
        close(pOUT[1]);

        // Execução do agregador
        char *args[]= {"./ag", NULL}; 
        execv(args[0], args);

    } else {
        
        // Tratar do pipe de entrada, fechando entrada
        close(pIN[0]);

        // Tratar do pipe de saida, fechando a escrita
        close(pOUT[1]);

        Sell sell = createSell();

        lseek(sells, lastAgregation * 16, SEEK_SET);

        while(read(sells, sell, 16)){
            write(pIN[1], sell, 16);
            lastAgregation++;
        }
        
        // Fechar quando se acaba de enviar 
        close(pIN[1]);

        // Receber conjunto agregado
        while(read(pOUT[0], sell, 16)){
            sprintf(writeAux, "%d %d %.2f\n", sell->code, sell->quantity, sell->price);
            write(resultFile, writeAux, strlen(writeAux));
        }

        freeSell(sell);
    }
    return lastAgregation;
}


int main(int argc, char** argv){
    
    // Abrir ficheiros que o servidor precisa de aceder
    int articles = open("./files/ARTIGOS", O_RDONLY);
    int stocks = open("./files/STOCKS", O_RDWR);
    int sells = open("./files/VENDAS", O_RDWR | O_APPEND);

    // Estrutura para fazer o caching de preços
    Cache caching = createCache(CACHESIZE);
    
    //Criação de variáveis para ajudar na agregação
    int numberSells = 0;
    int lastAgregation = 0;

    Sell sell = createSell();

    while(read(sells, sell, 16)){
        numberSells++;
    }

    lseek(sells, 0, SEEK_SET);

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
                    //lastAgregation = agreggate(sells, numberSells, lastAgregation);
                    lastAgregation =  agreggateConcorrent(sells, numberSells, lastAgregation);
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
                    numberSells = clientSumStock(sells, stocks, atoi(quantity), stockAvailable, code, price, clientAddress, numberSells);
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