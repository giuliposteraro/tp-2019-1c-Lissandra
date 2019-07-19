/*
 * funcionesLFS.c
 *
 *  Created on: 14 may. 2019
 *      Author: utnso
 */

#include "funcionesLFS.h"

nodo_memtable* selectLFS(char* nombreDeTabla, char* key){
	uint16_t ikey = atoi(key);
	nodo_memtable* valor;
	t_response* response;

	char* direccionDeLaTabla = direccionDeTabla(nombreDeTabla);
	DIR* dir;

	t_config* metadata;

	pthread_mutex_t mutexTabla = obtenerMutex(nombreDeTabla);

	pthread_mutex_lock(&mutexTabla);
	pthread_mutex_unlock(&mutexTabla);
	//	Esta operación incluye los siguientes pasos:
	//	1. Verificar que la tabla exista en el file system.
	if(dir = existeLaTabla(nombreDeTabla)){
		//	2. Obtener la metadata asociada a dicha tabla.
		metadata = devolverMetadata(direccionDeLaTabla);
		if(metadata){
			//	3. Calcular cual es la par􀆟ción que con􀆟ene dicho KEY.
			// char* particiones = "PARTICIONES";
			int numeroDeParticiones = config_get_int_value(metadata, "PARTITIONS");
			int  particion = calcularParticion(ikey, numeroDeParticiones);
			//	4. Escanear la par􀆟ción obje􀆟vo, todos los archivos temporales y la memoria temporal de dicha
			//	tabla (si existe) buscando la key deseada.
			char* nombreDelArchivo = malloc(10);
			strcpy(nombreDelArchivo,  string_itoa(particion));
			strcat(nombreDelArchivo, ".bin");

			char* direccionDelArchivo = direccionDeArchivo(direccionDeLaTabla, nombreDelArchivo);
			nodo_memtable* registroBin = escanearArchivo( direccionDelArchivo, key, 0);
			//free(direccionDelArchivo);
			//free(nombreDelArchivo);

			nodo_memtable* registroTemporal = buscarEnTemporales(direccionDeLaTabla, key);

			nodo_memtable* registroMemtable = buscarMemoriaTemporal(nombreDeTabla, key);

			//	5. Encontradas las entradas para dicha Key, se retorna el valor con el Timestamp más grande.
			//char** registroSpliteado = string_split(registroMasNuevo(registroMemtable, registroMasNuevo( registroBin, registroTemporal)), ";");
			/*free(registroBin);
			free(registroTemporal);
			free(registroMemtable);*/
			valor = registroMasNuevo(registroMemtable, registroMasNuevo( registroBin, registroTemporal));
			//liberarCharAsteriscoAsterisco(registroSpliteado);
		}else{
			//error_show("No se abrio la metadata");
		}
		config_destroy(metadata);
	}else if (ENOENT == errno)
	{
		/* Directory does not exist. */
	}
	else
	{
		// opendir() failed for some other reason.
	}
	closedir(dir);
	//free(direccionDeLaTabla);
	return valor;
}


int insertLFS(char* nombreDeTabla, char* key, char* valor, char* timestamp){ // necesito control de errores?
	uint32_t tiempo;
	time_t resultado;
	int error = 0;
	if(!string_equals_ignore_case(nombreDeTabla, "USE_TIMESTAMP")){
		tiempo = getCurrentTime(); // no se para que sirve resultado. si no lo pongo me tira error.
	}
	else{
		tiempo = atoi(timestamp);
	}
	uint16_t ikey = atoi(key);
	extern t_dictionary *diccionario;

	printf("El nombre ingresado es: %s\n", nombreDeTabla);
	printf("La key ingresada es: %i\n", ikey);
	printf("El valor ingresado es: %s\n", valor);
	printf("El timestamp ingresado es: %i\n", tiempo);

	char* direccionDeLaTabla = direccionDeTabla(nombreDeTabla);
	DIR* dir;
	t_config* metadata;



	//1. Verificar que la tabla exista en el file system. En caso que no exista, informa el error y
	//con􀆟núa su ejecución.
	if(dir = existeLaTabla(nombreDeTabla)){
		error = 0;
	}
	else if (ENOENT == errno){
		error = 1;
		error_show("No existe la %s en el filesystem", nombreDeTabla);
	}
	else{
		// opendir() failed for some other reason.
	}
	nodo_memtable *nuevoNodo = malloc(sizeof(nodo_memtable));
	nuevoNodo->timestamp = tiempo;
	nuevoNodo->key = ikey;
	nuevoNodo->value = malloc(strlen(valor));
	strcpy(nuevoNodo->value, valor);

	//2. Verificar si existe en memoria una lista de datos a dumpear. De no exis􀆟r, alocar dicha
	//memoria.
	if(!dictionary_has_key(diccionario, nombreDeTabla)){
		dictionary_put(diccionario, nombreDeTabla, list_create());
	}
	//3. El parámetro Timestamp es opcional. En caso que un request no lo provea (por ejemplo
	//insertando un valor desde la consola), se usará el valor actual del Epoch UNIX.
	//4. Insertar en la memoria temporal

	// considero que aca deberia usar un semaforo
	list_add( dictionary_get(diccionario, nombreDeTabla), nuevoNodo);


	closedir(dir);
	free(direccionDeLaTabla);
	return error;
}


int createLFS(char* nombreDeTabla, char* tipoDeConsistencia, char* numeroDeParticiones, char* tiempoDeCompactacion){
	uint8_t inumeroDeParticiones = atoi(numeroDeParticiones);
	int itiempoDeCompactacion = atoi(tiempoDeCompactacion);
	printf("El nombre ingresado es: %s\n", nombreDeTabla);
	printf("El tipo de consistencia ingresada es: %s\n", tipoDeConsistencia);
	printf("El numero de particiones ingresado es: %i\n", inumeroDeParticiones);
	if(existeLaTabla(nombreDeTabla)){
		crearTabla(nombreDeTabla, tipoDeConsistencia, inumeroDeParticiones, itiempoDeCompactacion);
		ingresarTablaEnListaDeTablas(nombreDeTabla);
		iniciarSuHiloDeCompactacion(nombreDeTabla);
		return 0;
	}
	else{
		return 1;
	}
}

t_list* describeLSF(char* nombreDeTabla){

	t_list* listaDeMetadatas = list_create();
	datos_metadata* datos;
	nodo_lista_tablas* tabla;

	if(!strcmp(nombreDeTabla, "DEFAULT")){
		//1. Recorrer el directorio de árboles de tablas y descubrir cuales son las tablas que dispone el
		//sistema.

		//char* direccion_tablas = obtenerDireccionDirectorio("/tables/");



		//t_list* listaDeDirectorios = listarDirectorio(direccion_tablas);
		//2. Leer los archivos Metadata de cada tabla.
		for(int i = 0; i<listaDeTablas->elements_count; i++){
			//datos = conseguirSuMetadataEn_datos_metadata(list_get(listaDeDirectorios, i));
			tabla = list_get(listaDeTablas, i);
			datos->consistencia = malloc(strlen(tabla->consistencia));
			strcpy(datos->consistencia, tabla->consistencia);
			datos->nombreTabla = malloc(strlen(tabla->nombreTabla));
			strcpy(datos->nombreTabla, tabla->nombreTabla);
			datos->particiones = tabla->particiones;
			datos->tiempoDeCompactacion = tabla->tiempoDeCompactacion;

			list_add(listaDeMetadatas, datos);
		}
		//list_iterate(listaDeDirectorios, (void*)agregarSuMetadataALaLista(listaDeMetadatas));

	}
	else{
		//1. Verificar que la tabla exista en el file system.

		if(existeLaTabla(nombreDeTabla)){
			//2. Leer el archivo Metadata de dicha tabla.

			bool buscador(nodo_lista_tablas* elemento){
				return !strcmp(elemento->nombreTabla, nombreDeTabla);
			}
			tabla = list_find(listaDeTablas, (void*)buscador);

			datos->consistencia = malloc(strlen(nombreDeTabla));
			strcpy(datos->consistencia, nombreDeTabla);
			datos->nombreTabla = malloc(strlen(tabla->nombreTabla));
			strcpy(datos->nombreTabla, tabla->nombreTabla);
			datos->particiones = tabla->particiones;
			datos->tiempoDeCompactacion = tabla->tiempoDeCompactacion;

			list_add(listaDeMetadatas, datos);

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
	printf("El nombre ingresado es: %s\n", nombreDeTabla); // USAR RM-R PARA BORRAR EL DIRECTORIO
	if(existeLaTabla(nombreDeTabla)){
		eliminarTabla(nombreDeTabla);
		return 0;
	}
	else{
		return 1;
	}
}

