#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define TAMANOPAQUETE 4
#define IP "127.0.0.1"
#define PUERTO "6575"


int main(void)
{	char mensaje[50];
	int socketPL;
	printf("Iniciando CPU.. \n");
	if((socketPL = crearSocketCliente(IP,PUERTO))<0)
	{
		printf("No se pudo crear socket \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
		return 0;
	}
	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Conectado al Planificador en el puerto %s \n",PUERTO);

	while (status != 0)
	{
		status = recv(socketPL, (void*) mensaje, TAMANOPAQUETE, 0);
		if(mensaje[0]!=1)
		{
			printf("El mensaje recibido por el socket del planificador no pertenece al mismo \n");
			return -1;
		}
		if(mensaje[1]!=2)
				{
					printf("El mensaje recibido por el socket del planificador no tiene como destino una CPU \n");
					return -1;
				}
		if(mensaje[2]==1)
		{
			printf("Mensaje Recibido\n");
			close(socketPL);
			return 0;
		}
	}
	return 0;
}
