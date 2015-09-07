#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <librerias-sf/config.h>
#define TAMANOPAQUETE 4
#define IPADM "127.0.0.1"
#define IPPL "127.0.0.1"
#define PUERTOPL "6575"
#define PUERTOADM "6576"
#define RUTACONFIG "configuracion"
int main(void)
{	char mensaje[50];
	int socketPL;
	int socketADM;
	config_CPU configuracion;

	printf("Iniciando CPU.. \n");
	printf("Cargando configuracion.. \n");
	configuracion =  cargarConfiguracionCPU(RUTACONFIG);
	if (configuracion.estado!=1){
		printf("Error en el archivo de configuracion, cerrando CPU.. \n");
		return -1;
	}
	if (configuracion.estado==1){
		printf("Configuracion cargada correctamente: \n");
		printf("Puerto del Planificador: %s\n",configuracion.PUERTO_PLANIFICADOR);
		printf("IP del Planificador: %s\n",configuracion.IP_PLANIFICADOR);
		printf("IP del ADM: %s\n",configuracion.IP_MEMORIA);
		printf("Puerto del ADM: %s\n",configuracion.PUERTO_MEMORIA);
		printf("Cantidad de Hilos: %d\n",configuracion.CANTIDAD_HILOS);
		printf("Tiempo de Retardo: %d\n\n",configuracion.RETARDO);
	}
	if((socketPL = crearSocketCliente(configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR))<0)
	{
		printf("No se pudo crear socket en %s:%s \n",configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
		return 0;
	}
	if((socketADM = crearSocketCliente(configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA))<0)
		{
			printf("No se pudo crear socket en %s:%s \n",configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			close(socketPL);
			return 0;
		}
	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Conectado al Planificador en %s:%s \n",configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR);

	while (status != 0)
	{
		status = recv(socketPL, (void*) mensaje, TAMANOPAQUETE, 0);
		if(mensaje[0]!=1)
		{
			printf("El mensaje recibido por el socket del planificador no pertenece al mismo \n");
			close(socketADM);
			close(socketPL);
			return -1;
		}
		if(mensaje[1]!=2)
				{
					printf("El mensaje recibido por el socket del planificador no tiene como destino una CPU \n");
					close(socketADM);
					close(socketPL);
					return -1;
				}
		if(mensaje[2]==1)
		{
			printf("Mensaje Recibido\n");
			mensaje[0]=2;
			mensaje[1]=3;
			send(socketADM, mensaje, strlen(mensaje)+1, 0);
			close(socketADM);
			close(socketPL);
			return 0;
		}
	}
	close(socketADM);
	close(socketPL);
	return 0;
}
