/*
 * funciones.h
 *
 *  Created on: 20 may. 2019
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <commons/string.h>
#include <commons/config.h>

#include "../biblioteca/biblioteca_sockets.h"
#include "segmentacionPaginada.h"

#define MAX_VALUE 30

typedef struct{

	uint8_t header;
	uint8_t tam_nombre_tabla;
	char* nombre_tabla;
	uint16_t key;
	char* value;

}t_request;

t_request gestionarFuncionKernel(char* solicitud);
void consola();
void recibirPaquete(int socketCliente);
t_request armarInsert(char* nombreTabla, uint16_t key, char* value);
void prueba(void* memoria,t_list* tabla_segmentos);
void procesarRequest(void* memoria,t_list* tabla_segmentos,t_request request);

#endif /* FUNCIONES_H_ */