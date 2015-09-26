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
#include "tiposDato.h"
#include <stdint.h>

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

int enviarPCB(int socket,nodoPCB* PCB, uint32_t quantum) //-2 si no hay malloc
{	int resultado;
	mensaje_PL_CPU* mensaje;
	mensaje = malloc(sizeof(mensaje_PL_CPU));
	if(mensaje!=NULL){
	memcpy(&(mensaje->pid),&(PCB->info.pid),sizeof(uint32_t));
	memcpy(&(mensaje->ip),&(PCB->info.ip),sizeof(uint32_t));
	memcpy(&(mensaje->path),&(PCB->info.path),51*sizeof(char));
	memcpy(&(mensaje->quantum),&quantum,sizeof(uint32_t));
	printf("Antes de enviar mensaje: %s %d\n",mensaje->path,mensaje->pid);
	resultado = send(socket,(void*) mensaje,sizeof(mensaje_PL_CPU),0);
	free(mensaje);
	}
	else {
		resultado=-2;
	}
	return resultado;
}
int recibirPCB(int socket, proceso_CPU* proceso,uint32_t *quantum)
{	int resultado;
	mensaje_PL_CPU* mensajeRecibido;
	mensajeRecibido=malloc(sizeof(mensaje_PL_CPU));
	resultado = recv(socket,(void*) mensajeRecibido,sizeof(mensaje_PL_CPU),0);
	memcpy(&(proceso->pid),&(mensajeRecibido->pid),sizeof(uint32_t));
	memcpy(&(proceso->ip),&(mensajeRecibido->ip),sizeof(uint32_t));
	memcpy(&(proceso->path),&(mensajeRecibido->path),51*sizeof(char));
	memcpy(quantum,&(mensajeRecibido->quantum),sizeof(uint32_t));
	free(mensajeRecibido);
	return resultado;

}

int enviarInstruccionAlADM(int socket, mensaje_CPU_ADM* mensajeAMandar) //el CPU le manda al ADM
{
	int resultado;
	mensaje_CPU_ADM* mensaje1;
	char* mensaje2;
	mensaje1 = malloc(sizeof(mensaje_CPU_ADM));
	memcpy(&(mensaje1->instruccion), &(mensajeAMandar->instruccion), sizeof(instruccion_t));
	memcpy(&(mensaje1->pid), &(mensajeAMandar->pid), sizeof(uint32_t));
	memcpy(&(mensaje1->parametro), &(mensajeAMandar->parametro), sizeof(uint32_t));
	memcpy(&(mensaje1->tamTexto), &(mensajeAMandar->tamTexto), sizeof(uint32_t));
	resultado = send(socket,(void*) mensaje1,(sizeof(mensaje_CPU_ADM)),0);
	mensaje2 = malloc(sizeof(char)*(mensajeAMandar->tamTexto));
	memcpy(mensaje2, (mensajeAMandar->texto), sizeof(char)*(mensajeAMandar->tamTexto));
	resultado = send(socket,(void*) mensaje2,sizeof(char)*(mensajeAMandar->tamTexto),0);
	free(mensaje1);
	free(mensaje2);
	return resultado;
}

int recibirInstruccionDeCPU(int socket, mensaje_CPU_ADM* mensajeRecibido) //el ADM recibe el mensaje que le manda el CPU
{
	int resultado;
	mensaje_CPU_ADM* mensaje1;
	mensaje1 = malloc(sizeof(mensaje_CPU_ADM));
	resultado = recv(socket,(void*) mensaje1,sizeof(mensaje_PL_CPU),0);
	memcpy(&(mensajeRecibido->instruccion), &(mensaje1->instruccion), sizeof(instruccion_t));
	memcpy(&(mensajeRecibido->pid), &(mensaje1->pid), sizeof(uint32_t));
	memcpy(&(mensajeRecibido->parametro), &(mensaje1->parametro), sizeof(uint32_t));
	memcpy(&(mensajeRecibido->tamTexto), &(mensaje1->tamTexto), sizeof(uint32_t));
	mensaje1->texto = malloc(sizeof(char)*(mensaje1->tamTexto));
	resultado = recv(socket,(void*) mensaje1->texto, sizeof(char)*(mensaje1->tamTexto), 0);
	mensajeRecibido->texto = malloc(sizeof(char)*(mensaje1->tamTexto));
	memcpy(mensajeRecibido->texto, mensaje1->texto, sizeof(char)*(mensaje1->tamTexto));
	free(mensaje1);
	return resultado;
}

int enviarRetornoInstruccion(int socket, mensaje_ADM_CPU* mensajeAMandar) // El ADM le manda al CPU
{
		int resultado;
		mensaje_ADM_CPU* mensaje1;
		char* mensaje2;
		mensaje1 = malloc(sizeof(mensaje_ADM_CPU));
		memcpy(&(mensaje1->parametro), &(mensajeAMandar->parametro), sizeof(uint32_t));
		memcpy(&(mensaje1->tamanoMensaje), &(mensajeAMandar->tamanoMensaje), sizeof(uint32_t));
		resultado = send(socket,(void*) mensaje1,(sizeof(mensaje_ADM_CPU)),0);
		mensaje2 = malloc(sizeof(char)*(mensajeAMandar->tamanoMensaje));
		memcpy(mensaje2, (mensajeAMandar->texto), sizeof(char)*(mensajeAMandar->tamanoMensaje));
		resultado = send(socket,(void*) mensaje2,sizeof(char)*(mensajeAMandar->tamanoMensaje),0);
		free(mensaje1);
		free(mensaje2);
		return resultado;
}
int recibirRetornoInstruccion(int socket, mensaje_ADM_CPU* mensajeRecibido)// el CPU recibe el mensaje del ADM
{
	int resultado;
	mensaje_ADM_CPU* mensaje1;
	mensaje1 = malloc(sizeof(mensaje_ADM_CPU));
	resultado = recv(socket,(void*) mensaje1,sizeof(mensaje_ADM_CPU),0);
	memcpy(&(mensajeRecibido->parametro), &(mensaje1->parametro), sizeof(uint32_t));
	memcpy(&(mensajeRecibido->tamanoMensaje), &(mensaje1->tamanoMensaje), sizeof(uint32_t));
	mensaje1->texto = malloc(sizeof(char)*(mensaje1->tamanoMensaje));
	resultado = recv(socket,(void*) mensaje1->texto, sizeof(char)*(mensaje1->tamanoMensaje), 0);
	memcpy(&(mensajeRecibido->texto), &(mensaje1->texto), sizeof(char)*(mensaje1->tamanoMensaje));
	free(mensaje1);
	return resultado;

}
int enviarMensajeAPL(proceso_CPU datos_CPU, estado_t estado, uint32_t tiempoBloqueo, retornoInstruccion* payload, uint32_t tamPayload)//enviamos los retornos de instreuccion al PL, tamPayload es el tamaño real que ocupa, ya medido
{
	int resultado;
	mensaje_CPU_PL* mensaje1;
	char* mensaje2;
	mensaje1 = malloc(sizeof(mensaje_CPU_PL));
	memcpy(&(mensaje1->ip),&(datos_CPU.ip),sizeof(uint32_t));
	memcpy(&(mensaje1->nuevoEstado), &(estado), sizeof(estado_t));
	memcpy(&(mensaje1->tiempoBloqueo), &tiempoBloqueo, sizeof(uint32_t));
	memcpy(&(mensaje1->tamMensaje), &tamPayload, sizeof(uint32_t));
	resultado = send(datos_CPU.socket,(void*) mensaje1,(sizeof(mensaje_CPU_PL)),0);
	mensaje2 = malloc(tamPayload);
	memcpy(mensaje2, payload, tamPayload);
	resultado = send(datos_CPU.socket,(void*) mensaje2,tamPayload,0);
	free(mensaje1);
	free(mensaje2);
	return resultado;
}
