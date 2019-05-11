#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "article.c"

typedef struct cache {
    Article* array;
    int posCache;
    int nCache;
} *Cache;

Cache createCache(int size){
  Cache ca = malloc(sizeof(struct cache)); 
  ca->array = malloc(size * sizeof(Article));
  ca->posCache = 0;
  ca->nCache = 0;
  return ca;
}

void changeCache(Cache caching, int code, double price){
  for(int i = 0; i < caching->nCache; i++){
    if(caching->array[i]->code == code){
      caching->array[i]->price = price;
      //write(1, "Alterei na cache\n", 17);
      break;
    }
  }
}

double searchCache(Cache caching, int code){
  double price = -1;

  for(int i = 0; i < caching->nCache; i++){
    if(caching->array[i]->code == code){
      price = caching->array[i]->price;
      //write(1, "Encontrei cache\n", 16);
      break;
    }
  }

  return price;
}

double addToCache(Cache caching, int code, int articles, int size){
  // Buscar o preço do artigo
  lseek(articles, (code-1) * 16, SEEK_SET);

  // Criar o objeto artigo
  Article aux = createArticle();
  read(articles, aux, 16);

  // Colocar o preço
  double price = aux->price;

  //Adicionar à cache
  caching->array[caching->posCache] = aux;

  //Alterar a posição a preencher na cache
  caching->posCache += 1;

  //Alterar o número total de elementos na cache
  if(caching->nCache < size) caching->nCache += 1;

  // Alterar o apontador para preencher o array se estiver no maximo
  if(caching->posCache == size) caching->posCache = 0;

  return price;
}

void freeCache(Cache ca){
    free(ca->array);
    free(ca);
}