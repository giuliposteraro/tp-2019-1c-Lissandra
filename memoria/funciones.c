/*
 * funciones.c
 *
 *  Created on: 20 may. 2019
 *      Author: utnso
 */

#include "funciones.h"
//#include "segmentacionPaginada.h"

t_request gestionarFuncionKernel(char* solicitud){
	char** spliteado = string_split(solicitud, " ");
	t_request request;

	string_to_upper(spliteado[0]);

	if(!strcmp(spliteado[0], "SELECT")){
		printf("---select\n");
		request.header = 1;
		request.tam_nombre_tabla = strlen(spliteado[1]) + 1;
		request.nombre_tabla = malloc(request.tam_nombre_tabla);
		strcpy(request.nombre_tabla,spliteado[1]);
		request.key = atoi(spliteado[2]);
	}

	else if(!strcmp(spliteado[0], "INSERT")){
		printf("---insert\n");
		request.header = 2;
		strcpy(request.nombre_tabla,spliteado[1]);
		request.key = atoi(spliteado[2]);
		//request.key = strtol(spliteado[2],0); //probar si funciona
		strcpy(request.value,spliteado[3]);
	}

	else if(!strcmp(spliteado[0], "CREATE")){
		printf("---create\n");
	}

	else if(!strcmp(spliteado[0], "DESCRIBE")){
		printf("---describe\n");
	}

	else if(!strcmp(spliteado[0], "DROP")){
		printf("---drop\n");
	}

	else if(!strcmp(spliteado[0], "JOURNAL")){
		printf("---journal\n");
	}

	else{
		printf("La función no es correcta\n");
	}

	return request;
}

void consola(){

	char * linea;
	while(1) {
		linea = readline(">");
		if(linea)
			add_history(linea);
		if(!strncmp(linea, "exit", 4)) {
			free(linea);
			break;
		}
		gestionarFuncionKernel(linea);
		free(linea);
	}
}

void recibirPaquete(int socketCliente){

	int bytesRecibidos;
	t_request prueba;

	void* buffer = malloc(1000);

	bytesRecibidos = recv(socketCliente, buffer, sizeof(prueba.header), 0);

	if(bytesRecibidos <= 0) {
		perror("Error al recibir mensaje");
	}

	memcpy(&prueba.header,buffer,sizeof(prueba.header));
	printf("Recibí la clave: %d \n", prueba.header);

	bytesRecibidos = recv(socketCliente, buffer, sizeof(prueba.tam_nombre_tabla), 0);
	memcpy(&prueba.tam_nombre_tabla,buffer,sizeof(prueba.tam_nombre_tabla));
	printf("El tamano del nombre de la tabla es: %d \n", prueba.tam_nombre_tabla);

	bytesRecibidos = recv(socketCliente, buffer, prueba.tam_nombre_tabla, 0);
	prueba.nombre_tabla = malloc(prueba.tam_nombre_tabla);
	memcpy(prueba.nombre_tabla,buffer,prueba.tam_nombre_tabla);
	printf("El nombre de la tabla es: %s \n\n", prueba.nombre_tabla);

	free(prueba.nombre_tabla);
	free(buffer);

}

t_request armarInsert(char* nombreTabla, uint16_t key, char* value){

	t_request insert;

	strcpy(insert.nombre_tabla,nombreTabla);
	insert.key = key;
	strcpy(insert.value,value);

	return insert;
}

void prueba(void* memoria,t_list* tabla_segmentos){

	t_segmento* segmento_retornado;
	t_registro registro;

	registro.key = 16;
	registro.timestamp = 237;
	registro.value = "Espero que funcione";

	list_add(tabla_segmentos,crearSegmento("TABLA1"));
	segmento_retornado = (t_segmento*)list_get(tabla_segmentos,0);

	list_add(segmento_retornado->tabla_pagina,crearPagina(1,0,memoria,registro));

	//printf("%d\n",&memoria[4]);
	//printf("%d\n",&memoria[5]);
	//printf("%s\n",(char*)memoria + 6);

	//printf("Hasta aca funciona\n");
}

void procesarRequest(void* memoria,t_list* tabla_segmentos,t_request request){
	t_segmento* segmento_encontrado;
	t_pagina* pagina_encontrada;
	char* valueObtenido = malloc(MAX_VALUE);

	switch(request.header){
		case 1:

			segmento_encontrado = buscarSegmento(tabla_segmentos,request.nombre_tabla);
			pagina_encontrada = (t_pagina*)list_get(segmento_encontrado->tabla_pagina,0);
			valueObtenido = obtenerValue(pagina_encontrada->direccion);

			printf("%s\n",valueObtenido);

			break;
		case 2:

			break;
	}

	free(valueObtenido);
}