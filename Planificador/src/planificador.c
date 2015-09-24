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
#define RUTACONFIG "configuracion"

//variables globales (usar con cuidado)

config_pl configuracion;
nodo_Lista_CPU CPU1; //Va a ser lista
pthread_mutex_t MUTEXLISTOS;
pthread_mutex_t MUTEXPANTALLA;
pthread_t hConsola;
nodoPCB* raizListos;
pthread_t hServer;
sem_t SEMAFOROLISTOS;
int socketEscucha;

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
	}
	close(CPU1.socket); // Al terminar la consola no hace mas falta el cpu.
	printf("cierro socket cpu\n");
	return;
}

void manejadorCPU(void) //id Que CPU SOS
{	//mensaje_CPU_PL buffer; // VER IMPORTAR
	while(1) // AGEGAR CORTE
	{	sleep(4);
		sem_wait(&SEMAFOROLISTOS);
		pthread_mutex_lock(&MUTEXLISTOS);
		CPU1.ejecutando=sacarNodoPCB(&raizListos);
		printf("\nPor enviar a ejecutar: Path: %s Pid: %d\n",CPU1.ejecutando->info.path,CPU1.ejecutando->info.pid);
		pthread_mutex_unlock(&MUTEXLISTOS);
		enviarPCB(CPU1.socket,CPU1.ejecutando,-1);
		//recibirDeCPU(CPU1.socket); //DEFINIR y cuidado con IP al finalziar
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
	CPU1.socket = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	pthread_create(&hConsola,NULL,hiloConsola,NULL); //****************CREO LA CONSOLA
	pthread_create(&(CPU1.thread),NULL, manejadorCPU,NULL); //Chequear pasaje de parametro de id del cpu.
	pthread_join(hConsola,NULL);
	close(socketEscucha); //DEJO DE ESCUCHAR AL FINALIZAR LA CONSOLA
	printf("cierro socket escucha\n");
	return 0;
}

int main()
{
	pthread_mutex_init(&MUTEXLISTOS,NULL); //Inicializacion de los semaforos
	pthread_mutex_init(&MUTEXPANTALLA,NULL);
	sem_init(&SEMAFOROLISTOS,0,0);
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	printf("Bienvenido al proceso planificador \nEstableciendo conexion.. \n");
	pthread_create(&hServer,NULL,hiloServidor,NULL);
	pthread_join(hServer,NULL);
	pthread_mutex_destroy(&MUTEXLISTOS);
	sem_destroy(&SEMAFOROLISTOS);
	return 0;
}
