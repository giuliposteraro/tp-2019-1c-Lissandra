/*
 * gossiping.h
 *
 *  Created on: 26 jun. 2019
 *      Author: utnso
 */

#ifndef GOSSIPING_H_
#define GOSSIPING_H_

#include <commons/collections/list.h>
#include "../biblioteca/biblioteca_sockets.h"
#include "../biblioteca/biblioteca.h"
#include "memoria.h"

typedef struct{

	int id;
	uint8_t tam_ip;
	char* ip;
	int puerto;

}t_memoria;

void procesoGossiping();
void iniciarGossiping(int servidor);
void enviarTablaGossiping(int cliente);
t_list* obtenerUnion(t_list* lista1, t_list* lista2);
t_memoria* buscarMemoria(t_list* lista,int id);

#endif /* GOSSIPING_H_ */
