#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>
#include <librerias-sf/config.h>
#include <pthread.h>
#include <librerias-sf/strings.h>
#include <librerias-sf/tiposDato.h>
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracion"
#define TAMANIOMAXIMOTEXTO 200
#define TAMANIOMAXIMOLINEA 200

//Variables globales
int socketADM;
int retardoEjecucion;


int iniciarConfiguracion(config_CPU* configuracion)
{
	printf("Iniciando CPU.. \n");
	printf("Cargando configuracion.. \n");
	(*configuracion) =  cargarConfiguracionCPU(RUTACONFIG);
	if (configuracion->estado!=1)
	{
		printf("Error en el archivo de configuracion, cerrando CPU.. \n");
		return -1;
	}
	if (configuracion->estado==1)
	{
		printf("Configuracion cargada correctamente: \n");
		printf("Puerto del Planificador: %s\n",configuracion->PUERTO_PLANIFICADOR);
		printf("IP del Planificador: %s\n",configuracion->IP_PLANIFICADOR);
		printf("IP del ADM: %s\n",configuracion->IP_MEMORIA);
		printf("Puerto del ADM: %s\n",configuracion->PUERTO_MEMORIA);
		printf("Cantidad de Hilos: %d\n",configuracion->CANTIDAD_HILOS);
		printf("Tiempo de Retardo: %d\n\n",configuracion->RETARDO);
		return 0;
	}
	return -1;
}

void ejecutarInstruccion(proceso_CPU* datos_CPU, instruccion_t instruccion, int parametro1, char* parametro2, int* entrada_salida, int* finArchivo)//funcion que ejecuta las instrucciones
{
	switch(instruccion)
	{
	case INICIAR:
	{
		break;
	}
	case LEER:
	{
		break;
	}
	case ESCRIBIR:
	{
		break;
	}
	case ES:
	{
		(*entrada_salida) = 1;
		break;
	}
	case ERROR:
	{
		printf("Escribiste mal algo.asd.asd");
		break;
	}
	case FINALIZAR:
	{
		(*finArchivo) = 1;
		break;
	}
	}
}

void hiloCPU(void* datoCPUACastear)
{
	int status;
	int quantum;
	int entrada_salida; // informa si llego un mensaje de entrada salida.
	int finArchivo; // Informa si se llego al fin de archivo
	char lineaAEjecutar[TAMANIOMAXIMOLINEA]; // Aca se va  a ir guardando las diferentes instrucciones a ejecutar.
	instruccion_t instruccion;
	int parametro1;
	char parametro2[TAMANIOMAXIMOTEXTO];
	FILE* mCod;
	proceso_CPU datos_CPU = *((proceso_CPU*) datoCPUACastear);
	datos_CPU.listaRetornos->ant = NULL;
	datos_CPU.listaRetornos->sgte = NULL;
	while(status != 0)
	{
		entrada_salida = 0;//REINICIAMOS ESTOS VALORES PARA CADA VEZ QUE LEA UNA NUEVA PCB
		finArchivo = 0;
		status = recibirPCB(datos_CPU.socket, &datos_CPU, &quantum);
		printf("PCB RECIBIDO:\n PID: %d \n PATH: %s \n\n",datos_CPU.pid,datos_CPU.path);
		while (quantum != 0 && entrada_salida == 0 && finArchivo == 0)
		{
			mCod = fopen(datos_CPU.path, "r");
			sleep(retardoEjecucion); // simulamos el retardo en ejecutar
			leerlinea(mCod,lineaAEjecutar, datos_CPU.ip);
			datos_CPU.ip++;
			quantum--; // RESTAMOS EL QUANTUM DESPUES DE LEER UNA LINEA
			instruccion = interpretarMcod(lineaAEjecutar,&parametro1,parametro2);
			ejecutarInstruccion(&datos_CPU, instruccion, parametro1, parametro2, &entrada_salida, &finArchivo);
		}
	}
}

int main(void)
{
	int quantum;
	proceso_CPU proceso;
	config_CPU configuracion;
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	retardoEjecucion = configuracion.RETARDO;
	proceso_CPU CPUs[configuracion.CANTIDAD_HILOS];  //Declaramos el array donde cada componente es un hilo

	if((socketADM = crearSocketCliente(configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA))<0) // Se inicializa el socketADM GLOBAL
		{
			printf("No se pudo crear socket en %s:%s \n",configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}

	for(int i = 1;i <= configuracion.CANTIDAD_HILOS; i++) // Vamos creando un hilo por cada CPU.
	{
		CPUs[i].id = i;
		if((CPUs[i].socket = crearSocketCliente(configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR))<0) // ponemos el socket de cada CPU
		{
			printf("No se pudo crear socket planificador en %s:%s \n",configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}
		pthread_create(&CPUs[i].thread, NULL, (void*) hiloCPU, (void*) &(CPUs[i]));
		i++;
	}

	return 0;
}

