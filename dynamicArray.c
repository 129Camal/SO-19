#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaração da Estrutura do Array Dinâmico
typedef struct dynamicArray {
    char** data;
    int pos;
    int size;
} *array;

// Inicialização do Array Dinâmico
array init_array(int size) {
    array conjunto = malloc(sizeof(struct dynamicArray));
    conjunto -> data = malloc(size * sizeof(char *));
    conjunto-> pos = 0;
    conjunto-> size = size;
    return conjunto;
}


// Inserção no Array Dinâmico
void array_insert(array obj, char* string) {
    
    int position = obj->pos;
    
    if(obj -> pos == (obj -> size - 2)) {
        obj->size *= 2;
        obj->data = realloc(obj->data, obj->size * sizeof(char *));
    }

    obj->data[position] = string;
    obj->pos++;
}

// Procurar um valor na estrutura, retorna a posição onde se encontra;

int exist(array l, char* value){
    int exist = -1, i;
    
    for(i = 0; i < l->pos; i++){
        if(strcmp(l->data[i], value) == 0){
            exist = i;
            break;
        }
    }

    return exist;

}


// Procurar um valor na estrutura, recebe o id como argumento

char* getFromPosition(array l, int id){

    return l->data[id-1];

}

// Procurar um valor na estrutura, recebe o id como argumento
void addToPosition(array l, int id, char* value){

    l->data[id-1] = value;

}

// Libertação do Array da memória

void free_array(array l) {
    
    free(l->data);
    free(l);

}

// Ir buscar a última posição ocupada do Array

int getPosition(array l){

    return l -> pos;

}

// Ir buscar o Array que contém os dados
char** getArray(array l){

    return l -> data;

}

