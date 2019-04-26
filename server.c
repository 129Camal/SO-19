#include <stdio.h>
#include <string.h>
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

    // Abrir ficheiro dos artigos
    int articles = open("./txtFiles/ARTIGOS.txt", O_RDONLY);
    int stocks = open("./txtFiles/STOCKS.txt", O_RDWR);

    // Inicialização da estrutura de preços
    doubleArray arrayPrices = init_array(100);

    ssize_t res;
    char c[1024];
    char* token, *stopstring;
    double price;

    // Leitura do ficheiro ARTIGOS e armazenamento na estrutura correspondente
    while((res = readln(articles, &c)) > 0){
        
        // Desfazendo a parte lida procurando pelo preço
        token = strtok(c, " ");
        token = strtok(NULL, " ");
        token = strtok(NULL, " ");

        // Tratar e guardar o preço
        price = strtod(token, &stopstring);
        array_insert(arrayPrices, price);
    }

    // Fechar o fd
    close(articles);

    // Criar o fifo para comunicação
    mkfifo("./communicationFiles/server", 0666); 

    // Abrir o fifo para leitura
    int fifo = open("./communicationFiles/server", O_RDONLY);

    int code;
    char* stock;
    char* type, *clientPid;
    char writeAux[1024];
    char address[512];
    int response;
    

    // Ler informação a partir do fifo e ficar à espera
    while((res = readln(fifo, &c)) >= 0){
        
        // Quando não lê nada
        if(res == 0) continue;
        
        // Quando lê uma frase
        else {

            c[res-1] = '\0';

            //Descobrir qual o tipo de ação que a mensagem recebida despoleta
            type = strtok(c, " ");
            
            // Ver se é uma ação vinda do cliente
            if(strcmp(type,"c") == 0){

                // Desmembrar o resto da mensagem
                clientPid = strtok(NULL, " ");
                code = atoi(strtok(NULL, " "));
                token = strtok(NULL, " ");
                
                // Verificar se o código do produto é válido
                if(code > getPosition(arrayPrices) || code < 1){
                    
                    // Abrir fifo de comunicação
                    response = open(address, O_WRONLY);
                    // Responder
                    write(response, "Código Inválido!\n", 19);

                    // Fechar fifo
                    close(response);
                    
                    continue;
                }

                sprintf(address, "./communicationFiles/%s", clientPid);
                
                // Se for um query para devolver o stock e preço
                if(!token){
                    
                    // Buscar preço
                    price = getFromPosition(arrayPrices, code);

                    // Ler até ao stock
                    for(int i = 1; i <= code; i++){
                        res = readln(stocks, &c);
                    }

                    // Tornar apontador a 0
                    lseek(stocks, 0, SEEK_SET);

                    c[res-1] = '\0';

                    // Buscar o stock atual
                    stock = strtok(c, " ");
                    stock = strtok(NULL, " ");

                    // Preparar resposta e enviar
                    sprintf(writeAux, "Stock: %s; Price: %.2f\n", stock, price);
                    response = open(address, O_WRONLY);
                    write(response, writeAux, strlen(writeAux));
                    close(response);
            
                } 
                // Caso seja a query para alterar a quantidade do stock
                else {

                    double atualStock, plusStock;

                    // Ver qual o stock que se quer adicionar
                    plusStock = strtod(token, &stopstring);

                    // Inicializar estrutura para manipular a informação
                    doubleArray arrayStocks = init_array(100);

                    // Leitura do ficheiro STOCKS e armazenamento na estrutura correspondente
                    while((res = readln(stocks, &c)) > 0){
                        c[res-1]  = '\0';
                
                    // Desfazendo a parte lida procurando pelo stock
                    token = strtok(c, " ");
                    token = strtok(NULL, " ");
                
                    double s = strtod(token, &stopstring);

                    // Inserir na estrutura de stocks
                    array_insert(arrayStocks, s);
                }

                // Ir buscar o stock atual à estrutura
                atualStock = getFromPosition(arrayStocks, code);    
                
                // Efetuar os calculos e inserir de novo na estrutra
                addToPosition(arrayStocks, code, plusStock + atualStock);

                // Preparação para escrever no ficheiro, começando por fechar o antigo fd
                close(stocks);

                // Abrir o novo fd truncando o ficheiro
                stocks = open("./txtFiles/STOCKS.txt", O_WRONLY | O_TRUNC);

                // Buscar o informação sobre os artigos
                int pos = getPosition(arrayStocks);
                double* data = getArray(arrayStocks);

                // Imprimir stocks de novo no ficheiro
                for(int i = 0; i < pos; i++){
                    sprintf(writeAux, "%d %.0f\n", i+1, data[i]);
                    write(stocks, writeAux, strlen(writeAux));
                }

                // Fechar File System para voltar ao normal
                close(stocks);

                // Preparar a resposta e enviar pelo fifo que o cliente criou
                sprintf(writeAux, "Stock do produto atualizado de %.0f para %.0f!\n", atualStock, atualStock + plusStock);
                response = open(address, O_WRONLY);
                write(response, writeAux, strlen(writeAux));
                close(response);

                // Limpar memória
                free_array(arrayStocks);   
                
                // Voltar a abrir para ler o stocks
                stocks = open("./txtFiles/STOCKS.txt", O_RDONLY);
                }
            }

            if(strcmp(type,"m") == 0){
                
            }
        }
    }
}