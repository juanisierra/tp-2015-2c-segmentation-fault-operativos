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
}espacioOcupado;

//****** OJO QUE SON VARIABLES GLOBALES ******
FILE* archivo=NULL;
config_SWAP configuracion;
espacioLibre* libreRaiz=NULL;
espacioOcupado* ocupadoRaiz=NULL;

int iniciarConfiguracion(void)
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

void inicializarArchivo(void)
{
    int tamanioArchivo=(configuracion.CANTIDAD_PAGINAS)*(configuracion.TAMANIO_PAGINA);
    char* s = string_repeat('\0', tamanioArchivo);
    fprintf(archivo,"%s", s);
    return;
}

int crearArchivo(void)//0 mal 1 bien
{
	char nombre [30];
	strcpy(nombre,configuracion.NOMBRE_SWAP );
	archivo= fopen(nombre,"w+");
	if(archivo==0)
	{
		printf("fallo al crear el archivo en swap.c \n");
		return 0;
	}
    inicializarArchivo();
	libreRaiz=malloc(sizeof(espacioLibre));//creamos el primero nodo libre (que esTodo el archivo)
	if(!libreRaiz)
	{
		printf("fallo el malloc para la lista de libres en swap.c \n");
		return 0;
	}
	libreRaiz->sgte=NULL;
	libreRaiz->ant=NULL;
	libreRaiz->comienzo=1;//posicion 1 en vez de 0, mas comodo
	libreRaiz->cantPag=(configuracion.CANTIDAD_PAGINAS);
	return 1;
}

void ocupar(int posicion, int espacio)
{
	espacioLibre* aBorrar=libreRaiz;
	if(libreRaiz->cantPag == espacio)//si el espacio dado es el primer nodo completo
	{
		if(!(libreRaiz->sgte))//era el unico nodo libre
		{
			free(libreRaiz);
			libreRaiz=NULL;
			return;
		}
		libreRaiz= libreRaiz->sgte;
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
	while(aux->comienzo != posicion)//vamos al nodo elegido
	{
		aux= aux->sgte;
	}
	if(aux->cantPag == espacio)// nos piden el nodo entero
	{
		aBorrar=aux;
		if(aux->sgte == NULL) //si aux es el ultimo nodo
		{
			(aBorrar->ant)->sgte=NULL;
			free(aBorrar);
			return;
		}
		aux= aux->sgte;
		aux->ant = aBorrar->ant;
		aBorrar->ant->sgte=aux;
		free(aBorrar);
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

void moverInformacion(int inicioDe, int cantPags, int inicioA)// puse unos -1 alguien que me confime que tiene sentido
{
	char buffer[cantPags * configuracion.TAMANIO_PAGINA];//creamos el buffer
	fseek(archivo, (inicioDe -1) * configuracion.TAMANIO_PAGINA, SEEK_SET);//vamos al inicio de ocupado
	fread(buffer, sizeof(char), strlen(buffer), archivo);//leemos
	fseek(archivo, (inicioA -1) * configuracion.TAMANIO_PAGINA, SEEK_SET);//vamos a libre
	fwrite(buffer, sizeof(char), strlen(buffer), archivo);//escribimos
	return;//en el "nuevo" libre ahora hay basura
}

void desfragmentar(void)
{
	printf("se desfragmenta el archivo... \n");
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
    	while(auxO && auxO->comienzo != libreRaiz->comienzo + libreRaiz->cantPag)
    	{//vamos al nodo ocupado a la derecha del nodo libre, si existe
    		auxO=auxO->sgte;
    	}
    	if(!auxO) return;//si no hay nodos a la derecha nos vamos
    	moverInformacion(auxO->comienzo, auxO->cantPag, libreRaiz->comienzo);
    	auxO->comienzo= libreRaiz->comienzo;//lo colocamos al principio de los libres
    	libreRaiz->comienzo= libreRaiz->comienzo + auxO->cantPag;//movemos la raiz al final del nodo ocupado
    	if(libreRaiz->sgte->comienzo == libreRaiz->comienzo + libreRaiz->cantPag)
    	{//si la pagina siguiente tmb esta libre los unimos en un solo nodo
    		unirBloquesLibres();
    		sizeLibres--;
    	}
    }
	return;
}

int agregarOcupado(uint32_t pid, uint32_t cantPag, int comienzo)//LOS NOCOS OCUPADOS SE APILAN NO HAY ORDEN RESPECTO A LO QUE OCUPAN EN MEMORIA
{
	if(!ocupadoRaiz)
	{
		ocupadoRaiz= malloc(sizeof(espacioOcupado));
		if(!ocupadoRaiz)
			{
				printf("fallo el malloc para la lista de ocupados en swap.c \n");
				return 0;
			}
		ocupadoRaiz->ant=NULL;
		ocupadoRaiz->sgte=NULL;
		ocupadoRaiz->pid=pid;
		ocupadoRaiz->cantPag=cantPag;
		ocupadoRaiz->comienzo=comienzo;
		return 1;
	}
	espacioOcupado* ultimo= ocupadoRaiz;
	while(ultimo->sgte)//puntero al ultimo nodo de ocupados
	{
		ultimo=ultimo->sgte;
	}
	espacioOcupado* nuevo= malloc(sizeof(espacioOcupado));
	if(!nuevo)
	{
		printf("fallo el malloc para la lista de ocupados en swap.c \n");
		return 0;
	}
	ultimo->sgte=nuevo;
	nuevo->sgte=NULL;
	nuevo->ant=ultimo;
	nuevo->pid=pid;
	nuevo->cantPag=cantPag;
	nuevo->comienzo=comienzo;
	return 1;
}

int asignarMemoria( uint32_t pid, uint32_t cantPag)
{
	int inicio;
	inicio= hayEspacio(cantPag);
	if(!inicio)
	{
		desfragmentar();
		inicio = hayEspacio(cantPag);
		if(!inicio)
		{
			printf("no hay espacio suficiente para el proceso de pid: %u \n", pid);
			return 0;
		}
	}
	int exito= agregarOcupado(pid, cantPag, inicio);
	return exito;
}

int atras(espacioOcupado* nodo)// 0 no hay nada 1 libre 2 ocupado
{
	int comienzo = nodo->comienzo;
	espacioLibre* libreAux= libreRaiz;
	espacioOcupado* ocupadoAux= ocupadoRaiz;
	int encontrado= 0;
	while(!encontrado && libreAux)
	{
		if(comienzo == libreAux->comienzo + libreAux->cantPag)
		{
			encontrado=1;
		}
		libreAux= libreAux->sgte;
	}
	while(!encontrado && ocupadoAux)
	{
		if(comienzo == ocupadoAux->comienzo + ocupadoAux->cantPag)
		{
			encontrado=2;
		}
		ocupadoAux= ocupadoAux->sgte;
	}
	return encontrado;
}

int adelante(espacioOcupado* nodo)// 0 no hay nada 1 libre 2 ocupado
{
	int comienzo = nodo->comienzo;
	int cantPag = nodo->cantPag;
	espacioLibre* libreAux= libreRaiz;
	espacioOcupado* ocupadoAux= ocupadoRaiz;
	int encontrado= 0;
	while(!encontrado && libreAux)
	{
		if(comienzo + cantPag == libreAux->comienzo)
		{
			encontrado=1;
		}
		libreAux= libreAux->sgte;
	}
	while(!encontrado && ocupadoAux)
	{
		if(comienzo + cantPag == ocupadoAux->comienzo)
		{
			encontrado=2;
		}
		ocupadoAux= ocupadoAux->sgte;
	}
	return encontrado;
}

void borrarNodoOcupado(espacioOcupado* aBorrar)
{
	if(ocupadoRaiz == aBorrar)
	{
		if(ocupadoRaiz->sgte)
		{
			ocupadoRaiz= ocupadoRaiz->sgte;
			ocupadoRaiz->ant= NULL;
		}
		else
		{
			ocupadoRaiz=NULL;
		}
		free(aBorrar);
		return;
	}
	aBorrar->ant->sgte= aBorrar->sgte;
	if(aBorrar->sgte)
	{
		aBorrar->sgte->ant= aBorrar->ant;
	}
	free(aBorrar);
	return;
}

int liberarMemoria(espacioOcupado* aBorrar)
{
	int atrasVar= atras(aBorrar);
	int adelanteVar= adelante(aBorrar);
	if(atrasVar==0 && adelanteVar==0)//el proceso ocupa el archivo entero
	{
		inicializarArchivo();//no hace falta pero bueno ya que estamos
		libreRaiz=malloc(sizeof(espacioLibre));//creamos el primero nodo libre (que esTodo el archivo)
		if(!libreRaiz)
		{
			printf("fallo el malloc para la lista de libres en swap.c \n");
			return 0;
		}
		libreRaiz->sgte=NULL;
		libreRaiz->ant=NULL;
		libreRaiz->comienzo=1;//posicion 1 en vez de 0, mas comodo
		libreRaiz->cantPag=(configuracion.CANTIDAD_PAGINAS);
		borrarNodoOcupado(aBorrar);
		return 1;
	}
	else if(atrasVar==0 && adelanteVar==1)//tiene un libre adelante
	{
		libreRaiz->comienzo=1;
		libreRaiz->cantPag= libreRaiz->cantPag + aBorrar->cantPag;//ACA NO HACE FALTA FREE CREO
		borrarNodoOcupado(aBorrar);
		return 1;
	}
	else if(atrasVar==1 && adelanteVar==0)//tiene libre atras y nada adelante
	{
		espacioLibre* ultimo= libreRaiz;
		while(ultimo->sgte)//vamos al ultimo nodo libre
		{
			ultimo= ultimo->sgte;
		}
		ultimo->cantPag= ultimo->cantPag + aBorrar->cantPag;
		borrarNodoOcupado(aBorrar);
		return 1;
	}
	else if(atrasVar==2 && adelanteVar==2)//esta entre dos ocupados, hay varios sub casos aca
	{
		if(!libreRaiz)// estaba ocupado entero y sacamos un proceso
		{
			libreRaiz=malloc(sizeof(espacioLibre));
			if(!libreRaiz)
			{
				printf("fallo el malloc para la lista de libres en swap.c \n");
				return 0;
			}
			libreRaiz->sgte=NULL;
			libreRaiz->ant=NULL;
			libreRaiz->comienzo= aBorrar->comienzo;
			libreRaiz->cantPag= aBorrar->cantPag;
			borrarNodoOcupado(aBorrar);
			return 1;
		}
		else if(libreRaiz->comienzo > aBorrar->cantPag + aBorrar->comienzo)
		{//la raiz esta adelante. pasamos la raiz atras y agregamos un nodo donde estaba la raiz
			espacioLibre* nuevo;
			nuevo=malloc(sizeof(espacioLibre));
			if(!nuevo)
			{
				printf("fallo el malloc para la lista de libres en swap.c \n");
				return 0;
			}
			if(libreRaiz->sgte)
			{
				libreRaiz->sgte->ant=nuevo;
			}
			nuevo->cantPag = libreRaiz->cantPag;
			nuevo->comienzo = libreRaiz->comienzo;
			nuevo->sgte= libreRaiz->sgte;
			nuevo->ant= libreRaiz;
			libreRaiz->cantPag= aBorrar->cantPag;
			libreRaiz->comienzo= aBorrar->comienzo;
			libreRaiz->ant=NULL;
			libreRaiz->sgte=nuevo;
			borrarNodoOcupado(aBorrar);
			return 1;
		}
		else//la raiz esta atras
		{
			espacioLibre* anterior= libreRaiz;
			while(anterior->sgte && anterior->comienzo + anterior->cantPag < aBorrar->comienzo)//buscamos el libre anterior
			{
				anterior= anterior->sgte;
			}//no estamos seguros por cual de las dos condiciones salio del while
			if(anterior->comienzo + anterior->cantPag > aBorrar->comienzo)//si salio por ser el ultimo barbaro pero sino chequeamos este if
			{
				anterior= anterior->ant;
			}
			espacioLibre* nuevo;
			nuevo=malloc(sizeof(espacioLibre));
			if(!nuevo)
			{
				printf("fallo el malloc para la lista de libres en swap.c \n");
				return 0;
			}
			nuevo->comienzo= aBorrar->comienzo;
			nuevo->cantPag= aBorrar->cantPag;
			nuevo->ant= anterior;
			anterior->sgte= nuevo;
			borrarNodoOcupado(aBorrar);
			return 1;
		}
	}
	else if(atrasVar==1 && adelanteVar==2)//esta entre un libre y un ocupado
	{
		espacioLibre* anterior= libreRaiz;
		while(anterior->comienzo + anterior->cantPag != aBorrar->comienzo)
		{
			anterior= anterior->sgte;
		}
		anterior->cantPag = anterior->cantPag + aBorrar->cantPag;
		borrarNodoOcupado(aBorrar);
		return 1;
	}
	else if(atrasVar==2 && adelanteVar==1)//esta entre un ocupado y un libre
	{
			espacioLibre* siguiente= libreRaiz;
			while(siguiente->comienzo != aBorrar->comienzo + aBorrar->cantPag)
			{//si a la derecha esta la raiz anda igual
				siguiente= siguiente->sgte;
			}
			siguiente->comienzo= aBorrar->comienzo;
			siguiente->cantPag= siguiente->cantPag + aBorrar->cantPag;
			borrarNodoOcupado(aBorrar);
			return 1;
	}
	else if(atrasVar==1 && adelanteVar==1)//esta entre dos libres
	{
		espacioLibre* anterior= libreRaiz;
		while(anterior->comienzo + anterior->cantPag != aBorrar->comienzo)
		{
			anterior= anterior->sgte;
		}
		anterior->cantPag = anterior->cantPag + (anterior->sgte)->cantPag + aBorrar->cantPag;
		if(anterior->sgte->sgte)//si hay un tercer nodo libre
		{
			anterior->sgte->sgte->ant=anterior;
		}
		borrarNodoOcupado(aBorrar);
		free(anterior->sgte);
		return 1;
	}
	else if(atrasVar==2 && adelanteVar==0)//entre un ocupado y el final del archivo
	{
		if(!libreRaiz)
		{
			libreRaiz=malloc(sizeof(espacioLibre));
			if(!libreRaiz)
			{
				printf("fallo el malloc para la lista de libres en swap.c \n");
				return 0;
			}
			libreRaiz->sgte=NULL;
			libreRaiz->ant=NULL;
			libreRaiz->comienzo= aBorrar->comienzo;//posicion 1 en vez de 0, mas comodo
			libreRaiz->cantPag= aBorrar->cantPag;
			borrarNodoOcupado(aBorrar);
			return 1;
		}
		else
		{
			espacioLibre* anterior= libreRaiz;
			while( anterior->sgte )//vamos al ultimo nodo de los libres
			{
				anterior= anterior->sgte;
			}
			espacioLibre* nuevo;
			nuevo=malloc(sizeof(espacioLibre));
			if(!nuevo)
			{
				printf("fallo el malloc para la lista de libres en swap.c \n");
				return 0;
			}
			nuevo->comienzo= aBorrar->comienzo;
			nuevo->cantPag= aBorrar->cantPag;
			nuevo->ant= anterior;
			anterior->sgte= nuevo;
			borrarNodoOcupado(aBorrar);
			return 1;
		}
	}
	else if(atrasVar==0 && adelanteVar==2)//entre el inicio del archivo y un ocupado
	{
		if(!libreRaiz)
		{
			libreRaiz=malloc(sizeof(espacioLibre));
			if(!libreRaiz)
			{
				printf("fallo el malloc para la lista de libres en swap.c \n");
				return 0;
			}
			libreRaiz->sgte=NULL;
			libreRaiz->ant=NULL;
			libreRaiz->comienzo= aBorrar->comienzo;
			libreRaiz->cantPag= aBorrar->cantPag;
			borrarNodoOcupado(aBorrar);
			return 1;
		}
		espacioLibre* nuevo;
		nuevo=malloc(sizeof(espacioLibre));
		if(!nuevo)
		{
			printf("fallo el malloc para la lista de libres en swap.c \n");
			return 0;
		}
		if(libreRaiz->sgte)//corremos la raiz al inicio y nuevo donde estaba la raiz
		{
			libreRaiz->sgte->ant=nuevo;
		}
		nuevo->cantPag = libreRaiz->cantPag;
		nuevo->comienzo = libreRaiz->comienzo;
		nuevo->sgte= libreRaiz->sgte;
		nuevo->ant= libreRaiz;
		libreRaiz->cantPag= aBorrar->cantPag;
		libreRaiz->comienzo= aBorrar->comienzo;
		libreRaiz->ant=NULL;
		borrarNodoOcupado(aBorrar);
		return 1;
	}
	return 0;//si llego hasta aca es porque algo salio mal
}

char* leer(espacioOcupado* aLeer, uint32_t pagALeer)
{
	char * buffer =NULL;
	buffer= malloc(configuracion.TAMANIO_PAGINA);
	if(!buffer)
	{
		printf("Fallo la creacion del buffer en el swap funcion leer");
		return buffer;
	}
	fseek(archivo,(aLeer->comienzo -1) * configuracion.TAMANIO_PAGINA +  pagALeer * configuracion.TAMANIO_PAGINA, SEEK_SET);//vamos la pagina a leer (sin los menos uno la pasamos)
	fread(buffer, sizeof(char), (configuracion.TAMANIO_PAGINA)/sizeof(char), archivo);//leemos

	if(buffer!=NULL) printf("Se lee: %s \n", buffer); //////////////////////valgrind
	return buffer;
}

void escribir(espacioOcupado* aEscribir, uint32_t pagAEscribir, char* texto)// 0 mal 1 bien
{
	fseek(archivo,(aEscribir->comienzo -1) * configuracion.TAMANIO_PAGINA +  pagAEscribir * configuracion.TAMANIO_PAGINA, SEEK_SET);
	fwrite(texto, sizeof(char), strlen(texto), archivo);

	return;
}

int interpretarMensaje(mensaje_ADM_SWAP mensaje,int socketcito)
{
	mensaje_SWAP_ADM aEnviar;
	int resultado;
	espacioOcupado* aBorrar;
	espacioOcupado*aEscribir;
	switch (mensaje.instruccion)
	{
	case INICIAR:
		resultado=asignarMemoria(mensaje.pid, mensaje.parametro);
		aEnviar.contenidoPagina=NULL;
		if (resultado==0)
		{
			aEnviar.estado=1;
			aEnviar.instruccion=mensaje.instruccion;
			int i= enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
			if(!i) printf("No se pudo enviar mensaje al ADM\n"); // Distinto de i no sirve, i es cuanto manda.--------------------
			return 0;
		}
		break;

	case FINALIZAR:
		aBorrar=ocupadoRaiz;
		while (aBorrar && aBorrar->pid != mensaje.pid) aBorrar= aBorrar->sgte;
		aEnviar.contenidoPagina=NULL;
		if(!aBorrar)
		{
			printf ("El proceso no se encuentra en el swap \n");
			aEnviar.estado=1;
			aEnviar.instruccion=mensaje.instruccion;
			int i=enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
			if(!i) printf("No se pudo enviar mensaje al ADM\n"); //INSTRUCCION =i, i es la cantidad de datos que enviaa,
			return 0;
		}
		liberarMemoria(aBorrar);
		printf("Se libero la memoria del proceso\n");
		aEnviar.contenidoPagina=NULL;
		break;

	case LEER:
		aEnviar.contenidoPagina=malloc(configuracion.TAMANIO_PAGINA);
		espacioOcupado* aLeer=ocupadoRaiz;
		while(aLeer && aLeer->pid != mensaje.pid) aLeer=aLeer->sgte;
		if(!aLeer)
		{
			printf ("El proceso no se encuentra en el swap \n");
			aEnviar.estado=1;
			aEnviar.instruccion=mensaje.instruccion;
			aEnviar.contenidoPagina=NULL;
			int i=enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
			if(!i) printf("No se pudo enviar mensaje al ADM\n");
			return 0;
		}
		char* leido= leer(aLeer, mensaje.parametro);
		aEnviar.contenidoPagina=malloc(configuracion.TAMANIO_PAGINA);
		memcpy((aEnviar.contenidoPagina),leido,configuracion.TAMANIO_PAGINA); //COPIO Y FATA FREE DE AENVIAR
		free(leido);
		break;

	case ESCRIBIR:

		aEscribir=ocupadoRaiz;
		while(aEscribir && aEscribir->pid != mensaje.pid) aEscribir=aEscribir->sgte;
		aEnviar.contenidoPagina=NULL;
		if(!aEscribir)
		{
			printf ("El proceso no se encuentra en el swap \n");
			aEnviar.estado=1;
			aEnviar.instruccion=mensaje.instruccion;

			int i=enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
			if(!i) printf("No se pudo enviar mensaje al ADM\n");
			return 0;
		}
		escribir(aEscribir, mensaje.parametro, mensaje.contenidoPagina);
		aEnviar.contenidoPagina=NULL;
		break;
	}

	aEnviar.instruccion=mensaje.instruccion;
	aEnviar.estado=0;// si llegó hasta acá es porque esta OK (estado=0)

	int i=0;
	i=enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
	if(!i)
	{
		printf("No se pudo enviar mensaje al ADM\n");
		return 0;
	}
	if(aEnviar.contenidoPagina!=NULL)
		{
			free(aEnviar.contenidoPagina);
			aEnviar.contenidoPagina=NULL;
		}

	if(mensaje.contenidoPagina!=NULL)
		{
			free(mensaje.contenidoPagina);
			mensaje.contenidoPagina=NULL;
		}

	return 1;
}

void eliminarListas(void)
{
	if(libreRaiz)//si hay nodos libres
	{
		espacioLibre* ultimoLibre=libreRaiz;
		while(ultimoLibre->sgte)
		{
			ultimoLibre=ultimoLibre->sgte;
		}
		espacioLibre* anterior= ultimoLibre->ant;
		while(anterior)
		{
			free(ultimoLibre);
			ultimoLibre=anterior;
			anterior= ultimoLibre->ant;
		}
		free(ultimoLibre);
		libreRaiz=NULL;
	}
	if(ocupadoRaiz)//si hay nodos ocupados
	{
		espacioOcupado* ultimoOcupado=ocupadoRaiz;
		while(ultimoOcupado->sgte)
		{
			ultimoOcupado=ultimoOcupado->sgte;
		}
		espacioOcupado* anterior= ultimoOcupado->ant;
		while(anterior)
		{
			free(ultimoOcupado);
			ultimoOcupado=anterior;
			anterior= ultimoOcupado->ant;
		}
		free(ultimoOcupado);
		ocupadoRaiz=NULL;
	}
	return;
}

int main()
{
	mensaje_ADM_SWAP mensaje;
	if(iniciarConfiguracion()==-1) return -1;
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
	int exito= crearArchivo();
	if(exito) printf("se creo el archivo correctamente\n");
	while (status != 0)
	{
		status = recibirPaginaDeADM(socketADM,&mensaje,configuracion.TAMANIO_PAGINA);
		if(status==0) break; //SACAR PARA VER DOBLE FREEE!!!!
		printf("Recibi de ADM: Pid: %d Inst: %d Parametro: %d\n",mensaje.pid,mensaje.instruccion,mensaje.parametro);
		interpretarMensaje(mensaje,socketADM);
	}
	close(socketADM);
	close(socketEscucha);
	fclose(archivo);
	eliminarListas();
	return 0;
}
