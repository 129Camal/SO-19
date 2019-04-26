#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaração da Estrutura do Array Dinâmico
typedef struct dynamicArray {
    double* data;
    int pos;
    int size;
} *doubleArray;

// Inicialização do Array Dinâmico
doubleArray init_array(int size) {
    doubleArray conjunto = malloc(sizeof(struct dynamicArray));
    conjunto -> data = malloc(size * sizeof(double));
    conjunto-> pos = 0;
    conjunto-> size = size;
    return conjunto;
}


// Inserção no Array Dinâmico
void array_insert(doubleArray obj, double string) {
    
    int position = obj->pos;
    
    if(obj -> pos == (obj -> size - 2)) {
        obj->size *= 2;
        obj->data = realloc(obj->data, obj->size * sizeof(double));
    }

    obj->data[position] = string;
    obj->pos++;
}

// Procurar um valor na estrutura, retorna a posição onde se encontra;

int exist(doubleArray l, double value){
    int exist = -1, i;
    
    for(i = 0; i < l->pos; i++){
        if(l->data[i] == value){
            exist = i;
            break;
        }
    }

    return exist;

}


// Procurar um valor na estrutura, recebe o id como argumento

double getFromPosition(doubleArray l, int id){
    double r = -1.0;

    if(id > l->pos || id < 1){
        return r;
    } else {
        return l->data[id-1];
    }
}

// Procurar um valor na estrutura, recebe o id como argumento
void addToPosition(doubleArray l, int id, double value){

    l->data[id-1] = value;

}

// Libertação do Array da memória

void free_array(doubleArray l) {
    
    free(l->data);
    free(l);

}

// Ir buscar a última posição ocupada do Array

int getPosition(doubleArray l){

    return l -> pos;

}

// Ir buscar o Array que contém os dados
double* getArray(doubleArray l){

    return l -> data;

}


