//SWAP, MADE IN CABALLITO//
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
#include <commons/log.h>
#define TAMANOCONSOLA 1024
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracion"
#define ARCHIVOLOG "Swap.log"
//****** VARIABLES GLOBALES ******
FILE* archivo=NULL;
config_SWAP configuracion;
espacioLibre* libreRaiz=NULL;
espacioOcupado* ocupadoRaiz=NULL;
t_log* log;
//********************************

int iniciarConfiguracion(void)
{//cargamos la configuracion del swap
	printf("Cargando configuracion.. \n \n");
	configuracion =  cargarConfiguracionSWAP(RUTACONFIG);
	if (configuracion.estado!=1)
	{
		printf("Error en el archivo de configuracion, cerrando Administrador de SWAP.. \n");
		log_error(log, "Error en el archivo de configuracion");
		return -1;
	}
	if(configuracion.estado==1)
	{
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
{//lo llenamos con el caracter correspondiente
    int tamanioArchivo=(configuracion.CANTIDAD_PAGINAS)*(configuracion.TAMANIO_PAGINA);
    char* s = string_repeat('\0', tamanioArchivo);
    fprintf(archivo,"%s", s);
    free(s);
    return;
}

int crearArchivo(void)//0 mal 1 bien
{//creamos el archivo y el primer nodo de listos
	char nombre [30];
	strcpy(nombre,configuracion.NOMBRE_SWAP );
	archivo= fopen(nombre,"w+");
	if(archivo==0)
	{
		printf("fallo al crear el archivo en swap.c \n");
		log_error(log, "Fallo al crear el archivo de swap");
		return 0;
	}
    inicializarArchivo();
	libreRaiz=malloc(sizeof(espacioLibre));//creamos el primero nodo libre (que esTodo el archivo)
	if(!libreRaiz)
	{
		printf("fallo el malloc para la lista de libres en swap.c \n");
		log_error(log, "Fallo el malloc para la lista libres en swap");
		return 0;
	}
	libreRaiz->sgte=NULL;
	libreRaiz->ant=NULL;
	libreRaiz->comienzo=1;//posicion 1 en vez de 0, mas comodo
	libreRaiz->cantPag=(configuracion.CANTIDAD_PAGINAS);
	return 1;
}

void ocupar(int posicion, int espacio)
{//cambia un espacio libre a ocupado
	espacioLibre* aux=libreRaiz;
	while(aux->comienzo != posicion)//vamos al nodo a ocupar
	{
		aux= aux->sgte;
	}
	aux->comienzo= aux->comienzo + espacio;
	aux->cantPag= aux->cantPag - espacio;
	if(aux->cantPag == 0)//si nos piden el nodo entero
	{
		if(aux->ant)
		{
			aux->ant->sgte= aux->sgte;
		}
		if(aux->sgte)
		{
			aux->sgte->ant= aux->ant;
		}

		if(aux == libreRaiz)
		{
			libreRaiz= libreRaiz->sgte;
			if(libreRaiz)
			{
				libreRaiz->ant= NULL;
			}
		}
		free(aux);
	}
	return;
}

int hayEspacio(int espacio)//espacio esta en paginas
{//te dice si hay un nodo libre con es el espacio requerido
	int hay;
	espacioLibre* raiz=libreRaiz;//para no cambiar al puntero
	while(raiz)
	{
		if(raiz->cantPag >= espacio)
		{
			hay= raiz->comienzo;//el comienzo minimo es 1
			ocupar(hay, espacio);//ocupamos el espacio requerido por el proceso
			return hay;//devolvemos la posicion inicial del espacio
		}
		raiz= raiz->sgte;
	}
	return 0;
}

void unirBloquesLibres(void)
{//une dos nodos libres en uno solo mas grande
   espacioLibre* nodoABorrar= libreRaiz->sgte;
   libreRaiz->cantPag= libreRaiz->cantPag + nodoABorrar->cantPag;
   if(!(nodoABorrar->sgte))//solo son dos nodos
   {
	   libreRaiz->sgte= NULL;
	   free(nodoABorrar);
	   return;
   }
    nodoABorrar->sgte->ant= libreRaiz;
    libreRaiz->sgte= nodoABorrar->sgte;
    free(nodoABorrar);
    return;
}

void moverInformacion(int inicioDe, int cantPags, int inicioA)// puse unos -1 alguien que me confime que tiene sentido
{//intercambia lo escrito en el swap para cuando movemos un nodo ocupado al desfragmentar
	char buffer[cantPags * configuracion.TAMANIO_PAGINA];//creamos el buffer
	fseek(archivo, (inicioDe -1) * configuracion.TAMANIO_PAGINA, SEEK_SET);//vamos al inicio de ocupado
	fread(buffer, sizeof(char), strlen(buffer), archivo);//leemos
	fseek(archivo, (inicioA -1) * configuracion.TAMANIO_PAGINA, SEEK_SET);//vamos a libre
	fwrite(buffer, sizeof(char), strlen(buffer), archivo);//escribimos
	return;//en el "nuevo" libre ahora hay basura
}

int alcanzanPaginas(int cantPags)//0 no alcanzan, 1 alcanzan
{//nos dice si vale la pena desfragmentar (si hay paginas libres suficientes)
	espacioLibre* auxL=libreRaiz;
	int pagsTotales=0;
	while (auxL)
	{
		pagsTotales= pagsTotales + auxL->cantPag;//sumamos todas las paginas libres en pagsTotales
		auxL= auxL->sgte;
	}
	if(cantPags > pagsTotales) return 0;//si es mayor a las paginasTotales, nos vimo en disney
	return 1;
}

void desfragmentar(void)
{//movemos los nodos ocupados y juntamos los espacios libres en uno solo
	printf("se desfragmenta el archivo... \n");
	int sizeLibres=0;
	espacioLibre* auxLibre=libreRaiz;
	while(auxLibre)//contamos cuantos espacios libres hay
    {
        auxLibre=auxLibre->sgte;
        sizeLibres++;
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

int agregarOcupado(uint32_t pid, uint32_t cantPag, int comienzo)//LOS NODOS OCUPADOS SE APILAN SIN ORDEN
{//llega un proceso nuevo y le asignamos un nodo correspondiente
	espacioOcupado* aux= ocupadoRaiz;
	while(aux && aux->sgte)//vamos al ultimo nodo si hay una lista
	{
		aux= aux->sgte;
	}
	espacioOcupado* nuevo= malloc(sizeof(espacioOcupado));
	if(!nuevo)
	{
		printf("fallo el malloc para la lista de ocupados \n");
		log_error(log, "Fallo el malloc para la lista de ocupados");
		return 0;
	}
	nuevo->pid= pid;
	nuevo->cantPag= cantPag;
	nuevo->comienzo= comienzo;
	nuevo->sgte= NULL;
	nuevo->ant= aux;
	nuevo->escribio= 0;
	nuevo->leyo= 0;
	if(!aux)//si no habia nodos
	{
		ocupadoRaiz= nuevo;
		return 1;
	}
	aux->sgte= nuevo;//si habia
	return 1;
}

int asignarMemoria( uint32_t pid, uint32_t cantPag)
{//le damos memoria al proceso nuevo
	int inicio;
	inicio= hayEspacio(cantPag);
	if(!inicio)
	{
		if(alcanzanPaginas (cantPag)) //si las paginas libres totales alcanzan, que desfragmente
		{
			log_info(log, "Se inicia la compactacion del archivo");
			sleep(configuracion.RETARDO_COMPACTACION);//antes de compactar hacemos el sleep
			desfragmentar();
			log_info(log, "Finaliza la compactacion del archivo");
			inicio = hayEspacio(cantPag);
		}
		else
		{
			log_info(log, "Se rechaza el proceso de pid %u por no haber espacio suficiente", pid);
			printf("No hay espacio suficiente para el proceso de pid: %u\n", pid);
			return 0;
		}
	}
	int exito= agregarOcupado(pid, cantPag, inicio);
	log_info(log, "Se asignan %u bytes de memoria al proceso de pid %u desde el byte %u", cantPag*configuracion.TAMANIO_PAGINA, pid, inicio-1); //CANTIDAD DE BYTES ES NPAG*TMANIOPAGINA
	return exito;
}

int atras(espacioOcupado* nodo)//0 no hay nada 1 libre 2 ocupado
{//recibe un nodo ocupado y nos dice si la pagina de atras es libre u ocupada
	int comienzo = nodo->comienzo;
	espacioLibre* libreAux= libreRaiz;
	espacioOcupado* ocupadoAux= ocupadoRaiz;
	while(libreAux)
	{
		if(comienzo == libreAux->comienzo + libreAux->cantPag)
		{
			return 1;
		}
		libreAux= libreAux->sgte;
	}
	while(ocupadoAux)
	{
		if(comienzo == ocupadoAux->comienzo + ocupadoAux->cantPag)
		{
			return 2;
		}
		ocupadoAux= ocupadoAux->sgte;
	}
	return 0;
}

int adelante(espacioOcupado* nodo)// 0 no hay nada 1 libre 2 ocupado
{//recibe un nodo ocupado y nos dice si la pagina de adelante es libre u ocupada
	int comienzo = nodo->comienzo;
	int cantPag = nodo->cantPag;
	espacioLibre* libreAux= libreRaiz;
	espacioOcupado* ocupadoAux= ocupadoRaiz;
	while(libreAux)
	{
		if(comienzo + cantPag == libreAux->comienzo)
		{
			return 1;
		}
		libreAux= libreAux->sgte;
	}
	while(ocupadoAux)
	{
		if(comienzo + cantPag == ocupadoAux->comienzo)
		{
			return 2;
		}
		ocupadoAux= ocupadoAux->sgte;
	}
	return 0;
}

void borrarNodoOcupado(espacioOcupado* aBorrar)
{//se va un proceso y borramos su nodo
	if(aBorrar->ant)
	{
		aBorrar->ant->sgte=aBorrar->sgte;
	}
	if(aBorrar->sgte)
	{
		aBorrar->sgte->ant= aBorrar->ant;
	}
	if(aBorrar == ocupadoRaiz)
	{
		ocupadoRaiz= ocupadoRaiz->sgte;
		if(ocupadoRaiz)
		{
			ocupadoRaiz->ant= NULL;
		}
	}
	free(aBorrar);
	return;
}

int liberarMemoria(espacioOcupado* aBorrar)
{//se va un proceso y borramos su nodo ocupado y agregamos un libre
	int atrasVar= atras(aBorrar);
	int adelanteVar= adelante(aBorrar);
	log_info(log, "Se liberan %u bytes de memoria del proceso de pid %u desde el byte %u. El proceso leyo %u paginas y escribio %u.", (aBorrar->cantPag)*configuracion.TAMANIO_PAGINA, aBorrar->pid, (aBorrar->comienzo -1)*configuracion.TAMANIO_PAGINA, aBorrar->leyo, aBorrar->escribio);
	if(atrasVar==0 && adelanteVar==0)//el proceso ocupa el archivo entero
	{
		inicializarArchivo();
		libreRaiz=malloc(sizeof(espacioLibre));//creamos el primero nodo libre (que esTodo el archivo)
		if(!libreRaiz)
		{
			printf("fallo el malloc para la lista de libres en swap.c \n");
			log_error(log, "Fallo el malloc para la lista de libres");
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
		libreRaiz->cantPag= libreRaiz->cantPag + aBorrar->cantPag;
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
				log_error(log, "Fallo el malloc para la lista de libres");
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
				log_error(log, "Fallo el malloc para la lista de libres");
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
				log_error(log, "Fallo el malloc para la lista de libres");
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
				log_error(log, "Fallo el malloc para la lista de libres");
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
				log_error(log, "Fallo el malloc para la lista de libres");
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
				log_error(log, "Fallo el malloc para la lista de libres");
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
		nuevo=malloc(sizeof(espacioLibre));//ACA FALLARIA SEGUN VALGRIND
		if(!nuevo)
		{
			printf("fallo el malloc para la lista de libres en swap.c \n");
			log_error(log, "Fallo el malloc para la lista de libres");
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
		libreRaiz->sgte= nuevo;
		borrarNodoOcupado(aBorrar);
		return 1;
	}
	return 0;
}

char* leer(espacioOcupado* aLeer, uint32_t pagALeer)//pagALeer tiene como 0 a la pos inicial
{//leemos una pagina del archivo de swap
	char* buffer=NULL;
	buffer= calloc(configuracion.TAMANIO_PAGINA, 1);//es como el malloc pero inicializa en \0
	if(!buffer)
	{
		printf("Fallo la creacion del buffer en el swap funcion leer \n");
		log_error(log, "Fallo la creacion del buffer en el swap funcion leer");
		return buffer;
	}
	fseek(archivo,((aLeer->comienzo -1) * configuracion.TAMANIO_PAGINA) + (pagALeer * configuracion.TAMANIO_PAGINA), SEEK_SET);//vamos la pagina a leer (sin los menos uno la pasamos)
	fread(buffer, sizeof(char), (configuracion.TAMANIO_PAGINA)/sizeof(char), archivo);//leemos
	aLeer->leyo= aLeer->leyo +1;//aumentamos la cantidad de paginas leidas por el proceso
	log_info(log, "El proceso de pid %u lee %u bytes comenzando en el byte %u y leyo: %s", aLeer->pid, strlen(buffer)*sizeof(char), (aLeer->comienzo -1) * configuracion.TAMANIO_PAGINA +  pagALeer * configuracion.TAMANIO_PAGINA, buffer); //EN EL BYTE DESDE QUE COMIENZ AGREGO EL NUM PAG
	return buffer;
}

void escribir(espacioOcupado* aEscribir, uint32_t pagAEscribir, char* texto)// 0 mal 1 bien
{//escribimos en el archivo de swap
	fseek(archivo,((aEscribir->comienzo -1) * configuracion.TAMANIO_PAGINA) +  (pagAEscribir * configuracion.TAMANIO_PAGINA), SEEK_SET);
	fwrite(texto, sizeof(char), strlen(texto), archivo);
	aEscribir->escribio= aEscribir->escribio +1;//el proceso lee una pagina y lo documentamos
	log_info(log, "El proceso de pid %u escribe %u bytes comenzando en el byte %u y escribe %s", aEscribir->pid, strlen(texto)*sizeof(char), ((aEscribir->comienzo -1) * configuracion.TAMANIO_PAGINA) + (pagAEscribir * configuracion.TAMANIO_PAGINA), texto);
	return;
}

int interpretarMensaje(mensaje_ADM_SWAP mensaje,int socketcito)
{//depende de la instruccion tomamos acciones
	int resultado;
	mensaje_SWAP_ADM aEnviar;
	espacioOcupado* aBorrar;
	espacioOcupado*aEscribir;
	espacioOcupado* aLeer;
	switch (mensaje.instruccion)
	{
	case INICIAR:
		resultado=asignarMemoria(mensaje.pid, mensaje.parametro);
		aEnviar.contenidoPagina=NULL;
		if (!resultado)
		{
			aEnviar.estado=1;
			aEnviar.instruccion=mensaje.instruccion;
			int i= enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
			if(i==-1)//cambio esto, porque send devuelve -1 si no se pudo enviar.
			{
				printf("No se pudo enviar mensaje al ADM\n");
				log_error(log, "No se pudo enviar mensaje al ADM");
			}
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
			log_error(log, "El proceso no se encuentra en el swap");
			aEnviar.estado=1;
			aEnviar.instruccion=mensaje.instruccion;
			int i=enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
			if(i==-1)
			{
				printf("No se pudo enviar mensaje al ADM\n");
				log_error(log, "No se pudo enviar mensaje al ADM");
			}
			return 0;
		}
		printf("Se libero la memoria del proceso de pid %u \n", aBorrar->pid);
		liberarMemoria(aBorrar);
		aEnviar.contenidoPagina=NULL;
		break;

	case LEER:
		aLeer=ocupadoRaiz;
		while(aLeer && aLeer->pid != mensaje.pid) aLeer=aLeer->sgte;
		if(!aLeer)
		{
			printf ("El proceso no se encuentra en el swap \n");
			aEnviar.estado=1;
			aEnviar.instruccion=mensaje.instruccion;
			aEnviar.contenidoPagina=NULL;
			int i=enviarDeSwapAlADM(socketcito,&aEnviar,configuracion.TAMANIO_PAGINA);
			if(i==-1)
			{
				printf("No se pudo enviar mensaje al ADM\n");
				log_error(log, "No se pudo enviar mensaje al ADM");
			}
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
			if(i==-1)
			{
				printf("No se pudo enviar mensaje al ADM\n");
				log_error(log, "No se pudo enviar mensaje al ADM");
			}
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
	if(i==-1)
	{
		printf("No se pudo enviar mensaje al ADM\n");
		log_error(log, "No se pudo enviar mensaje al ADM");
	}
	if(aEnviar.contenidoPagina)
		{
			free(aEnviar.contenidoPagina);
			aEnviar.contenidoPagina=NULL;
		}
	if(mensaje.contenidoPagina)
		{
			free(mensaje.contenidoPagina);
			mensaje.contenidoPagina=NULL;
		}
	return 1;
}

void eliminarListas(void)
{//liberamos la memoria de las listas de ocupados y de libres
	if(libreRaiz)//si hay nodos libres
	{
		espacioLibre* siguiente= libreRaiz->sgte;
		while(siguiente)
		{
			free(libreRaiz);
			libreRaiz= siguiente;
			siguiente= siguiente->sgte;
		}
		free(libreRaiz);
		libreRaiz= NULL;
	}
	if(ocupadoRaiz)//si hay nodos ocupados
	{
		espacioOcupado* siguiente= ocupadoRaiz->sgte;
		while(siguiente)
		{
			free(ocupadoRaiz);
			ocupadoRaiz= siguiente;
			siguiente= siguiente->sgte;
		}
		free(ocupadoRaiz);
		ocupadoRaiz= NULL;
	}
	return;
}

int main()
{
	mensaje_ADM_SWAP mensaje;
	log= log_create(ARCHIVOLOG, "Swap", 0, LOG_LEVEL_INFO);
	log_info(log, "Proceso SWAP iniciado.");
	if(iniciarConfiguracion()==-1) return -1;
	printf("Iniciando Administrador de SWAP.. \n");
	printf("Estableciendo conexion.. \n");
	int socketEscucha;
	socketEscucha= crearSocketEscucha(10,configuracion.PUERTO_ESCUCHA);
	if(socketEscucha < 0)
	{
		printf("El socket al ADM en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP \n",configuracion.PUERTO_ESCUCHA);
		log_error(log,"El socket al ADM en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP",configuracion.PUERTO_ESCUCHA);
		return -1;
	}
	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP \n",configuracion.PUERTO_ESCUCHA);
		log_error(log,"El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de SWAP",configuracion.PUERTO_ESCUCHA);
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
	while (status)
	{
		status = recibirPaginaDeADM(socketADM,&mensaje,configuracion.TAMANIO_PAGINA);
		if(!status) break;
		printf("Recibi de ADM: Pid: %d Inst: %d Parametro: %d\n",mensaje.pid,mensaje.instruccion,mensaje.parametro);
		interpretarMensaje(mensaje,socketADM);
	}

	log_info(log, "Proceso SWAP finalizado.\n");
	close(socketADM);
	close(socketEscucha);
	fclose(archivo);
	eliminarListas();
	log_destroy(log);
	return 0;
}
