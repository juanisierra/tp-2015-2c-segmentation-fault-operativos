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
{
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

	printf("Creando Socket de conexion al SWAP en puerto %s \n",configuracion.PUERTO_SWAP);

	if((socketSWAP = crearSocketCliente(configuracion.IP_SWAP,configuracion.PUERTO_SWAP))<0)
		{
			printf("No se pudo crear socket de conexion al SWAP \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}






	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones.. \n");
	int socketCPU = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	printf("Conectado al CPU en el puerto %s \n",configuracion.PUERTO_ESCUCHA);
	mensaje_CPU_ADM mensajeARecibir;
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
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
	if(mensajeARecibir.tamTexto!=0) printf("Mensaje: %s\n",mensajeARecibir.texto);
	if(mensajeARecibir.instruccion == INICIAR)
	{printf("RECIBI INICIAR\n");
	mensajeParaSWAP.pid=mensajeARecibir.pid;
		mensajeParaSWAP.instruccion=INICIAR;
		mensajeParaSWAP.parametro=mensajeARecibir.parametro;
		mensajeParaSWAP.contenidoPagina=NULL;
		printf("Mensaje a mandar %d %d\n",mensajeParaSWAP.instruccion,mensajeParaSWAP.parametro);
		enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
		printf("mande INICIAR A SWAP %d %d %d\n",mensajeParaSWAP.pid,mensajeParaSWAP.instruccion,mensajeParaSWAP.parametro);
		recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
		printf("RECIBI DE SWAP %d %d\n",mensajeDeSWAP.estado,mensajeDeSWAP.instruccion);
		mensajeAMandar.parametro = mensajeDeSWAP.estado;
		mensajeAMandar.tamanoMensaje = 0;
		mensajeAMandar.texto = NULL;
		enviarInstruccionACPU(socketCPU, &mensajeAMandar);

	}
	if(mensajeARecibir.instruccion == LEER)
	{printf("RECIBI LEER\n");
	mensajeParaSWAP.pid=mensajeARecibir.pid;
		mensajeParaSWAP.instruccion=LEER;
		mensajeParaSWAP.parametro=mensajeARecibir.parametro;
		mensajeParaSWAP.contenidoPagina=NULL;
		enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
		recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
		mensajeAMandar.parametro=mensajeDeSWAP.estado;
		mensajeAMandar.tamanoMensaje = strlen(mensajeDeSWAP.contenidoPagina) +1;
		mensajeAMandar.texto=malloc(mensajeAMandar.tamanoMensaje);
		strcpy(mensajeAMandar.texto,mensajeDeSWAP.contenidoPagina); /// NO SIRVE CON LAS A PORUQE NO LLEVAN /0
		enviarInstruccionACPU(socketCPU, &mensajeAMandar);
		free(mensajeDeSWAP.contenidoPagina);
		mensajeDeSWAP.contenidoPagina=NULL;
		free(mensajeAMandar.texto);
		mensajeAMandar.texto=NULL;
	}
	if(mensajeARecibir.instruccion == ESCRIBIR)
	{printf("RECIBI ESCRIBIR\n");
	mensajeParaSWAP.pid=mensajeARecibir.pid;
	mensajeParaSWAP.instruccion=ESCRIBIR;
		mensajeParaSWAP.parametro=mensajeARecibir.parametro;
		mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
		strcpy(mensajeParaSWAP.contenidoPagina,mensajeARecibir.texto);
		enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
		recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
		mensajeAMandar.parametro = mensajeDeSWAP.estado;
		mensajeAMandar.tamanoMensaje =0;
		mensajeAMandar.texto =NULL;
		enviarInstruccionACPU(socketCPU, &mensajeAMandar);
		free(mensajeParaSWAP.contenidoPagina);
	}
	if(mensajeARecibir.instruccion ==FINALIZAR)
	{printf("RECIBI FINALIZAR\n");
	mensajeParaSWAP.pid=mensajeARecibir.pid;
	mensajeParaSWAP.instruccion=FINALIZAR;
	mensajeParaSWAP.parametro=mensajeARecibir.parametro;
	mensajeParaSWAP.contenidoPagina=NULL;
	enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
	recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
	mensajeAMandar.parametro = mensajeDeSWAP.estado;
	mensajeAMandar.tamanoMensaje = 0;
	mensajeAMandar.texto = NULL;
	enviarInstruccionACPU(socketCPU, &mensajeAMandar);
	}
	if(mensajeARecibir.tamTexto!=0) free(mensajeARecibir.texto);
	}

	close(socketCPU);
	close(socketEscucha);
	return 0;
}
