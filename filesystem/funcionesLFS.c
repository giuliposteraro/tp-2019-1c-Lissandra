#include "funcionesLFS.h"



nodo_memtable* selectLFS(char* nombreDeTabla, char* key){
	uint16_t ikey = atoi(key);
	nodo_memtable* resultado;

	char* direccionDeLaTabla = direccionDeTabla(nombreDeTabla);

	t_config* metadata;

	pthread_mutex_t* mutexTabla = obtenerMutex(nombreDeTabla);

	pthread_mutex_lock(mutexTabla);
	pthread_mutex_unlock(mutexTabla);


	if(existeLaTabla(nombreDeTabla)){
		metadata = devolverMetadata(direccionDeLaTabla);
		if(metadata){
			int numeroDeParticiones = config_get_int_value(metadata, "PARTITIONS");
			int  particion = calcularParticion(ikey, numeroDeParticiones);
			int longitudNombre = strlen(string_itoa(particion)) + strlen(".bin") + 1;
			char* nombreDelArchivo = malloc(longitudNombre);
			strcpy(nombreDelArchivo,  string_itoa(particion));
			strcat(nombreDelArchivo, ".bin");

			char* direccionDelArchivo = direccionDeArchivo(direccionDeLaTabla, nombreDelArchivo);

			nodo_memtable* registroBin = escanearArchivo( direccionDelArchivo, key, 0); /// TODO

			/*if(registroBin != NULL)
				printf("REGISTRO DE PARTICION: Timestamp = %i Key = %i Value = %s\n", registroBin->timestamp, registroBin->key, registroBin->value);
			else
				printf("EL REGISTRO DE PARTICION ES NULL\n");
			*/
			free(direccionDelArchivo);
			free(nombreDelArchivo);

			nodo_memtable* registroTemporal = buscarEnTemporales(direccionDeLaTabla, key);  /// TODO
			/*
			if(registroTemporal != NULL)
				printf("REGISTRO DE TEMPORALES: Timestamp = %i Key = %i Value = %s\n", registroTemporal->timestamp, registroTemporal->key, registroTemporal->value);
			else
				printf("EL REGISTRO DE TEMPORALES ES NULL\n");
			*/
			nodo_memtable* registroMemtable = buscarMemoriaTemporal(nombreDeTabla, key);

			/*
			if(registroMemtable != NULL)
				printf("REGISTRO DE MEMTABLE: Timestamp = %i Key = %i Value = %s\n", registroMemtable->timestamp, registroMemtable->key, registroMemtable->value);
			else
				printf("EL REGISTRO DE MEMTABLE ES NULL\n");
			*/
			/*free(registroBin);
			free(registroTemporal);
			free(registroMemtable);*/
			resultado = registroMasNuevoYLiberarElViejo(registroMemtable, registroMasNuevoYLiberarElViejo( registroBin, registroTemporal));
			//printf("SE OBTUVO EL RESULTADO\n");
		}else{
			//error_show("No se abrio la metadata");
		}
		config_destroy(metadata);
	}else{
		printf("entre al else\n");
		resultado = NULL;
	}
	free(direccionDeLaTabla);
	return resultado;
}


int insertLFS(char* nombreDeTabla, char* key, char* valor, char* timestamp){
	if(strlen(valor) < tamanioMaximoValue){
	uint32_t tiempo;
	int error = 0;
	if(!string_equals_ignore_case(nombreDeTabla, "USE_TIMESTAMP")){
		tiempo = getCurrentTime();
	}
	else{
		tiempo = atoi(timestamp);
	}
	printf("TIMESTAMP = %i\n", tiempo);
	uint16_t ikey = atoi(key);
	//extern t_dictionary *diccionario;

	if(existeLaTabla(nombreDeTabla)){
		error = 0;
	}
	else{
		error = 1;
	}


	nodo_memtable *nuevoNodo = malloc(sizeof(nodo_memtable));
	nuevoNodo->timestamp = tiempo;
	nuevoNodo->key = ikey;
	nuevoNodo->value = string_duplicate(valor);
	printf("hola\n");
	//nuevoNodo->value = malloc(strlen(valor));
	//strcpy(nuevoNodo->value, valor);

	if(!dictionary_has_key(diccionario, nombreDeTabla)){
		dictionary_put(diccionario, nombreDeTabla, list_create());
	}

	list_add( dictionary_get(diccionario, nombreDeTabla), nuevoNodo);

	return error;
	}else{
		printf("El value es muy grande.\n");
		return 1;
	}
}


int createLFS(char* nombreDeTabla, char* tipoDeConsistencia, char* numeroDeParticiones, char* tiempoDeCompactacion){
	uint8_t inumeroDeParticiones = atoi(numeroDeParticiones);
	int itiempoDeCompactacion = atoi(tiempoDeCompactacion);
	char* nombreDeLaTabla = malloc(strlen(nombreDeTabla)+1);
	strcpy(nombreDeLaTabla, nombreDeTabla);
	printf("Tabla: %s  Tipo de Consistencia: %s  Particiones: %i  Tiempo de Compactacion: %i\n", nombreDeTabla, tipoDeConsistencia, inumeroDeParticiones, itiempoDeCompactacion);
	if(!existeLaTabla(nombreDeLaTabla)){
		crearTabla(nombreDeLaTabla, tipoDeConsistencia, inumeroDeParticiones, itiempoDeCompactacion);
		ingresarTablaEnListaDeTablas(nombreDeLaTabla);
		iniciarSuHiloDeCompactacion(nombreDeLaTabla);
		return 0;
	}
	else{
		return 1;
	}
}

t_list* describeLSF(char* nombreDeTabla){

	t_list* listaDeMetadatas = list_create();
	nodo_lista_tablas* tabla = malloc(sizeof(nodo_lista_tablas));

	if(!strcmp(nombreDeTabla, "DEFAULT")){
		for(int i = 0; i<listaDeTablas->elements_count; i++){
			tabla = list_get(listaDeTablas, i);
			agregarMetadataALaLista(tabla, listaDeMetadatas);

		}

	}
	else{
		//1. Verificar que la tabla exista en el file system.

		if(existeLaTabla(nombreDeTabla)){
			//2. Leer el archivo Metadata de dicha tabla.

			bool buscador(nodo_lista_tablas* elemento){
				return !strcmp(elemento->nombreTabla, nombreDeTabla);
			}
			tabla = list_find(listaDeTablas, (void*)buscador);

			agregarMetadataALaLista(tabla, listaDeMetadatas);

			//printf("El nombre ingresado es: %s\n", nombreDeTabla);
		}
		else {
			logError("Filesystem: NO EXISTE LA TABLA.");
		}
	}
	//3. Retornar el contenido de dichos archivos Metadata.
	return listaDeMetadatas;
}

int dropLSF(char* nombreDeTabla){

	if(existeLaTabla(nombreDeTabla)){
		eliminarTabla(nombreDeTabla);
		return 0;
	}
	else{
		return 1;
	}
}

