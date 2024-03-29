#include "biblioteca_sockets.h"

#define MAX_CLIENTES 10



/*	ESCUCHAR
 * Recibe un puerto y es espera conexión en ese puerto.
 * Retorna -1 en caso de que ocurra un error.
 * Retorna el socket escuchado en caso de haber recibido una conexión.
 */


int escuchar(int puerto) {
	int socketDeEscucha;
	struct sockaddr_in direccionCliente;
	direccionCliente.sin_family = AF_INET;
	direccionCliente.sin_addr.s_addr = INADDR_ANY;
	direccionCliente.sin_port = htons(puerto);

	if((socketDeEscucha = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("Error al crear el socket de escucha\n");
		return -1;
	}
	int activador = 1;
	if(setsockopt(socketDeEscucha, SOL_SOCKET, SO_REUSEADDR, (char*) &activador, sizeof(activador)) < 0) {
		printf("Error al usar setsockopt\n");
		return -1;
	}

	if(bind(socketDeEscucha, (struct sockaddr *) &direccionCliente, sizeof(direccionCliente)) < 0) {
		printf("Error al bindear\n");
		return -1;

	}

	if(listen(socketDeEscucha, MAX_CLIENTES) < 0) {
		printf("Error al escuchar\n");
		return -1;
	}
	return socketDeEscucha;
}



/*	CONECTARSEA
 * Recibe un IP y un puerto y se conecta al SV.
 * Retonar 0 en caso de no conectarse.
 * Retonar el ID del socket cliente.
 */


int conectarseA(char* ip, int puerto) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(ip);
	direccionServidor.sin_port = htons(puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		//perror("No se pudo conectar\n");
		return 0;
	}
	return cliente;
}

/*ACEPTARCONEXION
 * Retorna 1 en caso de no poder conectarse.
 * Retorna el ID del socket que se conectó.
 */

int aceptarConexion(int socketEscucha) {
	int nuevoSocket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	if ((nuevoSocket = accept(socketEscucha, (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0) {
		return 1;
	}

	return nuevoSocket;
}


///*ENVIARMENSAJE
// *
// */

int enviarMensaje(char* mensaje, int socketDestino) {
	int envioCorrecto=0;
	int longitudMensaje = strlen(mensaje);
	envioCorrecto = send(socketDestino, mensaje, longitudMensaje + 1, 0);
	return 0;
}


/*RECIBIRMENSAJE
 *  Retorna 1 en caso de haber un error al recibir el mensaje.
 *  Retonar el contenido del mensaje.
 */

char* recibirMensaje(int socketCliente) {
	char* buffer = malloc(1000);

	int bytesRecibidos = recv(socketCliente, buffer, 1000, 0);

	if(bytesRecibidos <= 0) {
		perror("Error al recibir mensaje");
		return 1;
	}

	buffer[bytesRecibidos] = '\0';
	printf("Recibí: %s", buffer);

	return buffer;
}

void enviarRequest(int servidor,t_request request){
	int posicion = 0;

	int tamano_buffer;
	void* buffer;

	switch(request.header){
		case SELECT:

			tamano_buffer = sizeof(request.key) + sizeof(request.header)
			+ sizeof(request.tam_nombre_tabla) + request.tam_nombre_tabla;
			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&request.header,sizeof(request.header));
			posicion += sizeof(request.header);

			memcpy(&buffer[posicion],&request.key,sizeof(request.key));
			posicion += sizeof(request.key);

			memcpy(&buffer[posicion],&request.tam_nombre_tabla,sizeof(request.tam_nombre_tabla));
			posicion += sizeof(request.tam_nombre_tabla);

			memcpy(&buffer[posicion],request.nombre_tabla,request.tam_nombre_tabla);

			break;
		case INSERT:

			tamano_buffer = sizeof(request.key) + sizeof(request.header)
			+ sizeof(request.tam_nombre_tabla) + request.tam_nombre_tabla
			+ sizeof(request.tam_value) + request.tam_value;
			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&request.header,sizeof(request.header));
			posicion += sizeof(request.header);

			memcpy(&buffer[posicion],&request.key,sizeof(request.key));
			posicion += sizeof(request.key);

			memcpy(&buffer[posicion],&request.tam_nombre_tabla,sizeof(request.tam_nombre_tabla));
			posicion += sizeof(request.tam_nombre_tabla);

			memcpy(&buffer[posicion],request.nombre_tabla,request.tam_nombre_tabla);
			posicion += request.tam_nombre_tabla;

			memcpy(&buffer[posicion],&request.tam_value,sizeof(request.tam_value));
			posicion += sizeof(request.tam_value);

			memcpy(&buffer[posicion],request.value,request.tam_value);

			break;
		case CREATE:

			tamano_buffer = sizeof(request.header) + sizeof(request.tam_nombre_tabla) + request.tam_nombre_tabla
			+ sizeof(request.tipo_consistencia) + sizeof(request.numero_particiones) + sizeof(request.compaction_time);

			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&request.header,sizeof(request.header));
			posicion += sizeof(request.header);

			memcpy(&buffer[posicion],&request.tam_nombre_tabla,sizeof(request.tam_nombre_tabla));
			posicion += sizeof(request.tam_nombre_tabla);

			memcpy(&buffer[posicion],request.nombre_tabla,request.tam_nombre_tabla);
			posicion += request.tam_nombre_tabla;

			memcpy(&buffer[posicion],&request.tipo_consistencia,sizeof(request.tipo_consistencia));
			posicion += sizeof(request.tipo_consistencia);

			memcpy(&buffer[posicion],&request.numero_particiones,sizeof(request.numero_particiones));
			posicion += sizeof(request.numero_particiones);

			memcpy(&buffer[posicion],&request.compaction_time,sizeof(request.compaction_time));

			break;
		case DESCRIBE:

			if(request.tam_nombre_tabla){
				tamano_buffer = sizeof(request.header)
				+ sizeof(request.tam_nombre_tabla) + request.tam_nombre_tabla;

				buffer = malloc(tamano_buffer);

				memcpy(&buffer[posicion],&request.header,sizeof(request.header));
				posicion += sizeof(request.header);

				memcpy(&buffer[posicion],&request.tam_nombre_tabla,sizeof(request.tam_nombre_tabla));
				posicion += sizeof(request.tam_nombre_tabla);

				memcpy(&buffer[posicion],request.nombre_tabla,request.tam_nombre_tabla);
			}
			else{
				tamano_buffer = sizeof(request.header) + sizeof(request.tam_nombre_tabla);

				buffer = malloc(tamano_buffer);

				memcpy(&buffer[posicion],&request.header,sizeof(request.header));
				posicion += sizeof(request.header);

				memcpy(&buffer[posicion],&request.tam_nombre_tabla,sizeof(request.tam_nombre_tabla));
			}

			break;
		case DROP:

			tamano_buffer = sizeof(request.header)
			+ sizeof(request.tam_nombre_tabla) + request.tam_nombre_tabla;

			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&request.header,sizeof(request.header));
			posicion += sizeof(request.header);

			memcpy(&buffer[posicion],&request.tam_nombre_tabla,sizeof(request.tam_nombre_tabla));
			posicion += sizeof(request.tam_nombre_tabla);

			memcpy(&buffer[posicion],request.nombre_tabla,request.tam_nombre_tabla);

			break;
		case MAX_TAM_VALUE:
		case JOURNAL:

			tamano_buffer = sizeof(request.header);

			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&request.header,sizeof(request.header));

			break;
	}

	send(servidor,buffer,tamano_buffer,0);

	free(buffer);
}

void enviarResponse(int cliente,t_response response){
	int posicion = 0;

	int tamano_buffer;
	void* buffer;

	switch(response.header){
		case SELECT_R:

			tamano_buffer = sizeof(response.header)	+ response.tam_value + sizeof(response.tam_value) + sizeof(response.timestamp);
			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&response.header,sizeof(response.header));
			posicion += sizeof(response.header);

			memcpy(&buffer[posicion],&response.tam_value,sizeof(response.tam_value));
			posicion += sizeof(response.tam_value);

			memcpy(&buffer[posicion],response.value,response.tam_value);
			posicion += response.tam_value;

			memcpy(&buffer[posicion],&response.timestamp,sizeof(response.timestamp));

			break;
		case DESCRIBE_R:

			tamano_buffer = sizeof(response.header)	+ response.tam_nombre_tabla + sizeof(response.tam_nombre_tabla)
			+ sizeof(response.tipo_consistencia) + sizeof(response.numero_particiones) + sizeof(response.compaction_time);

			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&response.header,sizeof(response.header));
			posicion += sizeof(response.header);

			memcpy(&buffer[posicion],&response.tam_nombre_tabla, sizeof(response.tam_nombre_tabla));
			posicion +=  sizeof(response.tam_nombre_tabla);

			memcpy(&buffer[posicion],response.nombre_tabla,response.tam_nombre_tabla);
			posicion += response.tam_nombre_tabla;

			memcpy(&buffer[posicion],&response.tipo_consistencia,sizeof(response.tipo_consistencia));
			posicion += sizeof(response.tipo_consistencia);

			memcpy(&buffer[posicion],&response.numero_particiones,sizeof(response.numero_particiones));
			posicion += sizeof(response.numero_particiones);

			memcpy(&buffer[posicion],&response.compaction_time,sizeof(response.compaction_time));

			break;
		case MAX_TAM_VALUE_R:

			tamano_buffer = sizeof(response.header) + sizeof(response.max_tam_value);
			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&response.header,sizeof(response.header));
			posicion += sizeof(response.header);

			memcpy(&buffer[posicion],&response.max_tam_value, sizeof(response.max_tam_value));

			break;
		case INSERT_R:
		case CREATE_R:
		case DROP_R:
		case JOURNAL_R:
		case FULL_R:
		case ERROR_R:
			tamano_buffer = sizeof(response.header);
			buffer = malloc(tamano_buffer);

			memcpy(&buffer[posicion],&response.header,sizeof(response.header));

			break;
	}

	send(cliente,buffer,tamano_buffer,0);

	free(buffer);
}

void enviarCantidadDeDescribes(int cliente,uint8_t cantidadDeDescribes){
	int posicion = 0;
	uint8_t header = CANT_DESCRIBE_R;
	int tamano_buffer = sizeof(uint8_t) * 2;
	void* buffer = malloc(tamano_buffer);

	memcpy(&buffer[posicion], &header,sizeof(uint8_t));
	posicion += sizeof(uint8_t);
	memcpy(&buffer[posicion],&cantidadDeDescribes,sizeof(uint8_t));

	send(cliente,buffer,tamano_buffer,0);

	free(buffer);
}

t_request recibirRequest(int servidor){

	int bytesRecibidos;
	t_request request;

	void* buffer = malloc(1000);

	bytesRecibidos = recv(servidor, buffer, sizeof(request.header), 0);
	request.error = 0;

	if(bytesRecibidos <= 0) {
		perror("Se desconecto el cliente");
		request.error = 1;
		return request;
	}

	memcpy(&request.header,buffer,sizeof(request.header));

	switch(request.header){
		case SELECT:

			recv(servidor, buffer, sizeof(request.key), 0);
			memcpy(&request.key,buffer,sizeof(request.key));

			recv(servidor, buffer, sizeof(request.tam_nombre_tabla), 0);
			memcpy(&request.tam_nombre_tabla,buffer,sizeof(request.tam_nombre_tabla));

			recv(servidor, buffer, request.tam_nombre_tabla, 0);
			request.nombre_tabla = malloc(request.tam_nombre_tabla);
			memcpy(request.nombre_tabla,buffer,request.tam_nombre_tabla);

			break;
		case INSERT:

			recv(servidor, buffer, sizeof(request.key), 0);
			memcpy(&request.key,buffer,sizeof(request.key));

			recv(servidor, buffer, sizeof(request.tam_nombre_tabla), 0);
			memcpy(&request.tam_nombre_tabla,buffer,sizeof(request.tam_nombre_tabla));

			recv(servidor, buffer, request.tam_nombre_tabla, 0);
			request.nombre_tabla = malloc(request.tam_nombre_tabla);
			memcpy(request.nombre_tabla,buffer,request.tam_nombre_tabla);

			recv(servidor, buffer, sizeof(request.tam_value), 0);
			memcpy(&request.tam_value,buffer,sizeof(request.tam_value));

			recv(servidor, buffer, request.tam_value, 0);
			request.value = malloc(request.tam_value);
			memcpy(request.value,buffer,request.tam_value);

			break;
		case CREATE:

			recv(servidor, buffer, sizeof(request.tam_nombre_tabla), 0);
			memcpy(&request.tam_nombre_tabla,buffer,sizeof(request.tam_nombre_tabla));

			recv(servidor, buffer, request.tam_nombre_tabla, 0);
			request.nombre_tabla = malloc(request.tam_nombre_tabla);
			memcpy(request.nombre_tabla,buffer,request.tam_nombre_tabla);

			recv(servidor, buffer, sizeof(request.tipo_consistencia), 0);
			memcpy(&request.tipo_consistencia,buffer,sizeof(request.tipo_consistencia));

			recv(servidor, buffer, sizeof(request.numero_particiones), 0);
			memcpy(&request.numero_particiones,buffer,sizeof(request.numero_particiones));

			recv(servidor, buffer, sizeof(request.compaction_time), 0);
			memcpy(&request.compaction_time,buffer,sizeof(request.compaction_time));

			break;
		case DESCRIBE:	/*no esta terminado*/
			recv(servidor, buffer, sizeof(request.tam_nombre_tabla), 0);
			memcpy(&request.tam_nombre_tabla,buffer,sizeof(request.tam_nombre_tabla));

			if(request.tam_nombre_tabla){
				recv(servidor, buffer, request.tam_nombre_tabla, 0);
				request.nombre_tabla = malloc(request.tam_nombre_tabla);
				memcpy(request.nombre_tabla,buffer,request.tam_nombre_tabla);
			}

			break;
		case DROP:
			recv(servidor, buffer, sizeof(request.tam_nombre_tabla), 0);
			memcpy(&request.tam_nombre_tabla,buffer,sizeof(request.tam_nombre_tabla));

			recv(servidor, buffer, request.tam_nombre_tabla, 0);
			request.nombre_tabla = malloc(request.tam_nombre_tabla);
			memcpy(request.nombre_tabla,buffer,request.tam_nombre_tabla);
			break;
		case JOURNAL:
			break;
		case GOSSIPING:
			break;
		case MAX_TAM_VALUE:
			break;
	}

	free(buffer);

	return request;
}

t_response recibirResponse(int servidor){

	int bytesRecibidos;
	t_response response;

	void* buffer = malloc(100); //antes era 1000

	bytesRecibidos = recv(servidor, buffer, sizeof(response.header), 0);
	response.error = 0;

	if(bytesRecibidos <= 0) {
		perror("Se desconecto el cliente");
		response.error = 1;
		return response;
	}

	memcpy(&response.header,buffer,sizeof(response.header));

	switch(response.header){
		case SELECT_R:

			recv(servidor, buffer, sizeof(response.tam_value), 0);
			memcpy(&response.tam_value,buffer,sizeof(response.tam_value));

			recv(servidor, buffer, response.tam_value, 0);
			response.value = malloc(response.tam_value);
			memcpy(response.value,buffer,response.tam_value);

			recv(servidor, buffer, sizeof(response.timestamp), 0);
			memcpy(&response.timestamp,buffer,sizeof(response.timestamp));

			break;
		case INSERT_R:


			break;
		case CREATE_R:


			break;
		case DESCRIBE_R:

			recv(servidor, buffer, sizeof(response.tam_nombre_tabla), 0);
			memcpy(&response.tam_nombre_tabla,buffer,sizeof(response.tam_nombre_tabla));

			recv(servidor, buffer, response.tam_nombre_tabla, 0);
			response.nombre_tabla = malloc(response.tam_nombre_tabla);
			memcpy(response.nombre_tabla,buffer,response.tam_nombre_tabla);

			recv(servidor, buffer, sizeof(response.tipo_consistencia), 0);
			memcpy(&response.tipo_consistencia,buffer,sizeof(response.tipo_consistencia));

			recv(servidor, buffer, sizeof(response.numero_particiones), 0);
			memcpy(&response.numero_particiones,buffer,sizeof(response.numero_particiones));

			recv(servidor, buffer, sizeof(response.compaction_time), 0);
			memcpy(&response.compaction_time,buffer,sizeof(response.compaction_time));

			break;
		case DROP_R:
		case ERROR_R:
		case JOURNAL_R:

			break;
		case CANT_DESCRIBE_R:
			recv(servidor, buffer, sizeof(response.cantidad_describe), 0);
			memcpy(&response.cantidad_describe, buffer,sizeof(response.cantidad_describe));

			break;
		case FULL_R:

			break;
		case MAX_TAM_VALUE_R:
			recv(servidor, buffer, sizeof(response.max_tam_value), 0);
			memcpy(&response.max_tam_value, buffer,sizeof(response.max_tam_value));

			break;
	}

	free(buffer);

	return response;
}

void recibirResponseDescribes(t_list* listaDeResponseDescribe, int servidor){
	t_response* new = malloc(sizeof(t_response));
	t_response respuestaFS = recibirResponse(servidor);

	new->tam_nombre_tabla = respuestaFS.tam_nombre_tabla;
	new->nombre_tabla = malloc(respuestaFS.tam_nombre_tabla);
	strcpy(new->nombre_tabla,respuestaFS.nombre_tabla);
	new->tipo_consistencia = respuestaFS.tipo_consistencia;
	new->compaction_time = respuestaFS.compaction_time;
	new->numero_particiones = respuestaFS.numero_particiones;

	list_add(listaDeResponseDescribe, new);

	free(respuestaFS.nombre_tabla);
}
