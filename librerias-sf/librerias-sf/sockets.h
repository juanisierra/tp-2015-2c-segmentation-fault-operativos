/*
 * sockets.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_SOCKETS_H_
#define LIBRERIAS_SF_SOCKETS_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "tiposDato.h"
int crearSocketEscucha (int cantidadConexiones, char puerto[]);
int crearSocketCliente (char IP[], char PUERTO[]);
int enviarPCB(int socket,pcb PCB, uint32_t quantum);
int recibirPCB(int socket, proceso_CPU* proceso,int *quantum);
#endif /* LIBRERIAS_SF_SOCKETS_H_ */
