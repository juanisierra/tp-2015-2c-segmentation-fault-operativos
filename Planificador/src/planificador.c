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
#include <librerias-sf/listas.h>
#include <semaphore.h>
#include<sys/time.h>
#define TAMANOCONSOLA 1024
#define RUTACONFIG "configuracionPlanificador"

//variables globales (usar con cuidado)

config_pl configuracion;
nodo_Lista_CPU* raizCPUS; //Va a ser lista
nodoPCB* PCBBloqueado; //PCB QUE ESTA SIENDO BLOQEUADO EN ES
pthread_mutex_t MUTEXLISTOS;
pthread_mutex_t MUTEXBLOQUEADOS;
pthread_mutex_t MUTEXCPUS;
pthread_mutex_t MUTEXPROCESOBLOQUEADO; //Se usa para finalizar el proceso que esta siendo bloqueado en el hilo de bloqueos.
pthread_t hBloqueados;
pthread_t hConsola;
pthread_t hEnvios;
pthread_t hRecibir;
nodoPCB* raizListos;
nodoPCB* raizBloqueados;
pthread_t hServer;
sem_t SEMAFOROCPUSLIBRES;
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
	int i;
	int pid_cuenta =0; //PID ACTUAL
	nodoPCB* aFinalizar;
	nodo_Lista_CPU* CPUAux; // Contiene el cpu que se esta chequeando para ver si hay que finalizar su proceso.
	aFinalizar=NULL;
	raizListos=NULL;
	printf("Conectado al CPU, ya puede enviar mensajes. Escriba 'salir' para salir\n");
	while(estado_consola)
	{
		fgets(ingresado, TAMANOCONSOLA, stdin);	// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		separarInstruccionParametro(ingresado,instruccion,parametro);
		//leemos por teclado instruccion y parametro
		if (strcmp(instruccion,"salir")==0) estado_consola = 0;// Chequeo que el usuario no quiera salir
		if(strcmp(instruccion,"ps")==0)
		{
		mostrarPCBS(raizListos, raizBloqueados,PCBBloqueado,raizCPUS);
		}
		if(strcmp(instruccion,"correr")==0)//el usuario corre un programa
		{
			pthread_mutex_lock(&MUTEXLISTOS);
			agregarNodoPCB(&raizListos,crearNodoPCB(pid_cuenta,parametro));//agregamos el PCB a la lista de listos, uno a la vez.
			pid_cuenta++; //Aumenta PID
			pthread_mutex_unlock(&MUTEXLISTOS);
			sem_post(&SEMAFOROLISTOS);
		} //FALTA FINALIZAR
		if(strcmp(instruccion,"finalizar")==0)
		{		pthread_mutex_lock(&MUTEXCPUS); //CAMBIAR PARA MUCHOS CPU***************************************************************************************************************
		printf("POR FINALIZAR PID: %d\n",atoi(parametro));
		aFinalizar=NULL;
		CPUAux=NULL;
		for(i=0;i<cantidadCPUS(raizCPUS);i++)
		{
			CPUAux=CPUPosicion(raizCPUS,i);
			if( CPUAux->ejecutando!=NULL && CPUAux->ejecutando->info.pid==atoi(parametro))
			{
				aFinalizar=CPUAux->ejecutando;
				CPUAux->ejecutando->info.ip=ultimaLinea(CPUAux->ejecutando->info.path);
				CPUAux->finalizar=1;
			}

		}
			pthread_mutex_unlock(&MUTEXCPUS);
			if(aFinalizar==NULL) { //LO BUSCA EN EL HILO DE BLOQUEOS
				pthread_mutex_lock(&MUTEXPROCESOBLOQUEADO);
				if(PCBBloqueado->info.pid==atoi(parametro))
					{
					PCBBloqueado->info.ip=ultimaLinea(PCBBloqueado->info.path);
					aFinalizar=PCBBloqueado;
					}
				pthread_mutex_unlock(&MUTEXPROCESOBLOQUEADO);
			}
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

	return;
}
void hiloBloqueados(void)
{
	while(1)   //FALTA CONDICION DE CORTEEE
	{	sem_wait(&SEMAFOROBLOQUEADOS);
		pthread_mutex_lock(&MUTEXBLOQUEADOS);
		PCBBloqueado=sacarNodoPCB(&raizBloqueados);
		pthread_mutex_unlock(&MUTEXBLOQUEADOS);  //MUTEX DENTRO DE MUTEX!!!!!!!!!!!!!!!!!
		sleep(PCBBloqueado->info.bloqueo);
		pthread_mutex_lock(&MUTEXPROCESOBLOQUEADO);
		PCBBloqueado->info.bloqueo=0;
		PCBBloqueado->info.estado=LISTO;
		pthread_mutex_lock(&MUTEXLISTOS);
		agregarNodoPCB(&raizListos,PCBBloqueado);
		pthread_mutex_unlock(&MUTEXLISTOS);
		pthread_mutex_unlock(&MUTEXPROCESOBLOQUEADO);
		sem_post(&SEMAFOROLISTOS);
		PCBBloqueado=NULL;
	}

}
void interPretarMensajeCPU(mensaje_CPU_PL* mensajeRecibido,nodoPCB** PCB,nodo_Lista_CPU* CPU)
{	printf("RECIBI DENUEVO PCB\n");
	switch(mensajeRecibido->nuevoEstado)
	{
	case LISTO:
	{
	if(CPU->finalizar==0) //CHEQUEA QUE NO SE HAYA QUERIDO FINALIZAR
		{(*PCB)->info.ip=mensajeRecibido->ip;
		}
		(*PCB)->info.estado=mensajeRecibido->nuevoEstado;
		pthread_mutex_lock(&MUTEXLISTOS);					//MUTEX ADENTRO DE OTRO< CUIDADOOO, ADEMAS CAMBIAR PARA MUCHOS CPU
		agregarNodoPCB(&raizListos,*PCB);
		pthread_mutex_unlock(&MUTEXLISTOS);
		sem_post(&SEMAFOROLISTOS);
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
	{
	if(CPU->finalizar==0) //CHEQUEA QUE NO SE HAYA QUERIDO FINALIZAR  //MUTEX DENTRO DE OTRO Y CAMBIAR PARA MAS DE UN CPU
			{(*PCB)->info.ip=mensajeRecibido->ip;
			}
	(*PCB)->info.estado=mensajeRecibido->nuevoEstado;
	(*PCB)->info.bloqueo=mensajeRecibido->tiempoBloqueo;
	pthread_mutex_lock(&MUTEXBLOQUEADOS);
	agregarNodoPCB(&raizBloqueados,*PCB);
	pthread_mutex_unlock(&MUTEXBLOQUEADOS);
	sem_post(&SEMAFOROBLOQUEADOS);
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
	CPU->finalizar=0; //Vuelve a poner en 0 para que pueda ejecutar denuevo.
	return;
}

int hiloServidor(void)
{	int cuentaCPU=0;
	int socketCPU;
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

	while(1){ //FALTA CONDICION DE CORTE
	////***************************CREACION DEL CPU********************
		printf("esperando Otro\n");
	socketCPU = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	printf("Por agrega CPU\n");
	agregarCPU(&MUTEXCPUS,cuentaCPU,socketCPU,&raizCPUS);
	cuentaCPU++;
	printf("Se conecto el CPU %d\n",cuentaCPU-1);
	sem_post(&SEMAFOROCPUSLIBRES);
	}
	close(socketEscucha); //DEJO DE ESCUCHAR AL FINALIZAR LA CONSOLA
	printf("cierro socket escucha\n");
	return 0;
}
int hiloEnvios (void)
{	 nodo_Lista_CPU* cpuAEnviar;
	while(1) //FALTA CONDICION
	{
		sem_wait(&SEMAFOROLISTOS);
		printf("Hay Proceso libreS\n"); ////-------------------
		sem_wait(&SEMAFOROCPUSLIBRES);
		printf("Hay CPUS LIBRES\n"); ////-------------------
		pthread_mutex_lock(&MUTEXLISTOS);
		pthread_mutex_lock(&MUTEXCPUS);
		cpuAEnviar=primerCPULibre(raizCPUS);
		cpuAEnviar->ejecutando=sacarNodoPCB(&raizListos);
		cpuAEnviar->ejecutando->info.estado=EJECUTANDO;
		printf("\nPor enviar a ejecutar a CPU %d: Path: %s Pid: %d\n",cpuAEnviar->id,cpuAEnviar->ejecutando->info.path,cpuAEnviar->ejecutando->info.pid);
		pthread_mutex_unlock(&MUTEXLISTOS);
		enviarPCB(cpuAEnviar->socket,cpuAEnviar->ejecutando,quantum);
		pthread_mutex_unlock(&MUTEXCPUS);
	}
}
int hiloRecibir (void)
{	struct timeval tiempo;
	tiempo.tv_sec=1;
	tiempo.tv_usec=0;

	fd_set fdLeer;
	fd_set fdACerrar;
	nodo_Lista_CPU* aux;
	int i=0;
	int status;
	int maximoSocket=0;
	char buffer[100]; //Para guardarLoRecibido
	mensaje_CPU_PL mensajeRecibido;
	printf("Estoy por entrar en el bucle de receive\n");
	while(1) //Condicion de corte-----------------------------
	{
	pthread_mutex_lock(&MUTEXCPUS);
	for(i=0;i<cantidadCPUS(raizCPUS);i++)
	{
		FD_SET(socketCPUPosicion(raizCPUS,i),&fdLeer);
		FD_SET(socketCPUPosicion(raizCPUS,i),&fdACerrar);
		if(socketCPUPosicion(raizCPUS,i) > maximoSocket) maximoSocket=socketCPUPosicion(raizCPUS,i); //Setea el maximo numero de descriptor.
	}
	pthread_mutex_unlock(&MUTEXCPUS);
	select(maximoSocket+1,&fdLeer,NULL,&fdACerrar,&tiempo);
	pthread_mutex_lock(&MUTEXCPUS);
	for(i=0;i<cantidadCPUS(raizCPUS);i++) //CHEQUEA CUALES SE CERRARON
	{
		aux=CPUPosicion(raizCPUS,i);
		if(FD_ISSET(aux->socket,&fdACerrar))
		{
		status=	recv(aux->socket,&buffer,sizeof(mensaje_CPU_PL),MSG_PEEK | MSG_DONTWAIT);
		if(status==0){
			close(aux->socket);
			eliminarCPU(aux->id,&SEMAFOROCPUSLIBRES,&raizCPUS);
		}
		}
	}
	for(i=0;i<cantidadCPUS(raizCPUS);i++)  //CHEQUEA CUALES MANDARON COSAS
	{	aux=CPUPosicion(raizCPUS,i);
		if(FD_ISSET(aux->socket,&fdLeer))
		{
		status=recibirPCBDeCPU(aux->socket,&mensajeRecibido);
		if(status!=0){
		interPretarMensajeCPU(&mensajeRecibido,&(aux->ejecutando),aux);
		printf("La rafaga fue: %s\n",mensajeRecibido.payload);
		if(mensajeRecibido.payload!=NULL) free(mensajeRecibido.payload);

		sem_post(&SEMAFOROCPUSLIBRES);
		} else {
									///Si TERMINA MAL EL CPU Y NO CIERRA EL SOCKET LO BORRA IGUAL
		close(aux->socket);
		eliminarCPU(aux->id,&SEMAFOROCPUSLIBRES,&raizCPUS);
		}
		aux->ejecutando=NULL;
		}
	}
	pthread_mutex_unlock(&MUTEXCPUS);
	}
	return 0;
}
int main()
{	raizCPUS=NULL;
	raizBloqueados=NULL;
	pthread_mutex_init(&MUTEXLISTOS,NULL); //Inicializacion de los semaforos
	pthread_mutex_init(&MUTEXBLOQUEADOS,NULL);
	pthread_mutex_init(&MUTEXPROCESOBLOQUEADO,NULL);
	pthread_mutex_init(&MUTEXCPUS,NULL);
	sem_init(&SEMAFOROLISTOS,0,0);
	sem_init(&SEMAFOROBLOQUEADOS,0,0);
	sem_init(&SEMAFOROCPUSLIBRES,0,0);
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	if(configuracion.ALGORITMO_PLANIFICACION==1) quantum= -2;
	if(configuracion.ALGORITMO_PLANIFICACION==2) quantum=configuracion.QUANTUM;
	printf("Bienvenido al proceso planificador \nEstableciendo conexion.. \n");
	pthread_create(&hServer,NULL,hiloServidor,NULL);
	pthread_create(&hEnvios,NULL,hiloEnvios,NULL); //VER JOINS
	pthread_create(&hBloqueados,NULL,hiloBloqueados,NULL); //VER JOINS
	printf("Por crear hilo Recibir\n");
	 pthread_create(&hRecibir,NULL,hiloRecibir,NULL);
	pthread_create(&hConsola,NULL,hiloConsola,NULL); //****************CREO LA CONSOLA
	pthread_join(hConsola,NULL); //EL CPU 1 no tiene join, no funciona el devolver porqe no esta esperando.
	pthread_mutex_destroy(&MUTEXLISTOS);
	pthread_mutex_destroy(&MUTEXBLOQUEADOS);
	pthread_mutex_destroy(&MUTEXPROCESOBLOQUEADO);
	pthread_mutex_destroy(&MUTEXCPUS);
	sem_destroy(&SEMAFOROCPUSLIBRES);
	sem_destroy(&SEMAFOROLISTOS);
	sem_destroy(&SEMAFOROBLOQUEADOS);
	return 0;
}
