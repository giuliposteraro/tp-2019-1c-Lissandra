/*
 * funciones.c
 *
 *  Created on: 12 jun. 2019
 *      Author: utnso
 */

#include "funcionesKernel.h"

void procesarRequest(uint8_t id,char* requestString){
	t_nueva_request* nuevaRequest = malloc(sizeof(t_nueva_request));
	t_request request;

	switch(id){
		case SELECT:
		case INSERT:
		case CREATE:
		case DESCRIBE:
		case DROP:
		case RUN:
			nuevaRequest->id = id;
			nuevaRequest->nuevaRequestString = malloc(strlen(requestString));
			strcpy(nuevaRequest->nuevaRequestString,requestString);

			sem_wait(&mutexNuevo);
			queue_push(queue_nuevo,nuevaRequest);
			sem_post(&mutexNuevo);
			sem_post(&semaforoNuevo);
			break;
		case JOURNAL:
			break;
		case ADD:
			request = gestionarSolicitud(requestString);
			agregarMemoria(request.id_memoria,request.tipo_consistencia);
			//liberarMemoriaRequest(request);
			break;
		case METRICS:
			break;
		default:
			break;
	}
}

void atenderNuevos(){
	while(1){

		sem_wait(&semaforoNuevo);
		sem_wait(&mutexNuevo);

		crearEstructura((t_nueva_request*)queue_pop(queue_nuevo));

		sem_post(&mutexNuevo);
	}
}

void atenderListos(){
	while(1){

		sem_wait(&semaforoListo);
		sem_wait(&semaforoExecLibre);

		pthread_t hiloEjecutar;
		pthread_create(&hiloEjecutar,NULL,(void*)ejecutar,queue_pop(queue_listo));
		pthread_detach(hiloEjecutar);

		//sem_wait(&semaforoExecOcupado);
	}
}

void ejecutar(t_queue* script){
	int quantumActual = quantum;
	t_request requestEjecutar;
	t_memoria* memoriaObtenida;
	int idMemoriaAnterior = 0;
	bool activador = false;

	t_response response_recibido;

	//int servidor;
	int servidor = conectarseA(ip_memoria, puerto_memoria);// conexion casera

	while(quantumActual > 0){

		if(queue_is_empty(script)){
			break;
		}

		requestEjecutar = gestionarSolicitud(queue_pop(script));
		//memoriaObtenida = obtenerMemoria(requestEjecutar.nombre_tabla); // obtengo ip y puerto

		/*
		printf("%d ",requestEjecutar.key);
		printf("%s ",requestEjecutar.nombre_tabla);
		if(requestEjecutar.header == 2){
			printf("%s",requestEjecutar.value);
		}
		printf("\n");
		*/

		/*
		if(activador){
			if(idMemoriaAnterior != memoriaObtenida->id){
				close(servidor);
				servidor = conectarseA(memoriaObtenida->ip, memoriaObtenida->puerto);
				idMemoriaAnterior = memoriaObtenida->id;
			}
		}
		else{
			activador = true;
			servidor = conectarseA(ip_memoria, puerto_memoria); // conexion predeterminada
			//servidor = conectarseA(memoriaObtenida->ip, memoriaObtenida->puerto); // conexion verdadera
			idMemoriaAnterior = memoriaObtenida->id;
		}
		*/


		//servidor = conectarseA(IP_LOCAL, PUERTO_ESCUCHA_MEM);// conexion casera
		enviarRequest(servidor,requestEjecutar);
		response_recibido = recibirResponse(servidor);

		if(response_recibido.error){
			printf("No se puedo recibir la respuesta\n");
		}
		else if (response_recibido.header == SELECT_R){
			printf("%s\n",response_recibido.value);
		}
		else if (response_recibido.header == CREATE_R){
			printf("tabla creada correctamente\n");
		}
		else if (response_recibido.header == DROP_R){
			printf("tabla borrada correctamente\n");
		}

		//close(servidor);

		liberarMemoriaRequest(requestEjecutar);
		liberarMemoriaResponse(response_recibido);
		quantumActual--;
	}

	activador = false; // no se si es necesario ponerlo
	//close(servidor);

	printf("\n");
	sleep(3); // cambiarlo por el valor del config

	close(servidor);

	if(queue_is_empty(script) == 0){
		queue_push(queue_listo,script);
		sem_post(&semaforoListo);
	}
	//queue_push(queue_listo,script);

	sem_post(&semaforoExecLibre);

}

void crearEstructura(t_nueva_request* request){
	t_queue * request_string = queue_create();

	switch(request->id){
		case SELECT:
		case INSERT:
		case CREATE:
		case DESCRIBE:
		case DROP:
			sem_wait(&mutexListo);

			queue_push(request_string,crearRequestString(request->nuevaRequestString));
			queue_push(queue_listo,request_string);

			sem_post(&mutexListo);
			sem_post(&semaforoListo);

			break;
		case RUN:

			//printf("%s\n",request->nuevaRequestString);

			sem_wait(&mutexListo);
			queue_push(queue_listo,leerArchivo(gestionarSolicitud(request->nuevaRequestString).path_archivo));
			sem_post(&mutexListo);
			sem_post(&semaforoListo);

			break;
		default:
			break;
	}
}

t_queue* leerArchivo(char * pathArchivo){

	printf("%s\n",pathArchivo);

	FILE * archivo = fopen(pathArchivo,"r");
	char * auxiliar = malloc(100);
	t_queue * request_string = queue_create();

	// RUN /home/utnso/Escritorio/pruebas/tp/comidas.lql
	// RUN /home/utnso/Escritorio/pruebas/tp/animales.lql
	// RUN /home/utnso/Escritorio/pruebas/tp/misc_1.lql
	// RUN /home/utnso/Escritorio/pruebas/tp/películas.lql

	if(archivo == NULL) {
		printf("No se abrio el archivo\n");
	}

	//while(!feof(archivo)){
	while(fgets(auxiliar, 100, archivo) != NULL){

		//token = strtok(auxiliar,"\n");
		//printf("%s\n",token);

		//fgets(auxiliar, 60, archivo);
		auxiliar[strcspn(auxiliar,"\n")] = 0;
		printf("%s\n",auxiliar);

		queue_push(request_string,crearRequestString(auxiliar));

	}

	free(auxiliar);
	fclose(archivo);

	return request_string;
}


char* crearRequestString(char* requestLeido){
	char* requestString = malloc(strlen(requestLeido)+1);
	strcpy(requestString,requestLeido);
	return requestString;
}

void leerArchivoConfig(){
	multiprocesamiento = config_get_int_value(archivo_config,"MULTIPROCESAMIENTO");
	quantum = config_get_int_value(archivo_config,"QUANTUM");
	ip_memoria = config_get_string_value(archivo_config,"IP_MEMORIA");
	puerto_memoria = config_get_int_value(archivo_config,"PUERTO_MEMORIA");
}

// procesoGossiping, no es el mismo de memoria
void procesoGossiping(){
	//int inicio = 1;
	int cliente;
	//char** ip_seeds;
	//char** puerto_seeds;
	//int puerto_seeds_int;
	bool activador = true;

	t_list* tabla_recibida;
	t_memoria* mem_temp; //solo para imprimir la tabla en la consola
	t_memoria* memoriaDesconectada;

	while(1){
		cliente = conectarseA(ip_memoria,puerto_memoria);

		if(cliente != 0){
			//printf("Si se pudo conectar\n");
			iniciarGossiping(cliente);

			tabla_gossiping = recibirTablaGossiping(cliente);

			/*
			if(list_size(tabla_recibida) != 0){
				printf("Se recibio la tabla\n");
				tabla_gossiping = obtenerUnion(tabla_gossiping,tabla_recibida);
			}
			*/

			//printf("%d\n",list_size(tabla_gossiping));

			/*
			for(int i=0; i<list_size(tabla_gossiping); i++){
				mem_temp = list_get(tabla_gossiping,i);
				printf("%d  ",mem_temp->id);
				printf("%d  ",mem_temp->tam_ip);
				printf("%s  ",mem_temp->ip);
				printf("%d\n",mem_temp->puerto);
			}
			*/

			close(cliente);
		}
		else{
			//printf("NO se pudo conectar\n");

			//printf("size tabla gossiping: %d\n",list_size(tabla_gossiping));

			// cambiar despues a buscarMemoriaPorIP
			memoriaDesconectada = buscarMemoriaPorPuerto(tabla_gossiping,puerto_memoria);

			if(memoriaDesconectada != NULL){
				eliminarMemoria(tabla_gossiping,memoriaDesconectada->id);
			}

			/*
			for(int i=0; i<list_size(tabla_gossiping); i++){
				mem_temp = list_get(tabla_gossiping,i);
				printf("%d  ",mem_temp->id);
				printf("%d  ",mem_temp->tam_ip);
				printf("%s  ",mem_temp->ip);
				printf("%d\n",mem_temp->puerto);
			}
			*/
//			if(activador){
//				t_memoria* nuevo = malloc(sizeof(t_memoria));
//				nuevo->ip = strdup("127.0.0.1"); // revisar
//				nuevo->tam_ip = strlen(nuevo->ip) + 1;
//				nuevo->puerto = obtenerPuertoConfig();
//				nuevo->id = obtenerIdMemoria();
//				list_add(tabla_gossiping,nuevo);
//
//				activador = 0;
//			}
		}

		sleep(3);
	}
}

void agregarMemoria(int idMemoria, uint8_t tipoConsistencia){

	switch(tipoConsistencia){
		case SC:
			if(list_size(criterio_SC) < 1){ // solo debe haber una memoria para SC
				sem_wait(&mutexCriterio);
				list_add(criterio_SC,idMemoria);
				sem_post(&mutexCriterio);
			}

			break;
		case SHC:
			break;
		case EC:
			sem_wait(&mutexCriterio);
			list_add(criterio_EC,idMemoria);
			sem_post(&mutexCriterio);

			break;
	}
}

t_memoria* obtenerMemoria(char* nombreTabla){
	//rand() %
	t_tabla* tablaEncontrada;
	t_memoria* memoriaObtenida;
	int idMemoriaObtenido;
	int numeroAleatorio;

	tablaEncontrada = buscarTabla(nombreTabla);

	switch(tablaEncontrada->tipo_consistencia){
		case SC:

			sem_wait(&mutexCriterio);
			idMemoriaObtenido = list_get(criterio_SC,0);
			sem_post(&mutexCriterio);
			//agregar mutex para la tabla gossiping
			memoriaObtenida = buscarMemoria(tabla_gossiping,idMemoriaObtenido);

			break;
		case EC:

			numeroAleatorio = (rand()%list_size(criterio_EC)) + 1;
			sem_wait(&mutexCriterio);
			idMemoriaObtenido = list_get(criterio_EC,numeroAleatorio);
			sem_post(&mutexCriterio);
			//agregar mutex para la tabla gossiping
			memoriaObtenida = buscarMemoria(tabla_gossiping,idMemoriaObtenido);

			break;
	}

	return memoriaObtenida;
}

t_tabla* buscarTabla(char* nombreTabla) {
	int igualNombre(t_tabla* p) {
		return string_equals_ignore_case(p->nombre_tabla, nombreTabla);
	}

	return list_find(metadata_tablas, (void*) igualNombre);
}

void agregarTabla(t_response tablaRecibida) {
    t_tabla* new = malloc(sizeof(t_tabla));
    new->nombre_tabla = strdup(tablaRecibida.nombre_tabla);
    new->tam_nombre_tabla = tablaRecibida.tam_nombre_tabla;
    new->tipo_consistencia = tablaRecibida.tipo_consistencia;
    new->compaction_time = tablaRecibida.compaction_time;
    new->numero_particiones = tablaRecibida.numero_particiones;

    sem_wait(&mutexMetadata);
    list_add(metadata_tablas,new);
    sem_post(&mutexMetadata);

    //revisar, no tiene que haber una tabla repetida
}
