/*
 * segmentacionPaginada.c
 *
 *  Created on: 20 may. 2019
 *      Author: utnso
 */

#include "segmentacionPaginada.h"

 t_segmento* crearSegmento(char *path) {
	 t_segmento* nuevo = malloc(sizeof(t_segmento));
	 nuevo->path = strdup(path);
	 nuevo->tabla_pagina = list_create();
	 return nuevo;
}

 t_pagina* crearPagina(int numeroPagina,int modificado,void* memoria,t_registro registro) {
    t_pagina* new = malloc(sizeof(t_pagina));
    new->numeroPagina = numeroPagina;
    new->modificado = modificado;
    new->direccion = guardarRegistro(memoria,registro);
    return new;
}

t_segmento* buscarSegmento(t_list* lista,char *path) {
	int igualPath(t_segmento *p) {
		return string_equals_ignore_case(p->path, path);
	}

	return list_find(lista, (void*) igualPath);
}

t_pagina* buscarPagina(t_list* lista,uint16_t key,void* memoria) {
	int igualKey(t_pagina* p){

		uint16_t keyEncontrada;
		void* direccion = p->direccion;

		memcpy(&keyEncontrada,(char*)direccion + sizeof(int), sizeof(keyEncontrada));

		if(keyEncontrada == key){
			return 1;
		}
		else{
			return 0;
		}
	}

	return list_find(lista, (void*) igualKey);
}

void* guardarRegistro(void* memoria,t_registro registro){
	int posicionAnalizada = 0;
	void * direccion;
	int posicion = 0;
	int lugarVacio = 0;

	while(lugarVacio != 1){
		direccion = (char*) memoria + posicionAnalizada;

		if(obtenerTimestamp(direccion) == 0){
			direccion = memcpy(memoria,&registro.timestamp,sizeof(registro.timestamp));
			posicion += sizeof(registro.timestamp);
			memcpy(&memoria[posicion],&registro.key,sizeof(registro.key));
			posicion += sizeof(registro.key);
			memcpy(&memoria[posicion],registro.value,strlen(registro.value) + 1);
			//posicion += VALUE;

			lugarVacio = 1;

			return direccion;
		}

		posicionAnalizada += MAX_VALUE + sizeof(registro.timestamp) + sizeof(registro.key);
	}
}

char* obtenerValue(void* direccion){
	char* value = malloc(30);

	memcpy(value,(char*)direccion + 6,MAX_VALUE);

	return value;
}

int obtenerTimestamp(void* direccion){
	//int* timestamp = malloc(30);
	int timestamp = 0;

	memcpy(&timestamp,(char*)direccion,sizeof(int));

	return timestamp;
}

int getCurrentTime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}
