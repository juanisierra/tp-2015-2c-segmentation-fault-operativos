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

//Variables globales
int socketADM;
config_CPU configuracion;

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

void ejecutarInstruccion(proceso_CPU* datos_CPU, instruccion_t instruccion, uint32_t parametro1, char* parametro2, uint32_t* entrada_salida, int* finArchivo, estado_t* estado)//funcion que ejecuta las instrucciones
{
	mensaje_ADM_CPU mensajeDeADM;
	mensaje_CPU_ADM mensajeParaADM;
	mensajeParaADM.instruccion = instruccion;
	mensajeParaADM.pid = datos_CPU->pid;
	mensajeParaADM.parametro = parametro1;
	mensajeParaADM.tamTexto = strlen(parametro2) +1;
	switch(instruccion)
	{
	case INICIAR:
	{	printf("Instruccion iniciar param: %d\n",parametro1);//***********************
		mensajeParaADM.texto=NULL;
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);
		printf("Inst enviada\n");
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		break;
	}
	case LEER:
	{	printf("Instruccion leer parametro: %d\n",parametro1);//*********************
		mensajeParaADM.texto=NULL;
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		printf("Recibi: %s\n ",mensajeDeADM.texto);
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		free(mensajeDeADM.texto);
		break;

	}
	case ESCRIBIR:
	{	printf("Instruccion escribir pag: %d  escribe: %s\n",parametro1,parametro2);//********************

		mensajeParaADM.texto = strdup(parametro2); //duplicamos la cadena en el heap
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		printf("Recibi: %s\n",mensajeDeADM.texto);
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		free(mensajeParaADM.texto);
		free(mensajeDeADM.texto);
		break;
	}
	case ES:
	{	printf("instruccion es tiempo: %d\n",parametro1);
		// ASIGNAR MENSAJE
		(*estado) = BLOQUEADO;
		mensajeDeADM.parametro = parametro1;
		mensajeDeADM.tamanoMensaje = 0;
		mensajeDeADM.texto = NULL;
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		(*entrada_salida) = 1; // marcara el tiempo de bloqueo
		break;
	}

	case FINALIZAR:
	{
		printf("Instruccion finalizar\n");
		mensajeParaADM.texto=NULL;
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);;
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		(*finArchivo) = 1;
		break;
	}
	case ERROR:
	{
		printf("Error: instruccion mal escrita\n");

		(*estado) = INVALIDO;
		(*finArchivo) = 1;
		break;
	}
	case CERRAR:
	{
		break;
	}
	}
}

void hiloCPU(void* datoCPUACastear)
{
	uint32_t quantum;
	uint32_t tamPayload;//Aqui se guardara el tamaño de lo que se le manda al PL
	estado_t estado; // es el estado de ejecucion
	char* mensajeParaPL; //aca se guardara toda la secuencia de datos a mandar al PL
	int status;
	uint32_t entrada_salida; // informa si llego un mensaje de entrada salida.
	int finArchivo; // Informa si se llego al fin de archivo
	char lineaAEjecutar[TAMANIOMAXIMOLINEA]; // Aca se va  a ir guardando las diferentes instrucciones a ejecutar.
	instruccion_t instruccion;
	uint32_t parametro1;
	char parametro2[TAMANIOMAXIMOTEXTO];
	FILE* mCod;
	proceso_CPU datos_CPU = *((proceso_CPU*) datoCPUACastear);
	status=1;
	while(status != 0)
	{	status = recibirPCB(datos_CPU.socket, &datos_CPU, &quantum);
		entrada_salida = 0;//REINICIAMOS ESTOS VALORES PARA CADA VEZ QUE LEA UNA NUEVA PCB
		finArchivo = 0; //Indica el final del mcod
		datos_CPU.listaRetornos = NULL; // ACA HAGO QUE LA LISTA REINICIE AL LEER UN MCOD COMPLETO,la lista es liberada en la funcion desempaquetar
		printf("CPU numero: %d \n", datos_CPU.id);
		printf("PCB RECIBIDO:\n PID: %d \n PATH: %s \n\n",datos_CPU.pid,datos_CPU.path);
		mCod = fopen(datos_CPU.path, "r");
		if(mCod==NULL)
			{
			printf("Archivo Erroneo\n");//AGREGAR CONTROL DEARCHIVO ERRONEO
			return;
			}
		while (quantum != 0 && entrada_salida == 0 && finArchivo == 0) //esta es la condicion para que deje de ejecutar
		{
			parametro2[0] = '\0';// ponemos el nulo para cada vez que lea un nuevo parametro, lo reiniciamos
			sleep(configuracion.RETARDO); // simulamos el retardo en ejecutar
			leerLinea(mCod,lineaAEjecutar, datos_CPU.ip);
			datos_CPU.ip++;
			quantum--; // RESTAMOS EL QUANTUM DESPUES DE LEER UNA LINEA
			instruccion = interpretarMcod(lineaAEjecutar,&parametro1,parametro2);
			printf("Ins: %d\n Param1: %d\n Param2: %s\n",instruccion,parametro1,parametro2);
			ejecutarInstruccion(&datos_CPU, instruccion, parametro1, parametro2, &entrada_salida, &finArchivo, &estado);
			//printf("%s \n",datos_CPU.listaRetornos->info.texto);
		}
		fclose(mCod);
		printf("La primer instruccion de la lista es: %d \n", datos_CPU.listaRetornos->info.instruccion);
		printf("El primer mensaje de la lista es: %s \n", datos_CPU.listaRetornos->info.texto);
		printf("tamaño de lo primero: %d \n", datos_CPU.listaRetornos->info.tamTexto);
		printf("La segunda instruccion de la lista es: %d \n", datos_CPU.listaRetornos->sgte->info.instruccion);
		printf("El segundo mensaje de la lista es: %s \n", datos_CPU.listaRetornos->sgte->info.texto);
		printf("tamaño de lo segundo: %d \n", datos_CPU.listaRetornos->info.tamTexto);
		tamPayload = desempaquetarLista(&mensajeParaPL, datos_CPU.listaRetornos);//pasa la lista a un array de datos que es mensajeParaPL
		printf("Todas las instrucciones a devolver son: \n%s", mensajeParaPL); //no se va a mostrar todo porque corta en el \0, muestra 1
		enviarMensajeAPL(datos_CPU,estado, entrada_salida, mensajeParaPL,tamPayload);
		free(mensajeParaPL);
		printf("Termino rafaga\n");
	}
	printf("Terminando Hilo de CPU\n");
	close(datos_CPU.socket);
	printf("Cerrando socket\n");
	return;
}

int main(void)
{
	int i;
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	proceso_CPU CPUs[configuracion.CANTIDAD_HILOS];  //Declaramos el array donde cada componente es un hilo

	if((socketADM = crearSocketCliente(configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA))<0) // Se inicializa el socketADM GLOBAL
		{
			printf("No se pudo crear socket en %s:%s \n",configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}

	for(i = 0;i < configuracion.CANTIDAD_HILOS; i++) // Vamos creando un hilo por cada CPU.
	{
		CPUs[i].id = i;
		if((CPUs[i].socket = crearSocketCliente(configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR))<0) // ponemos el socket de cada CPU
		{
			printf("No se pudo crear socket planificador en %s:%s \n",configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}
		pthread_create(&CPUs[i].thread, NULL, (void*)hiloCPU, (void*) &(CPUs[i]));
		i++;
	}
	for(i = 0;i < configuracion.CANTIDAD_HILOS; i++)// hacemos los join de cada cpu para que no corte antes.
	{
		pthread_join(CPUs[i].thread, NULL);
		i++;
	}
	close(socketADM);
	return 0;
}

