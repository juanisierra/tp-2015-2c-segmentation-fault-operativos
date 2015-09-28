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
int enviarPCB(int socket,nodoPCB* PCB, uint32_t quantum);
int recibirPCB(int socket, proceso_CPU* proceso,uint32_t *quantum);
int enviarInstruccionAlADM(int socket, mensaje_CPU_ADM* mensajeAMandar); //el CPU le manda al ADM
int recibirInstruccionDeCPU(int socket, mensaje_CPU_ADM* mensajeRecibido); //el ADM recibe el mensaje que le manda el CPU
int enviarInstruccionACPU(int socket, mensaje_ADM_CPU* mensajeAMandar); // El ADM le manda al CPU
int recibirInstruccionDeADM(int socket, mensaje_ADM_CPU* mensajeRecibido);// el CPU recibe el mensaje del ADM
int enviarMensajeAPL(proceso_CPU datos_CPU, estado_t estado, uint32_t tiempoBloqueo, retornoInstruccion* payload, uint32_t cantidadMensajes);
//enviamos los retornos de instreuccion al PL, cantidadMensajes seria un "tamaño", hay que multiplicarlo por el size
int recibirPCBDeCPU(int socket, mensaje_CPU_PL *mensaje);
#endif /* LIBRERIAS_SF_SOCKETS_H_ */
