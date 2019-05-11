#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct article {
    int code;
    int ref;
    double price;
} *Article;

Article createArticle(){
  Article art = malloc(sizeof(struct article)); 
  return art;
}

void freeArticle(Article art){
    
    free(art);

}