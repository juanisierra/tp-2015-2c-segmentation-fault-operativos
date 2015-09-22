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

typedef struct nodo_CPU_t
{
	struct nodo_Lista_CPU_t*sgte;
	struct nodo_Lista_CPU_t*ant;
	int id;
	int socket;
	pthread_t thread;
	pcb ejecutando;
} nodo_CPU;

//variables globales (usar con cuidado)

config_pl configuracion;
nodo_CPU CPU1; //Va a ser lista
nodoPCB raizListos;
int status;
pthread_mutex_t MUTEXLISTOS;
status = pthread_mutex_init(&MUTEXLISTOS,NULL);
sem_t SEMAFOROLISTOS;
status = sem_init(&SEMAFOROLISTOS,0,0);

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
	pcb auxPCB;
	printf("Conectado al CPU, ya puede enviar mensajes. Escriba 'salir' para salir\n");
	while(estado_consola)
	{
		fgets(ingresado, TAMANOCONSOLA, stdin);	// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		separarInstruccionParametro(ingresado,instruccion,parametro);
		//leemos por teclado instruccion y parametro
		if (strcmp(instruccion,"salir")==0) estado_consola = 0;// Chequeo que el usuario no quiera salir
		if(strcmp(instruccion,"correr")==0)//el usuario corre un programa
		{
			auxPCB.pid=pid_cuenta;//le damos pid's a los PCB desde 0
			pid_cuenta++;
			auxPCB.ip=0;//el puntero apunta a la primera instruccion
			auxPCB.estado=1;
			auxPCB.path=parametro;
			pthread_mutex_lock(&MUTEXLISTOS);
			agregarNodoPCB(raizListos,auxPCB);//agregamos el PCB a la lista de listos, uno a la vez.
			pthread_mutex_unlock(&MUTEXLISTOS);
			sem_post(SEMAFOROLISTOS);
		}
	}
	return;
}
int hiloServidor(void)
{
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
	CPU1.id = 1;
	CPU1.socket = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	pthread_create(&(CPU1.thread),NULL,manejadorCPU,NULL);
	close(socketEscucha);
	return 0;
}
int manejadorCPU(int id) //id Que CPU SOS
{	//mensaje_CPU_PL buffer; // VER IMPORTAR
	while(1) // AGEGAR CORTE
	{
		sem_wait(&SEMAFOROLISTOS);
		pthread_mutex_lock(&MUTEXLISTOS);
		CPU1.ejecutando=sacarNodoPCB(raizListos);
		pthread_mutex_unlock(&MUTEXLISTOS);
		enviarPCB(CPU1.socket,CPU1.ejecutando,-1);
		recibirDeCPU(CPU1.socket); //DEFINIR y cuidado con IP al finalziar
	}
	close(CPU1.socket);
	return 0;
}
int main()
{
	pthread_t hConsola;
	pthread_t hServer;
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	printf("Bienvenido al proceso planificador \nEstableciendo conexion.. \n");
	pthread_create(&hConsola,NULL,hiloConsola,NULL);
	pthread_create(&hServer,NULL,hiloServidor,NULL);
	pthread_join(hServer,NULL);
	pthread_join(hConsola,NULL);
	pthread_mutex_destroy(&MUTEXLISTOS);
	sem_destroy(&SEMAFOROLISTOS);
	return 0;
}
