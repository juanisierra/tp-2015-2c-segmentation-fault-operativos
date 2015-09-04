#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define TAMANOPAQUETE 4
#define IPADM "127.0.0.1"
#define IPPL "127.0.0.1"
#define PUERTOPL "6575"
#define PUERTOADM "6576"

int main(void)
{	char mensaje[50];
	int socketPL;
	int socketADM;
	printf("Iniciando CPU.. \n");
	if((socketPL = crearSocketCliente(IPPL,PUERTOPL))<0)
	{
		printf("No se pudo crear socket \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
		return 0;
	}
	if((socketADM = crearSocketCliente(IPADM,PUERTOADM))<0)
		{
			printf("No se pudo crear socket \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			close(socketPL);
			return 0;
		}
	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Conectado al Planificador en el puerto %s \n",PUERTOPL);

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
