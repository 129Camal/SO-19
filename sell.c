#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct sell {
    int code;
    int quantity;
    double price;
} *Sell;

Sell createSell(){
  Sell sel = malloc(sizeof(struct sell)); 
  return sel;
}

void freeSell(Sell sel){
    
    free(sel);

}