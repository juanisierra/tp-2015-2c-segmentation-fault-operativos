#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <librerias-sf/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#define TAMANOCONSOLA 1024
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracion"

typedef struct espacioLibre_t
{
	struct espacioLibre_t* sgte;
	struct espacioLibre_t* ant;
	uint32_t comienzo;
	uint32_t cantPag;
}espacioLibre;

typedef struct espacioOcupado_t
{
	struct espacioOcupado_t* sgte;
	struct espacioOcupado_t* ant;
	uint32_t comienzo;
	uint32_t cantPag;
	uint32_t pid;
	uint32_t tipoInst; //1 iniciar 2 leer pagina 3 escribir pagina 5 finaliza
	char*texto;
}espacioOcupado;

int iniciarConfiguracion(config_SWAP* configuracion)

{
	printf("Cargando configuracion.. \n \n");
	(*configuracion) =  cargarConfiguracionSWAP(RUTACONFIG);
	if (configuracion->estado!=1){
		printf("Error en el archivo de configuracion, cerrando Administrador de SWAP.. \n");
		return -1;
	}
	if(configuracion->estado==1){
		printf("Configuracion cargada correctamente: \nPuerto Escucha: %s \n",configuracion->PUERTO_ESCUCHA);
		printf("Nombre del Archivo de SWAP: %s \n", configuracion->NOMBRE_SWAP);
		printf("Cantidad de Paginas: %d \n",configuracion->CANTIDAD_PAGINAS);
		printf("Tamanio de Pagina: %d \n",configuracion->TAMANIO_PAGINA);
		printf("Retardo de Compactacion: %d \n \n",configuracion->RETARDO_COMPACTACION);
		return 0;
	}
	return -1;
}

espacioLibre* libreRaiz=NULL;
espacioOcupado* ocupadoRaiz=NULL;

int main()
{	char mensaje[3];
	config_SWAP configuracion;

	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	printf("Iniciando Administrador de SWAP.. \n");

	printf("Estableciendo conexion.. \n");

	int socketEscucha;
	socketEscucha= crearSocketEscucha(10,configuracion.PUERTO_ESCUCHA);
	if(socketEscucha < 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}

	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones en puerto %s..\n",configuracion.PUERTO_ESCUCHA);
	int socketADM = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);

	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Conectado al ADM en el puerto %s \n",configuracion.PUERTO_ESCUCHA);

	while (status != 0)
	{
		status = recv(socketADM, (void*) mensaje, TAMANOPAQUETE, 0);
		if(mensaje[0]!=3)
		{
			printf("El mensaje recibido por el socket del ADM no pertenece al mismo \n");
			close(socketADM);
			close(socketEscucha);
			return -1;


		}
		if(mensaje[1]!=4)
				{
					printf("El mensaje recibido por el socket del ADM no tiene como destino el Administrador de SWAP \n");

					close(socketADM);
					close(socketEscucha);
					return -1;
				}
		if(mensaje[2]==1)
		{
			printf("Mensaje Recibido\n");
			close(socketADM);
			close(socketEscucha);
			return 0;
		}
	}
	close(socketADM);
	close(socketEscucha);
	return 0;
}
FILE* crearArchivo(config_SWAP* configuracion)//si devuelve NULL fallo
{
	FILE* archivo;
	//char nombre [50];
	//strcpy(nombre,(*configuracion).NOMBRE_SWAP );
	archivo= fopen("swap.txt","w+");
	if(!archivo)
	{
		return archivo;
	}
	while(feof(archivo)) //incializa
	{
		fprintf(archivo,'/0');//nose si funciona bien
	}
	libreRaiz=malloc(sizeof(espacioLibre));//creamos el primero nodo libre (que es todo el archivo)
	if(!libreRaiz)
	{
		return 0;
	}
	libreRaiz->sgte=NULL;
	libreRaiz->ant=NULL;
	libreRaiz->comienzo=1;//posicion 1 en vez de 0, mas comodo
	libreRaiz->cantPag=(configuracion->CANTIDAD_PAGINAS);
	return archivo;
}
void ocupar(int posicion, int espacio)//probar esta funcion
{
	if(libreRaiz->cantPag == espacio)//si el espacio dado es el primer nodo completo
	{
		espacioLibre* aBorrar=libreRaiz;
		libreRaiz= libreRaiz->sgte;//si era el ultimo la raiz a libres apunta a nulo(no hay esp)
		libreRaiz->ant=NULL;
		free(aBorrar);//eliminamos el nodo
		return;
	}
	else if(posicion==1)//es el primer nodo pero sobra espacio
	{
		libreRaiz->comienzo= espacio+1;//creo que tiene sentido
		libreRaiz->cantPag= libreRaiz->cantPag - espacio;
		return;
	}
	espacioLibre* aux=libreRaiz;//no modificamos al la var global
	while(posicion-1)//vamos al nodo elegido
	{
		aux= aux->sgte;
		posicion--;
	}
	if(aux->cantPag == espacio)//que pasa si aux es el ultimo nodo?
	{
		espacioLibre* aBorrar1=aux;
		aux= aux->sgte; //no queda espacio
		(aux->ant) = (aBorrar1->ant);
		aBorrar1= aBorrar1->ant;
		aBorrar1->sgte = aux;
		aBorrar1= aBorrar1->sgte;
		free(aBorrar1);
		return;
	}
	else
	{
		aux->comienzo= espacio+1;
		aux->cantPag= libreRaiz->cantPag - espacio;
		return;
	}
}
int hayEspacio(int espacio)//espacio esta en paginas
{
	int noHay=1;
	espacioLibre* raiz=libreRaiz;//para no cambiar al puntero
	while(noHay && raiz)
	{
		if(raiz->cantPag >= espacio)
		{
			noHay= raiz->comienzo;//comienzo !=0
		}
		raiz= raiz->sgte;
	}
	if(noHay)
	{
		ocupar(noHay, espacio);
	}
	return noHay;//devolvemos 0 si no hay o la posicion inicial del hueco necesitado
}
//int asignarEspacio(espacioOcupado** ocupado, int posicion, int tamanio)//punteros a punteros por si hay que modificarlos
