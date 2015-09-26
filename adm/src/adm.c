#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>
#include <librerias-sf/config.h>
#include <librerias-sf/config.c>
#include <pthread.h>
#include <librerias-sf/strings.h>
#include <librerias-sf/tiposDato.h>
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracion"
#define TAMANIOMAXIMOTEXTO 200
#define TAMANIOMAXIMOLINEA 200

int iniciarConfiguracion(config_ADM* configuracion)
{
	printf("Cargando Configuracion..\n");
	(*configuracion) = cargarConfiguracionADM(RUTACONFIG);
	if(configuracion->estado==-1 || configuracion->estado==0){
		printf("Cerrando ADM..\n");
		return -1;
	}
	if (configuracion->estado==1){
			printf("Configuracion cargada correctamente: \n");
			printf("Puerto Escucha: %s\n",configuracion->PUERTO_ESCUCHA);
			printf("IP del SWAP: %s\n",configuracion->IP_SWAP);
			printf("Puerto del SWAP: %s\n",configuracion->PUERTO_SWAP);
			printf("Maximo de Marcos por Proceso: %d\n",configuracion->MAXIMO_MARCOS_POR_PROCESO);
			printf("Cantidad de Marcos: %d\n",configuracion->CANTIDAD_MARCOS);
			printf("Tamanio Marco: %d\n",configuracion->TAMANIO_MARCO);
			printf("Entradas TLB: %d\n",configuracion->ENTRADAS_TLB);
			printf("TLB Habilitada: %d\n",configuracion->TLB_HABILITADA);
			printf("Retardo Memoria: %d\n\n",configuracion->RETARDO_MEMORIA);
			return 0;
		}
	return -1;
}
#define RUTACONFIG "configuracion"
int main()
{	char mensaje[3];
	config_ADM configuracion;
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	printf("Administrador de Memoria \nEstableciendo conexion.. \n");
	int socketSWAP;
	int socketEscucha;
	socketEscucha= crearSocketEscucha(10,configuracion.PUERTO_ESCUCHA);
	if(socketEscucha < 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}

	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}
	/*
	printf("Creando Socket de conexion al SWAP en puerto %s \n",configuracion.PUERTO_SWAP);

	if((socketSWAP = crearSocketCliente(configuracion.IP_SWAP,configuracion.PUERTO_SWAP))<0)
		{
			printf("No se pudo crear socket de conexion al SWAP \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}*/






	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones.. \n");
	int socketCPU = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	printf("Conectado al CPU en el puerto %s \n",configuracion.PUERTO_ESCUCHA);
	mensaje_CPU_ADM mensajeARecibir;
	mensaje_ADM_CPU mensajeAMandar;//es el mensaje que le mandaremos al CPU
	int status = 1;		// Estructura que manjea el status de los recieve.
	while(1)
	{
	printf("esperando que el CPU mande \n");
	status = recibirInstruccionDeCPU(socketCPU, &mensajeARecibir);
	printf("%d \n", mensajeARecibir.instruccion);
	printf("%d \n", mensajeARecibir.parametro);
	printf("%d \n", mensajeARecibir.pid);
	printf("%d \n", mensajeARecibir.tamTexto);
	printf("%s \n \n \n", mensajeARecibir.texto);
	if(mensajeARecibir.instruccion == INICIAR)
	{
		mensajeAMandar.parametro = mensajeARecibir.parametro;
		mensajeAMandar.tamanoMensaje = strlen("mProx X - Iniciado") +1;
		mensajeAMandar.texto = strdup("mProx X - Iniciado");
		enviarRetornoInstruccion(socketCPU, &mensajeAMandar);
		free(mensajeAMandar.texto);
	}
	if(mensajeARecibir.instruccion == LEER)
	{
		mensajeAMandar.parametro = mensajeARecibir.parametro;
		mensajeAMandar.tamanoMensaje = strlen("mProc X - Pagina N leida:") +1;
		mensajeAMandar.texto = strdup("mProc X - Pagina N leida:");
		enviarRetornoInstruccion(socketCPU, &mensajeAMandar);
		free(mensajeAMandar.texto);
	}
	if(mensajeARecibir.instruccion == ESCRIBIR)
	{
		mensajeAMandar.parametro = mensajeARecibir.parametro;
		mensajeAMandar.tamanoMensaje = strlen("mProc X - Pagina N escrita:") +1;
		mensajeAMandar.texto = strdup(mensajeARecibir.texto);
		enviarRetornoInstruccion(socketCPU, &mensajeAMandar);
		free(mensajeAMandar.texto);
	}
	free(mensajeARecibir.texto);
	}
/*
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
	*/
	close(socketCPU);
	close(socketEscucha);
	return 0;
}
