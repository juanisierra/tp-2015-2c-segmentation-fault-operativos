#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <librerias-sf/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <librerias-sf/tiposDato.h>
#include <pthread.h>
#include <librerias-sf/strings.h>
#include <semaphore.h>
#define TAMANOCONSOLA 1024
#define RUTACONFIG "configuracionPlanificador"

//variables globales (usar con cuidado)

config_pl configuracion;
nodo_Lista_CPU CPU1; //Va a ser lista
pthread_mutex_t MUTEXLISTOS;
pthread_mutex_t MUTEXBLOQUEADOS;
pthread_mutex_t MUTEXPANTALLA;
pthread_t hConsola;
nodoPCB* raizListos;
nodoPCB* raizBloqueados;
pthread_t hServer;
sem_t SEMAFOROLISTOS;
sem_t SEMAFOROBLOQUEADOS;
int socketEscucha;
int quantum;
int iniciarConfiguracion(config_pl* configuracion)
{
	(*configuracion)= cargarConfiguracionPL(RUTACONFIG);
	if(configuracion->estado==0 || configuracion->estado==-1)
		{
			printf("Error de configuracion, cerrando proceso..");
			return -1;
		}
	if (configuracion->estado==1)
		{
			printf("Configuracion cargada correctamente: \n");
			printf("Puerto Escucha: %s\n",configuracion->PUERTO_ESCUCHA);
			printf("Codigo del Algoritmo de Planificacion: %d\n",configuracion->ALGORITMO_PLANIFICACION);
			printf("Quantum: %d\n\n",configuracion->QUANTUM);
			return 0;
		}
	return -1;
}
void hiloConsola(void)
{
	int estado_consola = 1;
	char ingresado[TAMANOCONSOLA];
	char instruccion[20];
	char parametro[50];
	int pid_cuenta =0; //PID ACTUAL
	nodoPCB* aFinalizar;
	raizListos=NULL;
	printf("Conectado al CPU, ya puede enviar mensajes. Escriba 'salir' para salir\n");
	while(estado_consola)
	{
		fgets(ingresado, TAMANOCONSOLA, stdin);	// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		separarInstruccionParametro(ingresado,instruccion,parametro);
		//leemos por teclado instruccion y parametro
		if (strcmp(instruccion,"salir")==0) estado_consola = 0;// Chequeo que el usuario no quiera salir
		if(strcmp(instruccion,"correr")==0)//el usuario corre un programa
		{
			pthread_mutex_lock(&MUTEXLISTOS);
			agregarNodoPCB(&raizListos,crearNodoPCB(pid_cuenta,parametro));//agregamos el PCB a la lista de listos, uno a la vez.
			pid_cuenta++; //Aumenta PID
			pthread_mutex_unlock(&MUTEXLISTOS);
			sem_post(&SEMAFOROLISTOS);
		}
		if(strcmp(instruccion,"finalizar")==0)
		{		pthread_mutex_lock(&(CPU1.MUTEXCPU)); //CAMBIAR PARA MUCHOS CPU***************************************************************************************************************
		printf("POR FINALIZAR PID: %d\n",atoi(parametro));
		if(CPU1.ejecutando->info.pid==atoi(parametro))
					{ aFinalizar=CPU1.ejecutando;
					CPU1.ejecutando->info.ip=ultimaLinea(CPU1.ejecutando->info.path);
					CPU1.finalizar=1;
					printf("puesto para finalzia en cpu1\n");
					} else {
						aFinalizar=NULL;
					}
					pthread_mutex_unlock(&(CPU1.MUTEXCPU));

			if(aFinalizar==NULL){
			pthread_mutex_lock(&MUTEXLISTOS);
			aFinalizar=buscarNodoPCB(raizListos,atoi(parametro));
			if(aFinalizar!=NULL) aFinalizar->info.ip=ultimaLinea(aFinalizar->info.path);
			pthread_mutex_unlock(&MUTEXLISTOS);
			}
			if(aFinalizar==NULL){ //No esta en listos, lo busca en bloqueados
			pthread_mutex_lock(&MUTEXBLOQUEADOS);
			aFinalizar=buscarNodoPCB(raizBloqueados,atoi(parametro));
			if(aFinalizar!=NULL) aFinalizar->info.ip=ultimaLinea(aFinalizar->info.path);
			pthread_mutex_unlock(&MUTEXBLOQUEADOS);
			}
		}
	}
	close(CPU1.socket); // Al terminar la consola no hace mas falta el cpu.
	printf("cierro socket cpu\n");
	return;
}

void interPretarMensajeCPU(mensaje_CPU_PL* mensajeRecibido,nodoPCB** PCB)
{	printf("RECIBID DENUEVO PCB\n");
	switch(mensajeRecibido->nuevoEstado)
	{
	case LISTO:
	{	pthread_mutex_lock(&(CPU1.MUTEXCPU));
	if(CPU1.finalizar==0) //CHEQUEA QUE NO SE HAYA QUERIDO FINALIZAR
		{(*PCB)->info.ip=mensajeRecibido->ip;
		}
		(*PCB)->info.estado=mensajeRecibido->nuevoEstado;
		sem_post(&SEMAFOROLISTOS);
		pthread_mutex_lock(&MUTEXLISTOS);					//MUTEX ADENTRO DE OTRO< CUIDADOOO, ADEMAS CAMBIAR PARA MUCHOS CPU
		agregarNodoPCB(&raizListos,*PCB);
		pthread_mutex_unlock(&MUTEXLISTOS);
		pthread_mutex_unlock(&(CPU1.MUTEXCPU));
		printf("EL PCB MANDADO A LA LISTA TIENE PID %d\n",(*PCB)->info.pid);
		(*PCB)=NULL;
		break;
	}
	case AFINALIZAR:
	{		(*PCB)->info.ip=mensajeRecibido->ip;
			(*PCB)->info.estado=mensajeRecibido->nuevoEstado;
			free(*PCB);
			(*PCB)=NULL;
		break;
	}
	case BLOQUEADO:
	{	pthread_mutex_lock(&(CPU1.MUTEXCPU));
	if(CPU1.finalizar==0) //CHEQUEA QUE NO SE HAYA QUERIDO FINALIZAR  //MUTEX DENTRO DE OTRO Y CAMBIAR PARA MAS DE UN CPU
			{(*PCB)->info.ip=mensajeRecibido->ip;
			}
	(*PCB)->info.estado=mensajeRecibido->nuevoEstado;
	(*PCB)->info.bloqueo=mensajeRecibido->tiempoBloqueo;
	sem_post(&SEMAFOROBLOQUEADOS);
	pthread_mutex_lock(&MUTEXBLOQUEADOS);
	agregarNodoPCB(&raizBloqueados,*PCB);
	pthread_mutex_unlock(&MUTEXBLOQUEADOS);
	pthread_mutex_unlock(&(CPU1.MUTEXCPU));
	(*PCB)=NULL;
		break;
	}
	case INVALIDO:
	{
		(*PCB)->info.ip=mensajeRecibido->ip;
		(*PCB)->info.estado=mensajeRecibido->nuevoEstado;
		printf("SE LEYO TEXTO INVALIDO EN EL ARCHIVO, CERRANDO PROCESO %d\n",(*PCB)->info.pid); //CAMBIARRRR**********************
		free(*PCB);
		(*PCB)=NULL;
		break;
	}
	case ERRORINICIO:
	{
		(*PCB)->info.ip=mensajeRecibido->ip;
		(*PCB)->info.estado=mensajeRecibido->nuevoEstado;
		printf("No hay espacio suficiente en el swap para el proceso %d \n",(*PCB)->info.pid); //CAMBIARRRR**********************
		free(*PCB);
		(*PCB)=NULL;
		break;
}
	}
	return;
}


void manejadorCPU(void) //id Que CPU SOS
{	mensaje_CPU_PL mensajeRecibido;

	while(1) // AGEGAR CORTE
	{	sleep(4);
		sem_wait(&SEMAFOROLISTOS);
		pthread_mutex_lock(&MUTEXLISTOS);
		CPU1.ejecutando=sacarNodoPCB(&raizListos);
		CPU1.ejecutando->info.estado=EJECUTANDO;
		CPU1.ant=NULL;
		CPU1.sgte=NULL;
		printf("\nPor enviar a ejecutar: Path: %s Pid: %d\n",CPU1.ejecutando->info.path,CPU1.ejecutando->info.pid);
		pthread_mutex_unlock(&MUTEXLISTOS);

		enviarPCB(CPU1.socket,CPU1.ejecutando,quantum);
		recibirPCBDeCPU(CPU1.socket,&mensajeRecibido);//DEFINIR y cuidado con IP al finalziar
		interPretarMensajeCPU(&mensajeRecibido,&(CPU1.ejecutando));
		printf("La rafaga fue: %s\n",mensajeRecibido.payload);
		if(mensajeRecibido.payload!=NULL) free(mensajeRecibido.payload);
	}
	return;
}
int hiloServidor(void)
{
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

	////***************************CREACION DEL CPU1********************
	CPU1.id = 1;
	CPU1.finalizar=0;
	pthread_mutex_init(&(CPU1.MUTEXCPU),NULL);
	CPU1.socket = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	pthread_create(&hConsola,NULL,hiloConsola,NULL); //****************CREO LA CONSOLA
	pthread_create(&(CPU1.thread),NULL, manejadorCPU,NULL); //Chequear pasaje de parametro de id del cpu.
	pthread_join(hConsola,NULL);
	pthread_mutex_destroy(&(CPU1.MUTEXCPU));
	close(socketEscucha); //DEJO DE ESCUCHAR AL FINALIZAR LA CONSOLA
	printf("cierro socket escucha\n");
	return 0;
}

int main()
{
	pthread_mutex_init(&MUTEXLISTOS,NULL); //Inicializacion de los semaforos
	pthread_mutex_init(&MUTEXBLOQUEADOS,NULL);
	pthread_mutex_init(&MUTEXPANTALLA,NULL);
	sem_init(&SEMAFOROLISTOS,0,0);
	sem_init(&SEMAFOROBLOQUEADOS,0,0);
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	if(configuracion.ALGORITMO_PLANIFICACION==1) quantum= -2;
	if(configuracion.ALGORITMO_PLANIFICACION==2) quantum=configuracion.QUANTUM;
	printf("Bienvenido al proceso planificador \nEstableciendo conexion.. \n");
	pthread_create(&hServer,NULL,hiloServidor,NULL);
	pthread_join(hServer,NULL);
	pthread_mutex_destroy(&MUTEXLISTOS);
	pthread_mutex_destroy(&MUTEXBLOQUEADOS);
	sem_destroy(&SEMAFOROLISTOS);
	sem_destroy(&SEMAFOROBLOQUEADOS);
	return 0;
}
