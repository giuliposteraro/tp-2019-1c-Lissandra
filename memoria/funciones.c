/*
 * funciones.c
 *
 *  Created on: 20 may. 2019
 *      Author: utnso
 */

#include "funciones.h"

void consola(){

	t_request request_ingresada;


	system("clear");
	printf("------------ MEMORIA ----------------\n");

	char * linea;
	while(1) {
		linea = readline(">");
		if(linea)
			add_history(linea);
		if(!strncmp(linea, "exit", 4)) {
			free(linea);
			break;
		}

		sem_wait(&mutexEscrituraMemoria);
		request_ingresada = gestionarSolicitud(linea);
		procesarRequest(request_ingresada);
		liberarMemoriaRequest(request_ingresada);
		sem_post(&mutexEscrituraMemoria);

		free(linea);
	}

	liberarRecursos();
}

void prueba(void* memoria,t_list* tabla_segmentos){

	t_segmento* segmento_retornado;
	t_registro registro;

	registro.key = 16;
	registro.timestamp = 237;
	registro.value = "Espero que funcione";

	list_add(tabla_segmentos,crearSegmento("TABLA1"));
	segmento_retornado = (t_segmento*)list_get(tabla_segmentos,0);

	list_add(segmento_retornado->tabla_pagina,crearPagina(list_size(segmento_retornado->tabla_pagina),0,registro));

}

void procesarRequest(t_request request){
	t_segmento* segmento_encontrado;
	t_pagina* pagina_encontrada;
	char* valueObtenido = malloc(MAX_VALUE);
	t_pagina* pagina_nueva;
	t_registro registroNuevo;
	int servidor;
	t_log* logger = iniciar_logger("memoria.log");

	switch(request.header){
		case 1://SELECT TABLA1 16

			segmento_encontrado = buscarSegmento(tabla_segmentos,request.nombre_tabla);

			if(segmento_encontrado != NULL){
				pagina_encontrada = buscarPagina(segmento_encontrado->tabla_pagina,request.key);

				if(pagina_encontrada != 0){
					valueObtenido = obtenerValue(pagina_encontrada->direccion);
					printf("%s\n",valueObtenido);
					log_info(logger, "Se ha seleccionado un value que estaba en memoria");
				}
			}else if(segmento_encontrado== NULL){
				//enviarFS(request);
				//RECIBIR VALUE SOLICITADO
				//LO DE ABAJO SERIA GUARDAR LO QUE NOS ENVIARON
				/*int posicionSegmentoNuevo;
				t_segmento* segmento_nuevo;
				posicionSegmentoNuevo = list_add(tabla_segmentos,crearSegmento(request.nombre_tabla));
				segmento_nuevo = (t_segmento*)list_get(tabla_segmentos,posicionSegmentoNuevo);
				list_add(segmento_nuevo->tabla_pagina,crearPagina(0,1,memoria,registroNuevo));*/
			//log_info(logger, "Se ha seleccionado un value que NO estaba en memoria");
			}

			break;
		case 2://INSERT
			segmento_encontrado = buscarSegmento(tabla_segmentos,request.nombre_tabla);
			registroNuevo.key = request.key;
			registroNuevo.value = request.value;
			registroNuevo.timestamp = getCurrentTime();

				if (segmento_encontrado!= NULL){
					pagina_encontrada = buscarPagina(segmento_encontrado->tabla_pagina,registroNuevo.key);

					if(pagina_encontrada != NULL){
						//valueObtenido = obtenerValue(pagina_encontrada->direccion);
						uint32_t timestampObtenido = obtenerTimestamp(pagina_encontrada->direccion);

						if(timestampObtenido < registroNuevo.timestamp){//se actualiza el value
							actualizarRegistro(pagina_encontrada, registroNuevo);
						}
						else if (timestampObtenido >= registroNuevo.timestamp){/*no hay que actualizar*/}
					}
					else if (pagina_encontrada == NULL){// si no la encuentra
						list_add(segmento_encontrado->tabla_pagina,crearPagina(list_size(segmento_encontrado->tabla_pagina),1,registroNuevo));

						//////////////////////
						//ver si hay espacio//
						//////////////////////
					}
				}
				else if (segmento_encontrado == NULL){ // no se encontro el segmento
					int posicionSegmentoNuevo;
					t_segmento* segmento_nuevo;

					posicionSegmentoNuevo = list_add(tabla_segmentos,crearSegmento(request.nombre_tabla));
					segmento_nuevo = (t_segmento*)list_get(tabla_segmentos,posicionSegmentoNuevo);
					list_add(segmento_nuevo->tabla_pagina,crearPagina(0,1,registroNuevo));
				}
				log_info(logger, "Se ha insertado un value.");
			break;
		case 3://CREATE
		
			//enviarFS(request);
			//RECIBIR DE FS EL OK O EL ERROR
			//printf(LO_RECIBIDO);
			//log_info(logger, "Se ha creado una table en el FS.");
			break;
		case 4://DESCRIBE

			//enviarFS(request);
			//RECIBIR DE FS EL OK O EL ERROR
			//printf(LO_RECIBIDO);
			//log_info(logger, "Se ha obtenido la metadata del FS.");
			break;
		case 5://DROP
			//enviarFS(request);
			segmento_encontrado = buscarSegmento(tabla_segmentos,request.nombre_tabla);
			if(segmento_encontrado!= NULL){
				//eliminarSegmento(segmento_encontrado);
				//log_info(logger, "Se ha eliminado un segmento.");
			}

			break;
		case 6://JOURNAL
			//log_info(logger, "Se ha hecho un journal.");
			break;
	}

	free(valueObtenido);
}


void conexionKernel(){
	void * conectado;
	while((conectado=aceptarConexion(puerto))!= 1){
		//printf("Se acepto conexion\n");
		pthread_t hiloRequest;
		pthread_create(&hiloRequest,NULL,(void*)atenderRequest,conectado);
		pthread_detach(hiloRequest);
	}
}

void atenderRequest(void* cliente){
	pthread_mutex_lock(&mutex);
	t_request request_ingresada = recibirRequest(cliente);

	while(request_ingresada.error != 1){

		sem_wait(&mutexEscrituraMemoria);
		procesarRequest(request_ingresada);

		/*
		printf("%d ",request_ingresada.key);
		printf("%s ",request_ingresada.nombre_tabla);
		if(request_ingresada.header == 2){
			printf("%s",request_ingresada.value);
		}
		printf("\n");
		*/

		liberarMemoriaRequest(request_ingresada);
		sem_post(&mutexEscrituraMemoria);

		request_ingresada = recibirRequest(cliente);
	}

	printf("\n");

	close(cliente);
	pthread_mutex_unlock(&mutex);
}
void enviarFS(t_request request){
	servidor = conectarseA(IP_LOCAL, 40904);// conexion casera, tienen que ir los valores del .config
	enviarRequest(servidor,request);
	close(servidor);
}

int obtenerPuertoConfig(){
	return config_get_int_value(archivo_config,"PUERTO");
}

int obtenerTamanioMemo(){
	return config_get_int_value(archivo_config,"TAM_MEM");
}

//tipoRetardo puede ser RETARDO_GOSSIPING RETARDO_JOURNAL RETARDO_MEM RETARDO_FS
void modificarRetardos(char* tipoRetardo,int valorNuevo){
	config_set_value(archivo_config, tipoRetardo, valorNuevo);
}

int obtenerRetardo(char* tipoRetardo){
	return config_get_int_value(archivo_config,tipoRetardo);
}
//se que es repetir codigo pero resulta declarativo
char* obtenerIP_FS(){
	return config_get_string_value(archivo_config,"IP_FS");
}
int obtenerPuertoFS(){
	return config_get_int_value(archivo_config,"PUERTO_FS");
}
char** obtenerIP_SEEDS(){
	return config_get_array_value(archivo_config,"IP_SEEDS");
}

char** obtenerPUERTO_SEEDS(){
	return config_get_array_value(archivo_config,"PUERTO_SEEDS");
}
void liberarRecursos(){
	free(memoria);
	close(puerto);
	config_destroy(archivo_config);
}



/*char* obtenerPath() {
    char buf[PATH_MAX + 1];
    char *res = realpath("memoria.config", buf);
    return res;
}// no se si hace lo que quiero
*/
