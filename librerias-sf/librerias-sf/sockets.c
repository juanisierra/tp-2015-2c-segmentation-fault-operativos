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
	if(mensaje!=NULL)
	{
		memcpy(&(mensaje->pid),&(PCB->info.pid),sizeof(uint32_t));
		memcpy(&(mensaje->ip),&(PCB->info.ip),sizeof(uint32_t));
		memcpy(&(mensaje->path),&(PCB->info.path),51*sizeof(char));
		memcpy(&(mensaje->quantum),&quantum,sizeof(uint32_t));
		printf("Antes de enviar mensaje: %s %d\n",mensaje->path,mensaje->pid);
		resultado = send(socket,(void*) mensaje,sizeof(mensaje_PL_CPU),0);
		free(mensaje);
	}
	else
	{
		resultado=-2;
	}
	return resultado;
}
int recibirPCB(int socket, proceso_CPU* proceso,uint32_t *quantum)
{
	int resultado;
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
//FUNCIONAN BIEN CAMBIADAS
int enviarInstruccionAlADM(int socket, mensaje_CPU_ADM* mensajeAMandar) //el CPU le manda al ADM
{
	int resultado;
	if(mensajeAMandar->texto==NULL) mensajeAMandar->tamTexto=0;

	void*buffer= malloc(sizeof(instruccion_t)+3*sizeof(uint32_t)+(mensajeAMandar->tamTexto));
	memcpy(buffer,&(mensajeAMandar->instruccion),sizeof(instruccion_t));
	memcpy(buffer+sizeof(instruccion_t),&(mensajeAMandar->pid),sizeof(uint32_t));
	memcpy(buffer+sizeof(instruccion_t)+sizeof(uint32_t),&(mensajeAMandar->parametro),sizeof(uint32_t));
	memcpy(buffer+sizeof(instruccion_t)+2*sizeof(uint32_t),&(mensajeAMandar->tamTexto),sizeof(uint32_t));
	memcpy(buffer+sizeof(instruccion_t)+3*sizeof(uint32_t),mensajeAMandar->texto,mensajeAMandar->tamTexto);
	resultado = send(socket,(void*) buffer,(sizeof(instruccion_t)+3*sizeof(uint32_t)+(mensajeAMandar->tamTexto)),0);
	free(buffer);
	return resultado;
}
int recibirInstruccionDeCPU(int socket, mensaje_CPU_ADM* mensajeRecibido) //el ADM recibe el mensaje que le manda el CPU
{
	int resultado;
	void*buffer=malloc(sizeof(instruccion_t)+3*sizeof(uint32_t));
	resultado = recv(socket,buffer,sizeof(instruccion_t)+3*sizeof(uint32_t),0);
	memcpy(&(mensajeRecibido->instruccion),buffer,sizeof(instruccion_t));
	memcpy(&(mensajeRecibido->pid),buffer+sizeof(instruccion_t),sizeof(uint32_t));
	memcpy(&(mensajeRecibido->parametro),buffer+sizeof(instruccion_t)+sizeof(uint32_t),sizeof(uint32_t));
	memcpy(&(mensajeRecibido->tamTexto),buffer+sizeof(instruccion_t)+2*sizeof(uint32_t),sizeof(uint32_t));
	if(mensajeRecibido->tamTexto!=0)
	{	mensajeRecibido->texto=malloc(mensajeRecibido->tamTexto);
		resultado = recv(socket,mensajeRecibido->texto,mensajeRecibido->tamTexto,0);
	} else
	{
		mensajeRecibido->texto=NULL;
	}
	free(buffer);
	return resultado;
}

int enviarInstruccionACPU(int socket, mensaje_ADM_CPU* mensajeAMandar) // El ADM le manda al CPU
{
	int resultado;
	if(mensajeAMandar->texto==NULL) mensajeAMandar->tamanoMensaje=0;
	void*buffer=malloc(2*sizeof(uint32_t)+(mensajeAMandar->tamanoMensaje));
	memcpy(buffer,&(mensajeAMandar->parametro),sizeof(uint32_t));
	memcpy(buffer+sizeof(uint32_t),&(mensajeAMandar->tamanoMensaje),sizeof(uint32_t));
	memcpy(buffer+2*sizeof(uint32_t),mensajeAMandar->texto,mensajeAMandar->tamanoMensaje);
	resultado = send(socket, buffer,(2*sizeof(uint32_t)+mensajeAMandar->tamanoMensaje),0);
	free(buffer);
	return resultado;
}
int recibirInstruccionDeADM(int socket, mensaje_ADM_CPU* mensajeRecibido)// el CPU recibe el mensaje del ADM
{
	int resultado;
	void*buffer=malloc(2*sizeof(uint32_t));
	resultado = recv(socket,buffer,2*sizeof(uint32_t),0);
	memcpy(&(mensajeRecibido->parametro),buffer,sizeof(uint32_t));
	memcpy(&(mensajeRecibido->tamanoMensaje),buffer+sizeof(uint32_t),sizeof(uint32_t));
	if(mensajeRecibido->tamanoMensaje!=0)
	{
		mensajeRecibido->texto=malloc((mensajeRecibido->tamanoMensaje)*sizeof(char));
		resultado = recv(socket,mensajeRecibido->texto,mensajeRecibido->tamanoMensaje,0);
	}
	else
	{
		mensajeRecibido->texto=NULL;
	}
	free(buffer);
	return resultado;
}
//FALTA CAMBIAR A MEMCOPY CON BUFFER
int enviarMensajeAPL(proceso_CPU datos_CPU, estado_t estado, uint32_t tiempoBloqueo, char* payload, uint32_t tamPayload)//enviamos los retornos de instreuccion al PL, cantidadMensajes seria un "tamaño", hay que multiplicarlo por el size
{
	int resultado;
	void* buffer= malloc(3*sizeof(uint32_t)+sizeof(estado_t)+tamPayload);
	memcpy(buffer, &(datos_CPU.ip), sizeof(uint32_t));
	memcpy(buffer+sizeof(uint32_t), &estado, sizeof(estado_t));
	memcpy(buffer+sizeof(uint32_t)+sizeof(estado_t), &tiempoBloqueo, sizeof(uint32_t));
	memcpy(buffer+2*sizeof(uint32_t)+sizeof(estado_t), &tamPayload, sizeof(uint32_t));
	memcpy(buffer+3*sizeof(uint32_t)+sizeof(estado_t), payload, tamPayload);
	resultado = send(datos_CPU.socket, buffer, 3*sizeof(uint32_t)+sizeof(estado_t)+tamPayload, 0 );
	free(buffer);
	return resultado;
}
int recibirPCBDeCPU(int socket, mensaje_CPU_PL *mensaje)
{
	int resultado;
	void*buffer=malloc(3*sizeof(uint32_t)+sizeof(estado_t));
	resultado = recv(socket,buffer,3*sizeof(uint32_t)+ sizeof(estado_t),0);
	memcpy(&(mensaje->ip),buffer,sizeof(uint32_t));
	memcpy(&(mensaje->nuevoEstado),buffer+sizeof(uint32_t),sizeof(estado_t));
	memcpy(&(mensaje->tiempoBloqueo),buffer+sizeof(uint32_t)+sizeof(estado_t),sizeof(uint32_t));
	memcpy(&(mensaje->tamPayload),buffer+2*sizeof(uint32_t)+sizeof(estado_t),sizeof(uint32_t));
	if(mensaje->tamPayload!=0)
	{
		mensaje->payload=malloc(mensaje->tamPayload);
		resultado = recv(socket,mensaje->payload,mensaje->tamPayload,0);
	}
	else
	{
		mensaje->payload=NULL;
	}
	free(buffer);
	return resultado;
}

int enviarDeADMParaSwap(int socket, mensaje_ADM_SWAP* mensajeAEnviar, int tamPagina)//del adm al swap
{
	int resultado;
	void* buffer =NULL;
	int cantidadNulos;
	int i = 0;//variable contadora para llenar con nulos
	void* nulos;
	char* nulo;
	if(mensajeAEnviar->contenidoPagina!=NULL) {
	cantidadNulos = tamPagina - (strlen(mensajeAEnviar->contenidoPagina)+1);
	nulos=malloc(cantidadNulos);
	nulo=malloc(1);
	buffer=malloc(2*sizeof(uint32_t)+ sizeof(instruccion_t)+ tamPagina);
	*nulo = '\0';
	for(i=0; i<cantidadNulos; i++)
	{
		memcpy(nulos+i, nulo, 1);
	}

	} else {
		buffer=malloc(2*sizeof(uint32_t)+ sizeof(instruccion_t));
	}
	memcpy(buffer, &(mensajeAEnviar->instruccion), sizeof(instruccion_t));
	memcpy(buffer+sizeof(instruccion_t), &(mensajeAEnviar->pid), sizeof(uint32_t));
	memcpy(buffer+sizeof(instruccion_t)+sizeof(uint32_t), &(mensajeAEnviar->parametro), sizeof(uint32_t));

	if(mensajeAEnviar->contenidoPagina!=NULL){
	memcpy(buffer+sizeof(instruccion_t)+2*sizeof(uint32_t), mensajeAEnviar->contenidoPagina, strlen(mensajeAEnviar->contenidoPagina)+1);
	memcpy(buffer+sizeof(instruccion_t)+2*sizeof(uint32_t)+strlen(mensajeAEnviar->contenidoPagina)+1, nulos, cantidadNulos);
	resultado = send(socket, buffer, 2*sizeof(uint32_t)+ sizeof(instruccion_t)+ tamPagina, 0 );
		free(nulo);
	free(nulos);
	} else {
		resultado = send(socket, buffer, 2*sizeof(uint32_t)+ sizeof(instruccion_t), 0 );
	}

	free(buffer);
	return resultado;
}
int recibirPaginaDeADM(int socket, mensaje_ADM_SWAP* mensajeARecibir, int tamPagina) //el swap recibe del adm
{
	int resultado;
	void* buffer = malloc(2*sizeof(uint32_t)+ sizeof(instruccion_t));
	resultado = recv(socket,buffer,2*sizeof(uint32_t)+ sizeof(instruccion_t),0);
	memcpy(&(mensajeARecibir->instruccion), buffer, sizeof(instruccion_t));
	memcpy(&(mensajeARecibir->pid),buffer+sizeof(instruccion_t), sizeof(uint32_t) );
	memcpy(&(mensajeARecibir->parametro),buffer+sizeof(instruccion_t)+sizeof(uint32_t), sizeof(uint32_t) );
	if(mensajeARecibir->instruccion==ESCRIBIR)
	{
		mensajeARecibir->contenidoPagina=malloc(tamPagina);
		resultado = recv(socket,mensajeARecibir->contenidoPagina,tamPagina,0);
	}
	else
	{
		mensajeARecibir->contenidoPagina=NULL;
	}
	free(buffer);
	return resultado;
}

int enviarDeSwapAlADM(int socket, mensaje_SWAP_ADM* mensajeAEnviar, int tamPagina)
{
	int resultado;
	void* buffer;
	if(mensajeAEnviar->contenidoPagina!=NULL)
	{
		buffer = malloc(sizeof(uint32_t) + sizeof(instruccion_t)+ tamPagina);
	}
	else
	{
		buffer = malloc(sizeof(uint32_t)+sizeof(instruccion_t));
	}
	memcpy(buffer, &(mensajeAEnviar->estado), sizeof(uint32_t));
	memcpy(buffer+sizeof(uint32_t),&(mensajeAEnviar->instruccion),sizeof(instruccion_t));
	if(mensajeAEnviar->contenidoPagina!=NULL)
	{
		memcpy(buffer+sizeof(uint32_t)+sizeof(instruccion_t), mensajeAEnviar->contenidoPagina, tamPagina); //PRUEBO SIN AMPERSANT
		resultado = send(socket, buffer, sizeof(uint32_t) + sizeof(instruccion_t) + tamPagina, 0 );
	}
	else
	{
		resultado = send(socket, buffer, sizeof(uint32_t)+sizeof(instruccion_t), 0 );
	}
	if(mensajeAEnviar->contenidoPagina!=NULL)
	{
		//
	}
	free(buffer);
	return resultado;
}

int recibirMensajeDeSwap(int socket, mensaje_SWAP_ADM* mensajeRecibido, int tamPagina)// el adm recibe el del swap
{
	int resultado;
	void* buffer = malloc(sizeof(uint32_t)+sizeof(instruccion_t));
	resultado = recv(socket, buffer, sizeof(uint32_t)+sizeof(instruccion_t),0);
	memcpy(&(mensajeRecibido->estado), buffer, sizeof(uint32_t));
	memcpy(&(mensajeRecibido->instruccion),buffer+sizeof(uint32_t),sizeof(instruccion_t));
	if(mensajeRecibido->instruccion==LEER){
		mensajeRecibido->contenidoPagina=malloc(tamPagina);
		resultado = recv(socket, mensajeRecibido->contenidoPagina,tamPagina,0); // BUFFER PAGINA DEBERIA SER CONTENIDOPAGINA
	} else {
		mensajeRecibido->contenidoPagina=NULL;
	}
	free(buffer);
	return resultado;
}

