/*
 * filesystem.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#include "kernel.h"

/*TAMANIODEARCHIVO
 * Devuelve el tamaño de un archivo.
 */
int32_t tamanioDeArchivo(FILE* f) {
	int32_t previo = ftell(f);
	fseek(f, 0, SEEK_END);
	int32_t tamanio = ftell(f);
	fseek(f, previo, SEEK_SET);
	return tamanio;
}



/* JOURNAL
 * Le envia a memoría el comando para realizar el JOURNAL.
 */
void journal() {
	printf("---select\n");
	int16_t servidor = conectarseA(IP_LOCAL, PUERTO_ESCUCHA_MEM);
	enviarMensaje(JOURNAL, servidor);
	close(servidor);

}


/*	PARSEARLQL
 * Recibe un archivo y un array de punteros, aloca cada fila del txt
 * en una posición del array y retorna la cantidad de elementos que tiene
 * el archivo.
 */

//FUNCIONA PERO SI AL FINAL DEL TXT HAY UN "ENTER" DEVUELVE BASURA
int16_t parsearLQL(FILE* f, char** buffer) {
	int32_t tamanioArchivo = tamanioDeArchivo(f);
	int16_t i = 0;
	while(!feof(f)){
		char* auxiliar = malloc(tamanioArchivo);
		buffer[i] = malloc(30);

		fgets(auxiliar, tamanioArchivo, f);
		if(strlen(auxiliar) > 0){
			strcpy(buffer[i], auxiliar);
			i++;
		}

		free(auxiliar);
	}

	return i;
}




/*PARSEAR POR ESPACIO
 *
 */
char** parsear(char** cosa, char* cadena) {
	cosa = string_split(cadena, " ");
	return cosa;
}


t_list* gestionarBloqueInstucciones(char** bloque, int16_t tamanioBloque) {

	t_list* retorno = list_create();

	for(int i=0; i<tamanioBloque; i++) {
		BloqueInstrucciones* instruccion = malloc(strlen(bloque[i]) + sizeof(int16_t));

		instruccion -> numero = i+1;
		instruccion -> instruccion =  bloque[i];

		list_add(retorno, instruccion);
	}

	return retorno;
}

int gestionarFuncion(char* solicitud) {
	char** spliteado = string_split(solicitud, " ");

	if(!strcmp(spliteado[0], "SELECT")) {
		printf("---select\n");
		// int servidor = conectarseA(ipDeLaMemoria, puertoDeLaMemoria);
		int16_t servidor = conectarseA(IP_LOCAL, PUERTO_ESCUCHA_MEM);
		enviarMensaje("Hola!", servidor);
		close(servidor);
//		enviarMensaje(spliteado[1], PUERTO_ESCUCHA_MEM); //IMPLEMENTACIÓN DE ENVIAR MENSAJE DE PRUEBA
		return 0;
	}

	else if(!strcmp(spliteado[0], "INSERT")) {
		printf("---insert\n");
		return 0;
	}

	else if(!strcmp(spliteado[0], "CREATE")) {
		printf("---create\n");
		return 0;
	}

	else if(!strcmp(spliteado[0], "DESCRIBE")) {
		printf("---describe\n");
		char* cadena = "SELECT eso FROM tabla";
		char** cosa = malloc(sizeof(cadena));


		cosa = parsear(cosa, cadena);
		printf("%s\n", cosa[0]);
		printf("%s\n", cosa[1]);
		printf("%s\n", cosa[2]);

		printf("%s\n", cosa[3]);

		return 0;
	}

	else if(!strcmp(spliteado[0], "DROP")) {
		printf("---drop\n");
		return 0;
	}

	else if(!strcmp(spliteado[0], "JOURNAL")) {
		printf("---journal\n");
		journal();
		return 0;
	}

	else if(!strcmp(spliteado[0], "ADD")) {
		printf("---add\n");
		return 0;
	}

	else if(!strcmp(spliteado[0], "RUN")) {
			printf("---run\n");
			FILE* archivo = fopen("programa.lql", "r");
			char** parseado = malloc(tamanioDeArchivo(archivo) + 500);
			int16_t cantidadLineas = parsearLQL(archivo, parseado);

//			for(int k=0; k<cantidadLineas; k++) {
//				printf("%s\n", parseado[k]);
//			}
			gestionarBloqueInstucciones(parseado, cantidadLineas);
			free(parseado);
			return 0;
	}

	else if(!strcmp(spliteado[0], "METRICS")) {
		printf("---metrics\n");
		return 0;
	}

	else {
		printf("La función no es correcta\n");
		return -1;
	}
}

t_list* gestionarInstuccion(char* linea) {

	t_list* retorno = list_create();
	BloqueInstrucciones* simple = malloc(strlen(linea) + sizeof(int16_t));

	simple -> numero = 1;
	simple -> instruccion = linea;

	list_add(retorno, simple);

	return retorno;
}




int main() {

	t_queue* colaListo = queue_create();
	char * linea;

	while(1) {
		linea = readline(">");

		if (!linea) {
			break;
		}
		queue_push(colaListo, gestionarInstuccion(linea));
		free(linea);
	}



	return 0;
}

