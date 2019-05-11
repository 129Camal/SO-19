#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include "sell.c"

int main(){
    char address[512];

    // Abrir um ficheiro para guardar os dados e efetuar o calculo
    sprintf(address, "./../agregationFiles/%d", getpid());
    int sumFile = open(address, O_RDWR | O_CREAT, 0666);
    int diferent_article = 0;

    // Declaração de variáveis
    Sell sell = createSell();
    Sell sell_aux = createSell();
    int i;
    int quantity = 0;
    double price = 0.0;
    
    // Receção das entradas do ficheiro de vendas vindo do pipe
    while(read(0, sell, 16)){
        
        lseek(sumFile, 0, SEEK_SET);
                
        // Inicializar uma SELL que vamos ler do ficheiro
        for(i = 0; i < diferent_article; i++){
            read(sumFile, sell_aux, 16);

            if(sell_aux->code == sell->code){

                // Somar os valores da quantidade e preço
                quantity = sell_aux->quantity + sell->quantity;
                price = sell_aux->price + sell->price;

                // Posicionar o apontador do ficheiro para escrever
                lseek(sumFile, (i*16) + 4, SEEK_SET);

                // Escrever no ficheiro a quantidade total
                write(sumFile, &quantity, 4);

                // Escrever no ficheiro o preço total 
                write(sumFile, &price, 8);

                break;
            }
        }

        if(i == diferent_article){
            lseek(sumFile, 0, SEEK_END);
            write(sumFile, sell, 16);
            diferent_article++;
        }
        
    }
        
    // Apontar para o inicio do ficheiro
    lseek(sumFile, 0, SEEK_SET);
        
    // Ler o resultado do ficheiro e mandar para o servidor
    while(read(sumFile, sell, 16) > 0){
        write(1, sell, 16);
    }

    // Fechar pipe após conclusão de escrita
    close(1);

    freeSell(sell);
    freeSell(sell_aux);


    // No final, apagar o ficheiro usado para fazer os somatórios
    execlp("rm", "rm", address, NULL);
}