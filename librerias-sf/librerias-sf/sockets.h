/*
 * sockets.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_SOCKETS_H_
#define LIBRERIAS_SF_SOCKETS_H_
int crearSocketEscucha (int cantidadConexiones, char puerto[]);
int crearSocketCliente (char IP[], char PUERTO[]);

#endif /* LIBRERIAS_SF_SOCKETS_H_ */
