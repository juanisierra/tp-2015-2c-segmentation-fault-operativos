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
#include <commons/log.h>
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracionCPU"
#define TAMANIOMAXIMOTEXTO 200
#define TAMANIOMAXIMOLINEA 200
#define ARCHIVOLOG "CPU.log"

typedef struct contadorInstrucciones{//Creamos el tipo de dato encargado de almacenar la cantidad de instrucciones y el socket para mandarselo al PL
	uint32_t contador;
	int socket;
}contadorInstrucciones;
//Variables globales
int socketADM;
config_CPU configuracion;
pthread_mutex_t mutexComADM; //mutex para comunicacion con el ADM
pthread_mutex_t mutexLog;
pthread_mutex_t instruccionesEjec;
t_log* log;
contadorInstrucciones* instruccionesEjecutadas; // va a contar las instrucciones ejecutadas por cada CPU
pthread_t hCalculo;

int iniciarConfiguracion(config_CPU* configuracion)
{
	printf("Iniciando CPU.. \n");
	//printf("Cargando configuracion.. \n");
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
	{
		//printf("Instruccion iniciar\tparametro:%d\n",parametro1);//***********************
		mensajeParaADM.texto=NULL;
		pthread_mutex_lock(&mutexComADM);
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		pthread_mutex_unlock(&mutexComADM);
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		if(mensajeDeADM.parametro == 1)// significa que fallo
		{
			(*finArchivo) = 1; //no termina el archivo pero se pone para que deja la ejecucion si es que falla la iniciazion de memoria
			(*estado) = ERRORINICIO;
		}
		else
		{
			(*estado)=LISTO;
		}
		break;
	}
	case LEER:
	{	//printf("Instruccion leer\tparametro:%d ",parametro1);//*********************
		mensajeParaADM.texto=NULL;
		pthread_mutex_lock(&mutexComADM);
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		pthread_mutex_unlock(&mutexComADM);
		if(mensajeDeADM.parametro == -1)
		{
			(*estado) = ERRORMARCO;
			(*finArchivo) = 1;
		}
		else
		{
			(*estado)=LISTO;
		}
		//printf("Recibi: %s\n",mensajeDeADM.texto);
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		free(mensajeDeADM.texto);
		break;

	}
	case ESCRIBIR:
	{
		//printf("Instruccion escribir\tpag: %d\tescribe: %s\n",parametro1,parametro2);//********************
		mensajeParaADM.texto = strdup(parametro2); //duplicamos la cadena en el heap
		pthread_mutex_lock(&mutexComADM);
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		pthread_mutex_unlock(&mutexComADM);
		//////////////////////////////////////////////////////
		if(mensajeDeADM.parametro == -1)
		{
			(*estado) = ERRORMARCO;
			(*finArchivo) = 1;
		}
		else
		{
			(*estado)=LISTO;
		}
		mensajeDeADM.texto=strdup(parametro2); // AGREGO ESTO PARA QUE EL TEXTO QUE SE ESCRIBE SEA E DE PARAMETRO 2 porque no retorna lo escrito.
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		free(mensajeParaADM.texto);
		free(mensajeDeADM.texto);
		break;
	}
	case ES:
	{	//printf("instruccion es tiempo: %d\n",parametro1);
		// ASIGNAR MENSAJE
		(*estado) = BLOQUEADO;
		mensajeDeADM.parametro = parametro1;
		mensajeDeADM.tamanoMensaje = 0;
		mensajeDeADM.texto = NULL;
		almacenarEnListaRetornos(mensajeDeADM, datos_CPU, instruccion);
		(*entrada_salida) = parametro1; // marcara el tiempo de bloqueo
		break;
	}

	case FINALIZAR:
	{
		(*estado) = AFINALIZAR;
		//printf("Instruccion finalizar\n");
		mensajeParaADM.texto=NULL;
		pthread_mutex_lock(&mutexComADM);
		enviarInstruccionAlADM(socketADM, &mensajeParaADM);
		recibirInstruccionDeADM(socketADM, &mensajeDeADM);
		pthread_mutex_unlock(&mutexComADM);
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
	uint32_t instruccionesEjecutadasHilo; //Instrucciones ejecutadas por rafaga
	int contador; // variable contadora utilizada para recorrer la lista de retornos
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
	pthread_mutex_lock(&instruccionesEjec);
	instruccionesEjecutadas[datos_CPU.id].contador = 0; //Instrucciones ejecutadas por calculo de tiempo. iniciamos las instrucciones ejecutadas a 0 cuando recien empieza, otro hilo sera el encargado de resetearla
	pthread_mutex_unlock(&instruccionesEjec);
	status=1;
	while(status != 0)
	{
		status = recibirPCB(datos_CPU.socket, &datos_CPU, &quantum);
		instruccionesEjecutadasHilo = 0; //Lo ponemos en 0 cada vez que vuelve a ejecutar un nuevo pcb
		if(status==0) break; //CIERRA EL RPOCESO
		pthread_mutex_lock(&mutexLog);
		log_info(log, "CPU:%d Recibido PCB:%d || Direccion archivo: %s  ||  Ip apuntando a la linea:%d   ||  Quantum recibido:%d", datos_CPU.id, datos_CPU.pid, datos_CPU.path, datos_CPU.ip, quantum);
		pthread_mutex_unlock(&mutexLog);
		entrada_salida = 0;//REINICIAMOS ESTOS VALORES PARA CADA VEZ QUE LEA UNA NUEVA PCB
		finArchivo = 0; //Indica el final del mcod
		datos_CPU.listaRetornos = NULL; // ACA HAGO QUE LA LISTA REINICIE AL LEER UN MCOD COMPLETO,la lista es liberada en la funcion desempaquetar
		//printf("CPU numero: %d \n", datos_CPU.id);
		//printf("PCB RECIBIDO: PID: %d  PATH: %s\n",datos_CPU.pid,datos_CPU.path);
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
			ejecutarInstruccion(&datos_CPU, instruccion, parametro1, parametro2, &entrada_salida, &finArchivo, &estado);
			pthread_mutex_lock(&instruccionesEjec);
			instruccionesEjecutadas[datos_CPU.id].contador ++;
			pthread_mutex_unlock(&instruccionesEjec);
			instruccionesEjecutadasHilo++;
			pthread_mutex_lock(&mutexLog);
			log_info(log, "CPU:%d || Instruccion ejecutada: %s || PID: %d", datos_CPU.id, lineaAEjecutar, datos_CPU.pid);//validar la funcion interpretarMCod para que si parametro 2 esta vacio lo devuelva con el \0 y si el parametro1 no esta que lo devuelva con -1
			if(parametro1 != -1) log_info(log, " Parametro 1: %d", parametro1);
			log_info(log, "Parametro 2: %s", parametro2);
			nodo_Retorno_Instruccion* aux = datos_CPU.listaRetornos; //vamos a tener que recorrer la lista ya que de ahi saco la respuesta
			for(contador = 1; contador < instruccionesEjecutadasHilo; contador++)
			{
				aux = aux->sgte;
			}
			log_info(log, " Resultado: %s", aux->info.texto);
			pthread_mutex_unlock(&mutexLog);
			//printf("%s \n",datos_CPU.listaRetornos->info.texto);
		}
		fclose(mCod);
		tamPayload = desempaquetarLista(&mensajeParaPL, datos_CPU.listaRetornos);//pasa la lista a un array de datos que es mensajeParaPL
		enviarMensajeAPL(datos_CPU,estado, entrada_salida, mensajeParaPL,tamPayload);
		free(mensajeParaPL);
		pthread_mutex_lock(&mutexLog);
		log_info("CPU: %d Rafaga del mProc cuyo PID es %d concluida.", datos_CPU.id, datos_CPU.pid);
		pthread_mutex_unlock(&mutexLog);
	}
	//printf("Terminando Hilo de CPU\n");
	pthread_mutex_lock(&mutexLog);
	//log_info("Terminando hilo de CPU %d", datos_CPU.id);
	pthread_mutex_unlock(&mutexLog);
	close(datos_CPU.socket);
	//printf("Cerrando socket\n");
	return;
}

void hiloCalculo()
{
	int contador;
	uint32_t porcentajeUso;
	mensaje_CPU_PL elMensaje;
	proceso_CPU CPU; //dato trucho para mandar el mensaje al PL y reutilizar el codigo.
	while(1)//va dentro de un while 1 para que se ejecute a menos que pase algun suceso
	{
		sleep(60);
		pthread_mutex_lock(&instruccionesEjec);
		for(contador = 0; contador < configuracion.CANTIDAD_HILOS; contador++)
		{
			if(configuracion.RETARDO>0) {
				porcentajeUso = ((instruccionesEjecutadas[contador].contador)*configuracion.RETARDO*100)/60;
			} else {
				porcentajeUso = ((instruccionesEjecutadas[contador].contador)*100)/60;
			}
			instruccionesEjecutadas[contador].contador = 0; // reiniciamos las instrucciones ejecutadas
			CPU.ip = contador;
			CPU.socket = instruccionesEjecutadas[contador].socket;
			enviarMensajeAPL(CPU, USOCPU, porcentajeUso,NULL, 0); //------------------
		}
		pthread_mutex_unlock(&instruccionesEjec);
	}
}
int main(void)
{
	log= log_create(ARCHIVOLOG, "CPU", 0, LOG_LEVEL_INFO);
	pthread_mutex_init(&mutexComADM,NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&instruccionesEjec, NULL);
	int i;
	log_info(log,"Iniciando CPU");
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	log_info(log,"Configuracion cargada correctamente");
	proceso_CPU CPUs[configuracion.CANTIDAD_HILOS];  //Declaramos el array donde cada componente es un hilo
	instruccionesEjecutadas = malloc(sizeof(contadorInstrucciones)* configuracion.CANTIDAD_HILOS);
	if((socketADM = crearSocketCliente(configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA))<0) // Se inicializa el socketADM GLOBAL
	{
		printf("No se pudo crear socket en %s:%s \n",configuracion.IP_MEMORIA,configuracion.PUERTO_MEMORIA); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
		log_error(log, "Proceso no se pudo conectar con el ADM", i);
		return 0;
	}
	log_info(log, "Proceso conectado con el ADM", i);
	for(i = 0;i < configuracion.CANTIDAD_HILOS; i++) // Vamos creando un hilo por cada CPU.
	{
		//printf("creando CPU %d\n",i);
		CPUs[i].id = i;
		if((CPUs[i].socket = crearSocketCliente(configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR))<0) // ponemos el socket de cada CPU
		{
			printf("No se pudo crear socket planificador en %s:%s \n",configuracion.IP_PLANIFICADOR,configuracion.PUERTO_PLANIFICADOR); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			pthread_mutex_lock(&mutexLog);
			log_error(log, "CPU %d no se pudo conectar con el Planificador", i);
			pthread_mutex_unlock(&mutexLog);
			close(socketADM); //
			return 0;
		}
		pthread_mutex_lock(&mutexLog);
		log_info(log, "CPU %d conectada con el Planificador", i);
		pthread_mutex_unlock(&mutexLog);
		instruccionesEjecutadas[i].socket = CPUs[i].socket; //Hacemos una copia del socket en la variable global
		pthread_create(&CPUs[i].thread, NULL, (void*)hiloCPU, (void*) &(CPUs[i]));
	}
	pthread_create(&hCalculo, NULL, (void*)hiloCalculo, NULL);
	for(i = 0;i < configuracion.CANTIDAD_HILOS; i++)// hacemos los join de cada cpu para que no corte antes.
	{
		pthread_join(CPUs[i].thread, NULL);
	}
	log_info(log,"Proceso CPU finalizado\n");
	close(socketADM);
	free(instruccionesEjecutadas);
	log_destroy(log);
	return 0;
}
