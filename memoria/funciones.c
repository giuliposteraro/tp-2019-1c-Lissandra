/*
 * funciones.c
 *
 *  Created on: 20 may. 2019
 *      Author: utnso
 */

#include "funciones.h"

void agregarMemoriaGossiping(){
	t_memoria* nuevo = malloc(sizeof(t_memoria));
	nuevo->ip = strdup(ip); // revisar; ahora esta bien
	nuevo->tam_ip = strlen(ip) + 1;
	nuevo->puerto = puerto_escucha_memoria;
	nuevo->id = numero_memoria;
	list_add(tabla_gossiping,nuevo);
}

void consola(){

	t_request request_ingresada;

	system("clear");
	printf("------------ MEMORIA %d ----------------\n",numero_memoria);

	//notificameQueCambio(PATH_CONFIG);

	char * linea;
	while(1) {
		linea = readline(">");
		if(linea)
			add_history(linea);
		if(!strncmp(linea, "exit", 4)) {
			free(linea);
			break;
		}

//		pthread_t hiloSolicitudConsola;
//		pthread_create(&hiloSolicitudConsola,NULL,(void*)solicitudConsola,linea);
//		pthread_detach(hiloSolicitudConsola);

		request_ingresada = gestionarSolicitud(linea);
		procesarRequest(request_ingresada);
		liberarMemoriaRequest(request_ingresada);


		free(linea);
	}

	liberarRecursos();
}

void solicitudConsola(char* linea){
	t_request request_ingresada;

	request_ingresada = gestionarSolicitud(linea);
	procesarRequest(request_ingresada);
	liberarMemoriaRequest(request_ingresada);

	free(linea);
}

void prueba(void* memoria,t_list* tabla_segmentos){

	t_segmento* segmento_retornado;
	t_pagina* pagina_nueva;
	t_registro registro;

	registro.key = 16;
	registro.timestamp = 237;
	registro.value = "Espero que funcione";

	list_add(tabla_segmentos,crearSegmento("TABLA1"));
	segmento_retornado = (t_segmento*)list_get(tabla_segmentos,0);

	pagina_nueva = crearPagina(obtenerIndicePagina(segmento_retornado->tabla_pagina),0,registro);
	list_add(segmento_retornado->tabla_pagina,pagina_nueva);

	agregarEnListaLRU(segmento_retornado->path,pagina_nueva);

	cantPaginasLibres--;
}

void procesoGossiping(){

	//int inicio = 1;
	int cliente;
	int indice;
	int puerto_seeds_int;
	bool activador = true;

	t_list* tabla_recibida;
	t_memoria* mem_temp; //solo para imprimir la tabla en la consola
	t_memoria* memoriaDesconectada;

	//puerto_seeds_int = atoi(puerto_seeds[0]);

	while(1){
		/*
		//////////////////////////////////////////////////////////////
		//				PRINTF SOLO PARA PROBAR INOTIFY
		printf("idMemoria: %d\n",numero_memoria);
		//////////////////////////////////////////////////////////////
		*/

		while (ip_seeds[indice] != NULL){

			cliente = conectarseA(ip_seeds[indice],atoi(puerto_seeds[indice]));

			if(cliente != 0){
				//printf("Si se pudo conectar\n");

				iniciarGossiping(cliente,FLAG_MEMORIA);
				tabla_recibida = recibirTablaGossiping(cliente);
				pthread_mutex_lock(&mutexTablaGossiping);
				if(list_size(tabla_recibida) != 0){
					//printf("Se recibio la tabla\n");
					obtenerUnion(tabla_gossiping,tabla_recibida);
				}
				enviarTablaGossiping(cliente,tabla_gossiping);
				pthread_mutex_unlock(&mutexTablaGossiping);

				//printf("%d\n",list_size(tabla_gossiping));

				/*
				for(int i=0; i<list_size(tabla_gossiping); i++){
					mem_temp = list_get(tabla_gossiping,i);
					printf("%d\t",mem_temp->id);
					printf("%d\t",mem_temp->tam_ip);
					printf("%s\t",mem_temp->ip);
					printf("%d\t\n",mem_temp->puerto);
				}
				*/

				close(cliente);
			}
			else{
				//printf("NO se pudo conectar\n");

				//printf("size tabla gossiping: %d\n",list_size(tabla_gossiping));

				pthread_mutex_lock(&mutexTablaGossiping);
				//memoriaDesconectada = buscarMemoriaPorPuerto(tabla_gossiping,atoi(puerto_seeds[indice]));
				memoriaDesconectada = buscarMemoriaPorIPyPuerto(tabla_gossiping,ip_seeds[indice],atoi(puerto_seeds[indice]));
				if(memoriaDesconectada != NULL){
					eliminarMemoria(tabla_gossiping,memoriaDesconectada->id);
				}
				pthread_mutex_unlock(&mutexTablaGossiping);

				/*
				for(int i=0; i<list_size(tabla_gossiping); i++){
					mem_temp = list_get(tabla_gossiping,i);
					printf("%d\t",mem_temp->id);
					printf("%d\t",mem_temp->tam_ip);
					printf("%s\t",mem_temp->ip);
					printf("%d\t\n",mem_temp->puerto);
				}
				*/
			}
			indice++;
		}

		usleep(retardo_gossiping);

		indice = 0;
	}
}

void procesoJournal(){

	while(1){

		journal();

		usleep(retardo_journal);
	}
}

t_response procesarRequest(t_request request){

	printf("Procesando request\n");

	t_segmento* segmento_encontrado;
	t_segmento* segmento_nuevo;
	int posicionSegmentoNuevo;
	t_pagina* pagina_encontrada;
	t_pagina* pagina_nueva;
	char* valueObtenido = malloc(max_value);
	t_registro registroNuevo;
	t_list* listaDescribes;

	t_response respuestaFS;
	t_response response;
	t_response* describeRecibido; // solo esta para mostrar por pantalla lo recibido
	//t_response responseRecibido;

	int servidorFS;

	printf("header procesarRequest: %d\n",request.header);

	switch(request.header){
		case SELECT://SELECT TABLA1 16
			usleep(retardo_acceso_memoria);
			pthread_mutex_lock(&mutexAccesoMemoria);
			segmento_encontrado = buscarSegmento(tabla_segmentos,request.nombre_tabla);

			if(segmento_encontrado != NULL){

				printf("se encontro segmento\n");

				pagina_encontrada = buscarPagina(segmento_encontrado->tabla_pagina,request.key);

				if(pagina_encontrada != NULL){

					printf("se encontro pagina\n");

					valueObtenido = obtenerValue(pagina_encontrada->direccion);
					//printf("Value obtenido: %s\n",valueObtenido);
					log_info(logMemoria, "Se ha seleccionado un value que estaba en memoria: %s",valueObtenido);
					agregarEnListaLRU(segmento_encontrado->path,pagina_encontrada);

					response.header = SELECT_R;
					response.tam_value = strlen(valueObtenido) + 1;

					printf("HOLA3\n");
					response.value = malloc(response.tam_value);
					printf("HOLA4\n");

					strcpy(response.value,valueObtenido);
					response.timestamp = 0; // al kernel no le importa el timestamp

					printf("Hasta aca funciona\n");
				}
				else if(pagina_encontrada == NULL){

					printf("NO se encontro pagina\n");

					//pthread_mutex_lock(&mutexMemoriaLlena);
					if(cantPaginasLibres==0){
						if(!liberarMemoriaLRU()){

							printf("MEMORIA LLENA\n");
							log_error(logMemoria, "La memoria esta llena.");
							response.header = FULL_R;
							pthread_mutex_unlock(&mutexAccesoMemoria);
							return response;
						}
					}
					if(cantPaginasLibres > 0){
						usleep(retardo_acceso_filesystem);
						respuestaFS = solicitarFS(request);

						if(respuestaFS.header == ERROR_R){
							//printf("El value no esta en Filesystem\n");
							log_error(logMemoria, "El value no esta en Filesystem");
							liberarMemoriaResponse(respuestaFS); // cuidado

							response.header = ERROR_R;
						}
						else if(respuestaFS.header == SELECT_R){
							registroNuevo.value = respuestaFS.value;
							registroNuevo.timestamp = respuestaFS.timestamp;
							registroNuevo.key = request.key;

							pagina_nueva = crearPagina(obtenerIndicePagina(segmento_encontrado->tabla_pagina),0,registroNuevo); // bit en 0 porque el dato es consistente

							list_add(segmento_encontrado->tabla_pagina,pagina_nueva);
							log_info(logMemoria, "Se ha seleccionado un value que NO estaba en la memoria: %s",respuestaFS.value); // cuidado

							cantPaginasLibres--;

							agregarEnListaLRU(segmento_encontrado->path,pagina_nueva);

							valueObtenido = obtenerValue(pagina_nueva->direccion);

							liberarMemoriaResponse(respuestaFS);
							//pthread_mutex_unlock(&mutexMemoriaLlena);

							response.header = SELECT_R;
							response.tam_value = strlen(valueObtenido) + 1;
							response.value = malloc(response.tam_value);
							strcpy(response.value,valueObtenido);
							response.timestamp = 0; // al kernel no le importa el timestamp
						}
						else {
							printf("Fallo en recibir header\n");
						}
					}

					printf("Hasta aca funciona\n");
				}
			}
			else if(segmento_encontrado == NULL){

				printf("NO se encontro segmento\n");

				//pthread_mutex_lock(&mutexMemoriaLlena);
				if(cantPaginasLibres==0){
					if(!liberarMemoriaLRU()){

						printf("MEMORIA LLENA\n");
						log_error(logMemoria, "La memoria esta llena.");
						response.header = FULL_R;
						pthread_mutex_unlock(&mutexAccesoMemoria);
						return response;
					}
				}
				if(cantPaginasLibres > 0){
					usleep(retardo_acceso_filesystem);
					respuestaFS = solicitarFS(request);

					if(respuestaFS.header == ERROR_R){
						//printf("El value no esta en Filesystem\n");
						log_error(logMemoria, "El value no esta en Filesystem");
						liberarMemoriaResponse(respuestaFS);

						response.header = ERROR_R;
					}
					else if(respuestaFS.header == SELECT_R){

						printf("%s\t",respuestaFS.value);
						printf("%d\t",respuestaFS.tam_value);
						printf("%d\n",respuestaFS.timestamp);

						posicionSegmentoNuevo = list_add(tabla_segmentos,crearSegmento(request.nombre_tabla));
						segmento_nuevo = (t_segmento*)list_get(tabla_segmentos,posicionSegmentoNuevo);

						registroNuevo.value = respuestaFS.value;
						registroNuevo.timestamp = respuestaFS.timestamp;
						registroNuevo.key = request.key;

						pagina_nueva = crearPagina(obtenerIndicePagina(segmento_nuevo->tabla_pagina),0,registroNuevo);

						list_add(segmento_nuevo->tabla_pagina,pagina_nueva);
						log_info(logMemoria, "Se ha seleccionado un value que NO estaba en la memoria: %s",respuestaFS.value);


						cantPaginasLibres--;

						agregarEnListaLRU(segmento_encontrado->path,pagina_nueva);

						valueObtenido = obtenerValue(pagina_nueva->direccion);

						liberarMemoriaResponse(respuestaFS);
						//pthread_mutex_unlock(&mutexMemoriaLlena);

						response.header = SELECT_R;
						response.tam_value = strlen(valueObtenido) + 1;
						response.value = malloc(response.tam_value);
						strcpy(response.value,valueObtenido);
						response.timestamp = 0; // al kernel no le importa el timestamp
					}
					else {
						printf("Fallo en recibir header\n");
					}
				}

				printf("Hasta aca funciona\n");
			}
			pthread_mutex_unlock(&mutexAccesoMemoria);

			printf("FIN SELECT\n");

			break;
		case INSERT:
			usleep(retardo_acceso_memoria);
			pthread_mutex_lock(&mutexAccesoMemoria);
			segmento_encontrado = buscarSegmento(tabla_segmentos,request.nombre_tabla);
			registroNuevo.key = request.key;
			registroNuevo.value = request.value;
			registroNuevo.timestamp = getCurrentTime();

			if (segmento_encontrado!= NULL){
				pagina_encontrada = buscarPagina(segmento_encontrado->tabla_pagina,registroNuevo.key);

				if(pagina_encontrada != NULL){
					//pthread_mutex_lock(&mutexMemoriaLlena);

					uint32_t timestampObtenido = obtenerTimestamp(pagina_encontrada->direccion);

					if(timestampObtenido < registroNuevo.timestamp){//se actualiza el value
						actualizarRegistro(pagina_encontrada, registroNuevo);
						pagina_encontrada->modificado = 1;

						agregarEnListaLRU(segmento_encontrado->path,pagina_encontrada);
						log_info(logMemoria, "Se ha actualizado un value: %s",registroNuevo.value);
					}
					else if (timestampObtenido >= registroNuevo.timestamp){
						log_info(logMemoria, "No se actualizo el value %s porque tiene un timestamp menor.",registroNuevo.value);
					}
					//pthread_mutex_unlock(&mutexMemoriaLlena);
				}
				else if (pagina_encontrada == NULL){// si no la encuentra
					//pthread_mutex_lock(&mutexMemoriaLlena);

					if(cantPaginasLibres==0){
						if(!liberarMemoriaLRU()){

							printf("MEMORIA LLENA\n");
							log_error(logMemoria, "La memoria esta llena.");
							response.header = FULL_R;
							pthread_mutex_unlock(&mutexAccesoMemoria);
							return response;
						}
					}
					if(cantPaginasLibres > 0){
						pagina_nueva = crearPagina(obtenerIndicePagina(segmento_encontrado->tabla_pagina),1,registroNuevo);
						list_add(segmento_encontrado->tabla_pagina,pagina_nueva);

						cantPaginasLibres--;

						agregarEnListaLRU(segmento_encontrado->path,pagina_nueva);
						log_info(logMemoria, "Se ha insertado un value: %s",registroNuevo.value);
						//pthread_mutex_unlock(&mutexMemoriaLlena);
					}
				}
			}
			else if (segmento_encontrado == NULL){ // no se encontro el segmento
				//pthread_mutex_lock(&mutexMemoriaLlena);
				if(cantPaginasLibres==0){
					if(!liberarMemoriaLRU()){

						printf("MEMORIA LLENA\n");
						log_error(logMemoria, "La memoria esta llena.");
						response.header = FULL_R;
						pthread_mutex_unlock(&mutexAccesoMemoria);
						return response;
					}
				}
				if(cantPaginasLibres > 0){
					posicionSegmentoNuevo = list_add(tabla_segmentos,crearSegmento(request.nombre_tabla));
					segmento_nuevo = (t_segmento*)list_get(tabla_segmentos,posicionSegmentoNuevo);

					pagina_nueva = crearPagina(obtenerIndicePagina(segmento_nuevo->tabla_pagina),1,registroNuevo);
					list_add(segmento_nuevo->tabla_pagina,pagina_nueva);

					cantPaginasLibres--;

					agregarEnListaLRU(segmento_nuevo->path,pagina_nueva);
					log_info(logMemoria, "Se ha insertado un value: %s",registroNuevo.value);
					//pthread_mutex_unlock(&mutexMemoriaLlena);
				}
			}
			pthread_mutex_unlock(&mutexAccesoMemoria);

			//printf("paginas libres: %d\n",cantPaginasLibres);

			response.header = INSERT_R;

			break;
		case CREATE:

			usleep(retardo_acceso_filesystem);
			respuestaFS = solicitarFS(request);

			if(respuestaFS.header == CREATE_R){
				response.header = CREATE_R;
				log_info(logMemoria, "La tabla %s ha sido creada.",request.nombre_tabla);
				//printf("Tabla creada\n");
			}
			else if(respuestaFS.header == ERROR_R){
				response.header = ERROR_R;
				log_error(logMemoria, "Error al crear la tabla.");
				//printf("Error al crear tabla\n");
			}
			else {
				printf("Fallo en recibir header\n");
			}

			//printf("create con el header: %d\n", respuestaFS.header);

			//log_info(logMemoria, "Se ha creado una tabla en el FS: %s",request.nombre_tabla);

			//printf("tabla %s\n",request.nombre_tabla);
			//printf("tipo consistencia %d\n",request.tipo_consistencia);
			//printf("particiones %d\n",request.numero_particiones);
			//printf("compaction time %d\n\n",request.compaction_time);

			break;
		case DESCRIBE:	// revisar control de errores
			listaDescribes = list_create();

			servidorFS = conectarseA(ip_fs,puerto_fs);
			enviarRequest(servidorFS,request);
			respuestaFS = recibirResponse(servidorFS);

			//printf("cantidad de describes: %d\n",respuestaFS.cantidad_describe);
			log_info(logMemoria, "La cantidad de describes es: %d",respuestaFS.cantidad_describe);

			if(respuestaFS.header == CANT_DESCRIBE_R){

				for(int i=0;i<respuestaFS.cantidad_describe; i++){
					recibirResponseDescribes(listaDescribes,servidorFS);
				}

				// solo para mostrar por pantalla
				for(int i=0;i<list_size(listaDescribes); i++){
					describeRecibido = list_get(listaDescribes,i);

					printf("El describe recibido es: %s\t%d\n",describeRecibido->nombre_tabla,describeRecibido->tam_nombre_tabla);
					log_info(logMemoria, "El describe recibido es: %s",describeRecibido->nombre_tabla);
					//printf("%s\n",describeRecibido->nombre_tabla);
				}

				log_info(logMemoria, "Se ha obtenido la metadata del FS.");

			}
			else {
				log_error(logMemoria,"Describe no recibido");
			}

			close(servidorFS); // cuidado; no se pq no estaba esto antes

			response.header = CANT_DESCRIBE_R;
			response.cantidad_describe = respuestaFS.cantidad_describe;
			response.lista = listaDescribes;

			break;
		case DROP:
			usleep(retardo_acceso_memoria);
			pthread_mutex_lock(&mutexAccesoMemoria);

			segmento_encontrado = buscarSegmento(tabla_segmentos,request.nombre_tabla);
			if(segmento_encontrado!= NULL){
				dropSegmento(segmento_encontrado);

				log_info(logMemoria, "Se ha eliminado el siguiente segmento en memoria: %s", request.nombre_tabla);
			}
			pthread_mutex_unlock(&mutexAccesoMemoria);

			usleep(retardo_acceso_filesystem);
			respuestaFS = solicitarFS(request);

			if(respuestaFS.header == DROP_R){
				//printf("Se ha hecho el drop en FS\n");
				log_info(logMemoria, "Se ha hecho drop en el FS.");
				response.header = DROP_R;
			}
			else if(respuestaFS.header == ERROR_R){
				//printf("Error al hacer el drop en FS\n");
				log_error(logMemoria, "Error al hacer drop en el FS.");
				response.header = ERROR_R;
			}
			else {
				printf("Fallo en recibir header\n");
			}
			//printf("Se ha hecho un drop y recibimos el siguiente header: %i\n", respuestaFS.header);

			break;
		case JOURNAL:

			journal();

			log_info(logMemoria, "Se ha hecho un journal.");
			response.header = JOURNAL_R;


			break;
	}

	// prueba solo para imprimir
	/*
	for(int i=0; i<list_size(lista_LRU); i++){
		t_registro_LRU* registro_prueba = list_get(lista_LRU,i);
		printf("Path: %s\t",registro_prueba->path);
		printf("Numero Pagina: %d\t",registro_prueba->numeroPagina);
		printf("Modificado: %d\t\n",registro_prueba->modificado);
	}
	*/

	free(valueObtenido);

	printf("Hasta aca funciona\n");

	return response;
}

void servidor(){
	void * conectado;
	while((conectado=aceptarConexion(puerto))!= 1){

		//printf("Se acepto conexion\n");
		pthread_t hiloRequest;
		pthread_create(&hiloRequest,NULL,(void*)atenderRequest,conectado);
		pthread_detach(hiloRequest);
	}
}

void atenderRequest(void* cliente){
	//pthread_mutex_lock(&mutex);
	t_request request_ingresada = recibirRequest(cliente);
	t_response response_generado;
	t_list* tabla_recibida;

	while(request_ingresada.error != 1){

		if(request_ingresada.header == GOSSIPING){

			pthread_mutex_lock(&mutexTablaGossiping);
			enviarTablaGossiping(cliente, tabla_gossiping);
			//printf("Se envio la tabla\n");
			tabla_recibida = recibirTablaGossiping(cliente);
			if(list_size(tabla_recibida) != 0){
				//obtenerUnion(tabla_gossiping,tabla_recibida);
				list_clean_and_destroy_elements(tabla_gossiping,(void*) liberarMemoriaGossiping);
				for(int i=0; i<list_size(tabla_recibida); i++){
					list_add(tabla_gossiping,list_get(tabla_recibida,i));
				}
			}
			pthread_mutex_unlock(&mutexTablaGossiping);

			break;
		}
		else if(request_ingresada.header == GOSSIPING_KERNEL){

			pthread_mutex_lock(&mutexTablaGossiping);
			enviarTablaGossiping(cliente, tabla_gossiping);
			//printf("Se envio la tabla\n");
			pthread_mutex_unlock(&mutexTablaGossiping);

			break;
		}
		else{
			//procesarRequest(request_ingresada);
			do{
				if(flagFullEnviado){
					pthread_mutex_lock(&mutexMemoriaLlena);
					flagFullEnviado = 0;
				}
				response_generado = procesarRequest(request_ingresada);

				//printf("header enviando: %d \n",response_generado.header);

			}while(response_generado.header == FULL_R && flagFullEnviado);

			liberarMemoriaRequest(request_ingresada);

			// se activa flag cuando se envia el mensaje de memoria llena
			if(response_generado.header == FULL_R){ flagFullEnviado = 1; }

			if(response_generado.header == CANT_DESCRIBE_R){
				t_response* describeRecibido;
				t_response structRespuesta;

				enviarCantidadDeDescribes(cliente,response_generado.cantidad_describe);

				for(int i=0; i<response_generado.cantidad_describe; i++){

					describeRecibido = list_get(response_generado.lista, i);

					structRespuesta.header = DESCRIBE_R;
					structRespuesta.tam_nombre_tabla = describeRecibido->tam_nombre_tabla;
					structRespuesta.nombre_tabla = malloc(describeRecibido->tam_nombre_tabla);
					strcpy(structRespuesta.nombre_tabla, describeRecibido->nombre_tabla);
					structRespuesta.compaction_time = describeRecibido->compaction_time;
					structRespuesta.tipo_consistencia = describeRecibido->tipo_consistencia;

					enviarResponse(cliente, structRespuesta);

					free(structRespuesta.nombre_tabla);
				}

				list_destroy(response_generado.lista);
			}
			else{

				// se envia el response generado
				enviarResponse(cliente,response_generado);
				liberarMemoriaResponse(response_generado); // cuidado
			}

			request_ingresada = recibirRequest(cliente);

			if(request_ingresada.header == JOURNAL){
				response_generado = procesarRequest(request_ingresada);
				enviarResponse(cliente,response_generado);
			}
		}

	}

	close(cliente);
}

t_response solicitarFS(t_request request){
	t_response responseFS;

	responseFS.header = ERROR_R;

	int servidor = conectarseA(ip_fs, puerto_fs);

	if(servidor != 0){
		enviarRequest(servidor,request);
		responseFS = recibirResponse(servidor);
		close(servidor);
	}
	return responseFS;
}

int handshakeFS(){
	t_request request;
	t_response response;
	request.header = MAX_TAM_VALUE;
	response = solicitarFS(request);

	if(response.error){ // fallo la conexion
		return 0;
	}

	max_value = response.max_tam_value;
	return 1;
}

//tipoRetardo = RETARDO_GOSSIPING RETARDO_JOURNAL RETARDO_MEM RETARDO_FS
void modificarRetardos(char* tipoRetardo,int valorNuevo){
	  //SE SUPONE QUE ESTA ESTA, QUISE COPIAR LA DE LAS COMMONS Y LA VERDAD QUE MEZCLA DE T.O.d.o
	    char* nuevoRetardo;
	    snprintf(nuevoRetardo, 10, "%d", valorNuevo);
	    config_set_value(archivo_config, tipoRetardo, nuevoRetardo);
	    config_save(archivo_config);
}

char* obtenerIP(){
	return config_get_string_value(archivo_config,"IP");
}

int obtenerPuertoConfig(){
	return config_get_int_value(archivo_config,"PUERTO");
}

int obtenerTamanioMemo(){
	return config_get_int_value(archivo_config,"TAM_MEM");
}

int obtenerRetardo(char* tipoRetardo){
	return config_get_int_value(archivo_config,tipoRetardo);
}

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

int obtenerIdMemoria(){
	return config_get_int_value(archivo_config,"MEMORY_NUMBER");
}

void liberarRecursos(){
	free(memoria);
	close(puerto);
	config_destroy(archivo_config);
	log_destroy(logMemoria);
}

int cantidadDePaginas(int tamanioMemo){
	int cantPaginas = tamanioMemo / sizeof(t_pagina);
	return cantPaginas;
}


void inicializarArchivoConfig(){
	archivo_config = config_create(PATH_CONFIG);

	ip = obtenerIP();
	puerto_escucha_memoria = obtenerPuertoConfig();
	ip_fs = obtenerIP_FS();
	puerto_fs = obtenerPuertoFS();
	ip_seeds = obtenerIP_SEEDS();
	puerto_seeds = obtenerPUERTO_SEEDS();
	numero_memoria = obtenerIdMemoria();
	tamano_memoria = obtenerTamanioMemo();
	retardo_gossiping = obtenerRetardo("RETARDO_GOSSIPING") * 1000;
	retardo_journal = obtenerRetardo("RETARDO_JOURNAL") * 1000;
	retardo_acceso_memoria = obtenerRetardo("RETARDO_MEM") * 1000;
	retardo_acceso_filesystem = obtenerRetardo("RETARDO_FS") * 1000;
}

///////////////////////LOG/////////////////////////////////

void inicializarLogMemo(){
	logMemoria = log_create(PATH_LOG,"memoria",false,LOG_LEVEL_INFO);
}

////////////////////////JOURNAL///////////////////////////////

void journal(){

	t_segmento* segmento_obtenido;
	t_pagina* pagina_obtenida;
	t_list* listaJournal = list_create();

	printf("armando la lista del journal.\n");
	for(int i=0; i<list_size(tabla_segmentos); i++){
		segmento_obtenido = list_get(tabla_segmentos,i);

		for(int z=0; z<list_size(segmento_obtenido->tabla_pagina); z++){
			pagina_obtenida = list_get(segmento_obtenido->tabla_pagina,z);

			if(pagina_obtenida->modificado){

				t_request* request = malloc(sizeof(t_request));

				request->header = INSERT;
				request->tam_nombre_tabla = strlen(segmento_obtenido->path) + 1;
				request->nombre_tabla = malloc(request->tam_nombre_tabla);
				strcpy(request->nombre_tabla,segmento_obtenido->path);
				request->key = obtenerKey(pagina_obtenida->direccion);
				request->timestamp = obtenerTimestamp(pagina_obtenida->direccion);
				request->tam_value = strlen(obtenerValue(pagina_obtenida->direccion)) + 1;
				request->value = malloc(request->tam_value);
				strcpy(request->value,obtenerValue(pagina_obtenida->direccion));

				list_add(listaJournal,request);

			}
		}
	}

	printf("cantidad de inserts: %d\n",list_size(listaJournal));

	//int servidor = conectarseA(ip_fs,puerto_fs);
	enviarListaJournal(servidor,listaJournal);
	//close(servidor);
	list_destroy_and_destroy_elements(listaJournal, (void*)eliminarListaJournal); //comentar si algo sale mal

	usleep(retardo_acceso_memoria);
	pthread_mutex_lock(&mutexAccesoMemoria);
	vaciarMemoria();
	list_clean_and_destroy_elements(lista_LRU,(void*) eliminarRegistroLRU);
	cantPaginasLibres = cantTotalPaginas;
	pthread_mutex_unlock(&mutexAccesoMemoria);
	pthread_mutex_unlock(&mutexMemoriaLlena);
	flagFullEnviado = 0;
}

void enviarListaJournal(int cliente, t_list* listaJournal){
	t_request* request_obtenida;
	int servidor;

	for(int i=0; i<list_size(listaJournal); i++){
		request_obtenida = list_get(listaJournal,i);

		t_request request_journal;
		request_journal.header = request_obtenida->header;
		request_journal.tam_nombre_tabla = request_obtenida->tam_nombre_tabla;
		request_journal.nombre_tabla = malloc(request_journal.tam_nombre_tabla);
		strcpy(request_journal.nombre_tabla,request_obtenida->nombre_tabla);
		request_journal.key = request_obtenida->key;
		request_journal.timestamp = request_obtenida->timestamp;
		request_journal.tam_value = request_obtenida->tam_value;
		request_journal.value = malloc(request_journal.tam_value);
		strcpy(request_journal.value,request_obtenida->value);

		solicitarFS(request_journal);
		//enviarRequest(cliente,request_journal);

		//printf("insert %d\n",i);

		liberarMemoriaRequest(request_journal); //comentar/revisar si algo sale mal
	}
}

void eliminarListaJournal(t_request* request){
	free(request->nombre_tabla);
	free(request->value);
	free(request);
}

//#include <stdio.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <errno.h>
//#include <sys/types.h>
//#include <sys/inotify.h>

int notificameQueCambio(char* pathConfig) {
	char buffer[BUF_LEN];

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	int watch_descriptor = inotify_add_watch(file_descriptor, pathConfig , IN_MODIFY | IN_CREATE | IN_DELETE);

	// El file descriptor creado por inotify, es el que recibe la información sobre los eventos ocurridos
	// para leer esta información el descriptor se lee como si fuera un archivo comun y corriente pero
	// la diferencia esta en que lo que leemos no es el contenido de un archivo sino la información
	// referente a los eventos ocurridos
	int length = read(file_descriptor, buffer, BUF_LEN);
	if (length < 0) {
		perror("read");
	}

	int offset = 0;

	// Luego del read buffer es un array de n posiciones donde cada posición contiene
	// un eventos ( inotify_event ) junto con el nombre de este.
	while (offset < length) {

		// El buffer es de tipo array de char, o array de bytes. Esto es porque como los
		// nombres pueden tener nombres mas cortos que 24 caracteres el tamaño va a ser menor
		// a sizeof( struct inotify_event ) + 24.
		struct inotify_event *event = (struct inotify_event *) &buffer[offset];

		// El campo "len" nos indica la longitud del tamaño del nombre
		if (event->len) {
			// Dentro de "mask" tenemos el evento que ocurrio y sobre donde ocurrio
			if (event->mask & IN_CREATE) {
				printf("The file %s was created.\n", event->name);
			} else if (event->mask & IN_DELETE) {
				printf("The file %s was deleted.\n", event->name);
			} else if (event->mask & IN_MODIFY) {
				printf("The file %s was modified.\n", event->name);
			}
		}
		offset += sizeof (struct inotify_event) + event->len;
	}

	//printf("Hola\n");

	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);

	config_destroy(archivo_config);
	inicializarArchivoConfig();

	return EXIT_SUCCESS;
}

void actualizacionArchivoConfig(){
	while(1){
		notificameQueCambio(PATH_CONFIG);
	}
}
