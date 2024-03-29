/*
 * funcionesLFS.h
 *
 *  Created on: 14 may. 2019
 *      Author: utnso
 */

// AQUI SE ENCONTRARAN TODAS LAS FUNCIONES QUE PIDE EL ENUNCIADO

#ifndef FUNCIONESLFS_H_
#define FUNCIONESLFS_H_

#include <commons/string.h>
#include <commons/config.h>
#include <commons/error.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "bibliotecaLFS.h"
#include "../biblioteca/biblioteca_sockets.h"


nodo_memtable* selectLFS(char* nombreDeTabla, char* key);
int insertLFS(char* nombreDeTabla, char* key, char* valor, char* timestamp);
int createLFS(char* nombreDeTabla, char* tipoDeConsistencia, char* numeroDeParticiones, char* tiempoDeCompactacion);
t_list* describeLSF(char* nombreDeTabla);
int dropLSF(char* nombreDeTabla);


#endif /* FUNCIONESLFS_H_ */
