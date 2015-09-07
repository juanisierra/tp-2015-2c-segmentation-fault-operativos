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

int crearSocketEscucha (int cantidadConexiones, char puerto[]);
int crearSocketCliente (char IP[], char PUERTO[]);

#endif /* LIBRERIAS_SF_SOCKETS_H_ */
