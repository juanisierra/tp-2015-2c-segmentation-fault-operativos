#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define TAMANOCONSOLA 1024
#define TAMANOPAQUETE 4
#define PUERTO "6577"


int main()
{	char mensaje[3];
	printf("SWAP \nEstableciendo conexion.. \n");

	int socketEscucha;
	socketEscucha= crearSocketEscucha(10,PUERTO);
	if(socketEscucha < 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP \n",PUERTO);
		return -1;
	}

	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP \n",PUERTO);
		return -1;
	}
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones.. \n");
	int socketADM = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);

	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Conectado al ADM en el puerto %s \n",PUERTO);

	while (status != 0)
	{
		status = recv(socketADM, (void*) mensaje, TAMANOPAQUETE, 0);
		if(mensaje[0]!=3)
		{
			printf("El mensaje recibido por el socket del ADM no pertenece al mismo \n");
			close(socketADM);
			close(socketEscucha);
			return -1;


		}
		if(mensaje[1]!=4)
				{
					printf("El mensaje recibido por el socket del ADM no tiene como destino el Administrador de SWAP \n");

					close(socketADM);
					close(socketEscucha);
					return -1;
				}
		if(mensaje[2]==1)
		{
			printf("Mensaje Recibido\n");
			close(socketADM);
			close(socketEscucha);
			return 0;
		}
	}
	close(socketADM);
	close(socketEscucha);
	return 0;
}
