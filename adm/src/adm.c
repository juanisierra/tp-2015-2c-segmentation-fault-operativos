#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define TAMANOCONSOLA 1024
#define TAMANOPAQUETE 4
#define PUERTOSERVER "6576"
#define PUERTOSWAP	"6577"
#define IP "127.0.0.1"

int main()
{	char mensaje[3];
	printf("Administrador de Memria \nEstableciendo conexion.. \n");
	int socketSWAP;
	int socketEscucha;
	socketEscucha= crearSocketEscucha(10,PUERTOSERVER);
	if(socketEscucha < 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",PUERTOSERVER);
		return -1;
	}

	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",PUERTOSERVER);
		return -1;
	}
	printf("Creando Socket de conexion al SWAP en puerto %s \n",PUERTOSWAP);

	if((socketSWAP = crearSocketCliente(IP,PUERTOSWAP))<0)
		{
			printf("No se pudo crear socket de conexion al SWAP \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}






	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones.. \n");
	int socketCPU = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);

	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Conectado al CPU en el puerto %s \n",PUERTOSERVER);

	while (status != 0)
	{
		status = recv(socketCPU, (void*) mensaje, TAMANOPAQUETE, 0);
		if(mensaje[0]!=2)
		{
			printf("El mensaje recibido por el socket del CPU no pertenece al mismo \n");
			close(socketSWAP);
			close(socketCPU);
			close(socketEscucha);
			return -1;


		}
		if(mensaje[1]!=3)
				{
					printf("El mensaje recibido por el socket del CPU no tiene como destino el Administrador de memoria \n");
					close(socketSWAP);
					close(socketCPU);
					close(socketEscucha);
					return -1;
				}
		if(mensaje[2]==1)
		{
			printf("Mensaje Recibido\n");
			mensaje[0]=3;
			mensaje[1]=4;

			send(socketSWAP, mensaje, strlen(mensaje)+1, 0);
			close(socketSWAP);
			close(socketCPU);
			close(socketEscucha);
			return 0;
		}
	}
	close(socketSWAP);
	close(socketCPU);
	close(socketEscucha);
	return 0;
}
