#include <stdlib.h>
#include <stdio.h>
#include <librerias-sf/sockets.h>
#include <librerias-sf/config.h>
#include <librerias-sf/tiposDato.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>
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
	instruccion_t tipoInst; //1 iniciar 2 leer pagina 3 escribir pagina 5 finaliza
	char*texto;
}espacioOcupado;

FILE* archivo;
config_SWAP configuracion;
espacioLibre* libreRaiz=NULL;
espacioOcupado* ocupadoRaiz=NULL;


int iniciarConfiguracion(cvoid)
{
	printf("Cargando configuracion.. \n \n");
	configuracion =  cargarConfiguracionSWAP(RUTACONFIG);
	if (configuracion.estado!=1)
	{
		printf("Error en el archivo de configuracion, cerrando Administrador de SWAP.. \n");
		return -1;
	}
	if(configuracion.estado==1){
		printf("Configuracion cargada correctamente: \nPuerto Escucha: %s \n",configuracion.PUERTO_ESCUCHA);
		printf("Nombre del Archivo de SWAP: %s \n", configuracion.NOMBRE_SWAP);
		printf("Cantidad de Paginas: %d \n",configuracion.CANTIDAD_PAGINAS);
		printf("Tamanio de Pagina: %d \n",configuracion.TAMANIO_PAGINA);
		printf("Retardo de Compactacion: %d \n \n",configuracion.RETARDO_COMPACTACION);
		return 0;
	}
	return -1;
}

int main()
{	char mensaje[3];


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

void inicializarArchivo(void)
{
    int tamanioArchivo=(configuracion.CANTIDAD_PAGINAS)*(configuracion.TAMANIO_PAGINA);
    char* s = string_repeat('\0', tamanioArchivo);
    fprintf(archivo,"%s", s);
    return;
}

int crearArchivo(void)//si devuelve NULL fallo
{
	char nombre [30];
	strcpy(nombre,configuracion.NOMBRE_SWAP );
	archivo= fopen(nombre,"w+");
	if(!archivo)
	{
		return archivo;
	}
    inicializarArchivo();
	libreRaiz=malloc(sizeof(espacioLibre));//creamos el primero nodo libre (que esTodo el archivo)
	if(!libreRaiz)
	{
		return 0;
	}
	libreRaiz->sgte=NULL;
	libreRaiz->ant=NULL;
	libreRaiz->comienzo=1;//posicion 1 en vez de 0, mas comodo
	libreRaiz->cantPag=(configuracion.CANTIDAD_PAGINAS);
	return 1;
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
	espacioLibre* aux=libreRaiz;//no modificamos a la var global
	while(posicion-1)//vamos al nodo elegido
	{
		aux= aux->sgte;
		posicion--;
	}
	if(aux->cantPag == espacio)
	{
		espacioLibre* aBorrar1=aux;
		if(aux->sgte == NULL) //si aux es el ultimo nodo
		{
			(aBorrar1->ant)->sgte=NULL;
			free(aBorrar1);
			return;
		}
		aux= aux->sgte;
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
	int hay=0;
	espacioLibre* raiz=libreRaiz;//para no cambiar al puntero
	while((!hay) && raiz)
	{
		if(raiz->cantPag >= espacio)
		{
			hay= raiz->comienzo;//comienzo !=0
		}
		raiz= raiz->sgte;
	}
	if(hay)
	{
		ocupar(hay, espacio);
	}
	return hay;//devolvemos 0 si no hay o la posicion inicial del hueco necesitado
}

void unirBloquesLibres(void)
{
    espacioLibre* nodoABorrar;
   libreRaiz->cantPag=libreRaiz->cantPag + (libreRaiz->sgte)->cantPag;
   if(!(libreRaiz->sgte->sgte))//solo son dos nodos
   {
	   nodoABorrar=libreRaiz->sgte;
	   free(nodoABorrar);
	   return;
   }
    (libreRaiz->sgte->sgte)->ant=libreRaiz;
    libreRaiz->sgte=(libreRaiz->sgte)->sgte;
    nodoABorrar=libreRaiz->sgte;
    free(nodoABorrar);
    return;
}

void desgragmentar(void)
{
	int sizeLibres=0;
	espacioLibre* auxLibre=libreRaiz;
	while(auxLibre)//contamos cuantos espacios libres hay
    {
        auxLibre=auxLibre->sgte;
        sizeLibres= sizeLibres+1;
	}

    espacioOcupado* auxO=ocupadoRaiz;
    while (sizeLibres>1)
    {
        if(libreRaiz->comienzo == 1)//si esta libre la primera pagina (o mas)
        {
        	moverInformacion(ocupadoRaiz->comienzo, ocupadoRaiz->cantPag, 1);
        	ocupadoRaiz->comienzo=1;//pasamos lo ocupado a la primera pagina
            libreRaiz->comienzo= 1 + ocupadoRaiz->cantPag;
            auxO=ocupadoRaiz;
            if(libreRaiz->sgte->comienzo == libreRaiz->comienzo + libreRaiz->cantPag)//si la pagina siguiente tmb esta libre
            {
            	unirBloquesLibres();
            	sizeLibres--;
            }
        }
        else
        {
            if(!auxO->sgte)// si ya ordenamos el ultimo nodo nos vamos

            {
            	return;
            }
        	auxO=auxO->sgte;//acomodamos el proximo proceso
        	moverInformacion(auxO->comienzo, auxO->cantPag, libreRaiz->comienzo);
            auxO->comienzo=libreRaiz->comienzo;//lo colocamos al principio de los libres
            libreRaiz->comienzo=libreRaiz->comienzo+auxO->cantPag;//movemos a libres
            if(libreRaiz->sgte->comienzo == libreRaiz->comienzo + libreRaiz->cantPag)//si la pagina siguiente tmb esta libre
            {
            	unirBloquesLibres();
            	sizeLibres--;
            }
        }
    }
    return;
}

void moverInformacion(int inicioDe, int cantPags, int inicioA)
{
	char buffer[cantPags * configuracion.TAMANIO_PAGINA];//creamos el buffer
	fseek(archivo, inicioDe * configuracion.TAMANIO_PAGINA, SEEK_SET);//vamos al inicio de ocupado
	fread(buffer, configuracion.TAMANIO_PAGINA, cantPags, archivo);//leemos
	fseek(archivo, inicioA * configuracion.TAMANIO_PAGINA, SEEK_SET);//vamos a libre
	fwrite(buffer, configuracion.TAMANIO_PAGINA, cantPags, archivo);//escribimos
	return;//en el "nuevo" libre ahora ha basura
}

