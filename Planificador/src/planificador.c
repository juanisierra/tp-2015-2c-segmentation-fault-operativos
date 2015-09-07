#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <librerias-sf/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define TAMANOCONSOLA 1024
#define RUTACONFIG "configuracion"

int main()
{
	char mensaje[3];
	char ingresado[TAMANOCONSOLA];
	config_pl configuracion;
	configuracion= cargarConfiguracionPL(RUTACONFIG);
	if(configuracion.estado==0 || configuracion.estado==-1){
		printf("Error de configuracion, cerrando proceso..");
		return -1;
	}
	if (configuracion.estado==1){
			printf("Configuracion cargada correctamente: \n");
			printf("Puerto Escucha: %s\n",configuracion.PUERTO_ESCUCHA);
			printf("Codigo del Algoritmo de Planificacion: %d\n",configuracion.ALGORITMO_PLANIFICACION);
			printf("Quantum: %d\n\n",configuracion.QUANTUM);

		}
	printf("Bienvenido al proceso planificador \nEstableciendo conexion.. \n");

	int socketEscucha;
	socketEscucha= crearSocketEscucha(10,configuracion.PUERTO_ESCUCHA);
	if(socketEscucha < 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Planificador \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}

	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Planificador \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones en puerto %s \n",configuracion.PUERTO_ESCUCHA);
	int socketCPU = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);

	int estado_consola = 1;

	printf("Conectado al CPU, ya puede enviar mensajes. Escriba 'salir' para salir\n");

	while(estado_consola)
	{
		fgets(ingresado, TAMANOCONSOLA, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (strcmp(ingresado,"salir\n")==0) estado_consola = 0;			// Chequeo que el usuario no quiera salir
		if(strcmp(ingresado,"correr programa\n")==0)
		{
			mensaje[0]=1;
			mensaje[1]=2;
			mensaje[2]=1;
			if (estado_consola) send(socketCPU, mensaje, strlen(mensaje)+1, 0);// Solo envio si el usuario no quiere salir.
		}

	}
	close(socketCPU);
	close(socketEscucha);
	return 0;
}
