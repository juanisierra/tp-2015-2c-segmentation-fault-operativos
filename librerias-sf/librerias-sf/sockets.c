/*
 * sockets.c
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


int crearSocketEscucha (int cantidadConexiones, char puerto[]) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, puerto, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE
	int socketEscucha;
		socketEscucha = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		if(socketEscucha < 0)
			{
			return socketEscucha;
			}
		if(bind(socketEscucha,serverInfo->ai_addr, serverInfo->ai_addrlen) <0)
			{
			return socketEscucha;
			}
		freeaddrinfo(serverInfo);
		return socketEscucha;
}
int crearSocketCliente (char IP[], char PUERTO[])
{ //Si retorna -2 es error de conexion
	struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
		hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

		getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion
		int serverSocket;
			serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
			if(serverSocket < 0)
					{
					return serverSocket;
					}

			if(connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)<0)
			{
				return -2;
			}
			freeaddrinfo(serverInfo);	// No lo necesitamos mas
			return serverSocket;
}
