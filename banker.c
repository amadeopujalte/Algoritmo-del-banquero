#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BUFFER 64

bool ingresarExistencia(int r, char* buffer, size_t size, int* existencia);
bool ingresarM(int p, int r, char* buffer, int** m);
void calcularDisponible(int p, int r, int* disponible, int* existencia, int** asignado);
void calcularNecesidad(int p, int r, int** necesidad, int** maximo, int** asignado);
bool puedeEjecutarse(int p, int r, int** necesidad, int* disponible);
void terminarProceso(int p, int r, int** asignado, int* disponible);
void liberarMatriz(int p, int** m);
void liberarMemoria(int p, int* existencia, int** maximo,int** asignados, int** necesidad, int* disponible,int* finished,
        int* trace);

int main(){
    char buffer[MAX_BUFFER];

    int* existencia = NULL;
    int** maximo = NULL;
    int** asignados = NULL;
    int** necesidad = NULL;
    int* disponible = NULL;
    int* trace = NULL;
    int* finished = NULL;

    printf("-----Algoritmo del banquero-----\n");

    printf("Ingrese la cantidad de recursos existentes\n");
    if(fgets(buffer, MAX_BUFFER, stdin) == NULL){
        printf("Error en fgets\n");
        return 1;
    }
    int r = atoi(buffer);

    printf("Ingrese la cantidad de procesos\n");
    if(fgets(buffer, MAX_BUFFER, stdin) == NULL){
        printf("Error en fgets\n");
        return 1;
    }
    int p = atoi(buffer);

    if(r <= 0 || p <= 0){
        printf("Error: cantidad de procesos o recursos invalida\n");
        return 1;
    }

    // Crear matrices
    existencia = calloc(r, sizeof(int));
    maximo = calloc(p, sizeof(int*));
    asignados = calloc(p, sizeof(int*));
    necesidad = calloc(p, sizeof(int*));
    disponible = calloc(r, sizeof(int));

    if(existencia == NULL || maximo == NULL ||
       asignados == NULL || necesidad == NULL ||
       disponible == NULL){

        printf("Error en calloc\n");
        liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
        return 1;
    }

    for(int i = 0; i < p; i++){
        maximo[i] = calloc(r, sizeof(int));
        asignados[i] = calloc(r, sizeof(int));
        necesidad[i] = calloc(r, sizeof(int));

        if(maximo[i] == NULL || asignados[i] == NULL ||
           necesidad[i] == NULL){

            printf("Error en calloc\n");
            liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
            return 1;
        }
    }

    if(!ingresarExistencia(r, buffer, MAX_BUFFER, existencia)){
        liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
        return 1;
    }

    printf("Rellenar matriz de maximo\n");
    if(!ingresarM(p, r, buffer, maximo)){
        liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
        return 1;
    }

    printf("Hay recursos ya asignados?(y/n)\n");
    if(fgets(buffer, MAX_BUFFER, stdin) == NULL){
        printf("Error en fgets\n");
        liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
        return 1;
    }

    char c = buffer[0];

    if(c == 'y' || c == 'Y'){
        if(!ingresarM(p, r, buffer, asignados)){
            liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
            return 1;
        }
    }

    // Validar maximo >= asignado
    for(int i = 0; i < p; i++){
        for(int j = 0; j < r; j++){
            if(maximo[i][j] < asignados[i][j]){
                printf("Error Maximo < asignado\n");
                liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
                return 1;
            }
        }
    }

    // Validar recursos asignados contra existentes
    for(int j = 0; j < r; j++){
        int total = 0;
        for(int i = 0; i < p; i++){
            total += asignados[i][j];
        }

        if(total > existencia[j]){
            printf("Error: asignados supera existencia\n");
            liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
            return 1;
        }
    }

    // Determinar estado inicial de las matrices
    calcularDisponible(p, r, disponible, existencia, asignados);
    calcularNecesidad(p, r, necesidad, maximo, asignados);

    trace = calloc(p, sizeof(int));
    finished = calloc(p, sizeof(int));

    if(trace == NULL || finished == NULL){
        printf("Error en calloc\n");
        liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);
        return 1;
    }

    for(int i = 0; i < p; i++){
        finished[i] = -1;
    }

    int z = 0;
    char bloqueo = 0;
    int j = 0;
    // Inicio
    while(z < p && !bloqueo){
        j = z;
        for(int i = 0; i < p; i++){
            if(finished[i] == -1 &&
               puedeEjecutarse(i, r, necesidad, disponible)){
                terminarProceso(i, r, asignados, disponible);
                trace[z] = i;
                finished[i] = 1;
                z++;
            }
        }

        if(j == z) bloqueo = 1;
    }
    if(!bloqueo){
        printf("Tiene al menos un camino seguro!\n");
        printf("El camino resulto ser:\n");

        for(int i = 0; i < p; i++){
            printf("P%d", trace[i]);
            if(i < p - 1) printf(" -> ");
        }
        printf("\n");
    }
    else{
        printf("No existe camino seguro\n");
    }
    liberarMemoria(p, existencia, maximo, asignados,necesidad, disponible, finished, trace);

    return 0;
}

bool ingresarExistencia(int r, char* buffer, size_t size, int* existencia){
    printf("Ingresar la cantidad de cada recurso\n");
    for(int i = 0; i < r; i++){
        printf("Recurso %d:\n", i);
        if(fgets(buffer, size, stdin) == NULL){
            printf("Error en fgets\n");
            return false;
        }

        existencia[i] = atoi(buffer);
    }

    return true;
}

bool ingresarM(int p, int r, char* buffer, int** m){
    for(int i = 0; i < p; i++){
        for(int j = 0; j < r; j++){
            printf("M[P%d][R%d]= ", i, j);
            if(fgets(buffer, MAX_BUFFER, stdin) == NULL){
                printf("Error en fgets\n");
                return false;
            }

            m[i][j] = atoi(buffer);
        }
    }

    return true;
}

void calcularDisponible(int p, int r, int* disponible,int* existencia, int** asignado){
    // Disponible = existente - asignado
    for(int i = 0; i < r; i++){
        int total = 0;
        for(int j = 0; j < p; j++){
            total += asignado[j][i];
        }

        disponible[i] = existencia[i] - total;
    }
}

void calcularNecesidad(int p, int r, int** necesidad,int** maximo, int** asignado){
    // Necesidad = maximo - asignado
    for(int i = 0; i < p; i++){
        for(int j = 0; j < r; j++){
            necesidad[i][j] = maximo[i][j] - asignado[i][j];
        }
    }
}

bool puedeEjecutarse(int p, int r, int** necesidad, int* disponible){
    // Necesidad <= disponible
    int sum = 0;
    for(int i = 0; i < r; i++){
        if(necesidad[p][i] <= disponible[i]){
            sum++;
        }
    }
    if(sum == r) return true;
    return false;
}

void terminarProceso(int p, int r, int** asignado, int* disponible){
    for(int i = 0; i < r; i++){
        disponible[i] += asignado[p][i];
    }
}

void liberarMatriz(int p, int** m){
    if(m == NULL) return;
    for(int i = 0; i < p; i++){
        free(m[i]);
    }
    free(m);
}

void liberarMemoria(int p, int* existencia, int** maximo,int** asignados, int** necesidad, int* disponible,int* finished,
        int* trace){
    free(existencia);
    liberarMatriz(p, maximo);
    liberarMatriz(p, asignados);
    liberarMatriz(p, necesidad);
    free(disponible);
    free(finished);
    free(trace);
}
