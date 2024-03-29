/*
 * servidor.h
 *
 *  Created on: 2 jun. 2019
 *      Author: utnso
 */

#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "bibliotecaLFS.h"
#include "biblioteca.h"


#define SC 1
#define SHC 2
#define EC 3

int socketDeEscucha;

//void inicializarServidor();
void inicializarServidorV2();
void atenderRequest(int socketCliente);
void crearHiloDeAtencion(int listenningSocket);

#endif /* SERVIDOR_H_ */
