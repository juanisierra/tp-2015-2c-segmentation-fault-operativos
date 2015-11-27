#include <stdlib.h>
#include <errno.h>
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
#include <semaphore.h>
#include <librerias-sf/strings.h>
#include <librerias-sf/tiposDato.h>
#include <signal.h>
#include <commons/log.h>
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracion"
#define TAMANIOMAXIMOTEXTO 200
#define TAMANIOMAXIMOLINEA 200
#define ARCHIVOLOG "ADM.log"
config_ADM configuracion;
int aciertosTLB;
int fallosTLB;
int indiceTLB;
int indiceMarcos;
int indiceClockM; //es el indice de lectura actual en clock m para reemplazo de paginas
int zonaCritica;
int flushMemoria;
char* memoria;
nodoListaTP* raizTP;
tlb* TLB;
tMarco* tMarcos;
pthread_t hMPFlush;
pthread_mutex_t MUTEXTLB;
pthread_mutex_t MUTEXTM;
pthread_mutex_t MUTEXLP;
pthread_mutex_t MUTEXLOG;
t_log* log;
int socketSWAP;
int socketEscucha;
pthread_t hTasaAciertos;
int iniciarConfiguracion(config_ADM* configuracion)
{
	//printf("Cargando Configuracion..\n");
	(*configuracion) = cargarConfiguracionADM(RUTACONFIG);
	if(configuracion->estado==-1 || configuracion->estado==0)
	{
		printf("Cerrando ADM..\n");
		return -1;
	}
	if (configuracion->estado==1)
	{
		printf("Configuracion cargada correctamente: \n");
		printf("Puerto Escucha: %s\n",configuracion->PUERTO_ESCUCHA);
		printf("IP del SWAP: %s\n",configuracion->IP_SWAP);
		printf("Puerto del SWAP: %s\n",configuracion->PUERTO_SWAP);
		printf("Maximo de Marcos por Proceso: %d\n",configuracion->MAXIMO_MARCOS_POR_PROCESO);
		printf("Cantidad de Marcos: %d\n",configuracion->CANTIDAD_MARCOS);
		printf("Tamanio Marco: %d\n",configuracion->TAMANIO_MARCO);
		printf("Entradas TLB: %d\n",configuracion->ENTRADAS_TLB);
		printf("TLB Habilitada: %d\n",configuracion->TLB_HABILITADA);
		printf("Retardo Memoria: %d\n",configuracion->RETARDO_MEMORIA);
		printf("Algoritmo de Reemplazo %d\n\n",configuracion->ALGORITMO_REEMPLAZO);
		return 0;
	}
	return -1;
}

int iniciarTablas (void) // Si devuelve -1 hubo fallo al inicializar la tabla
{
	int i=0;
	int fallo=0;
	if(configuracion.TLB_HABILITADA==1)
	{
		TLB=malloc(sizeof(tlb)*(configuracion.ENTRADAS_TLB));
		if(TLB==NULL) fallo=-1;
		for(i=0;i<configuracion.ENTRADAS_TLB;i++)
		{
			TLB[i].pid=-1;
			TLB[i].indice=-1;
			TLB[i].nPag=0;
			TLB[i].numMarco=-1;
		}
	}
	else
	{
		TLB=NULL;
	}
	tMarcos=malloc(sizeof(tMarco)*(configuracion.CANTIDAD_MARCOS));
	if(tMarcos!=NULL)
	{
		for(i=0;i<(configuracion.CANTIDAD_MARCOS);i++) //Inicia los marcos con el tamanio de cada uno.
		{
			tMarcos[i].indice=-1; //Inicializamos todos los marcos como libres
			tMarcos[i].pid=-1;
		}
	}
	else
	{
		fallo=-1;
	}
	memoria=malloc((sizeof(char)*(configuracion.TAMANIO_MARCO))*configuracion.CANTIDAD_MARCOS);
	if(memoria==NULL) fallo =-1;
	raizTP=NULL;
	if(fallo==0) //printf("Tablas iniciadas\n");
	return fallo;
}
nodoListaTP* buscarProceso(int pid) //Retorna NULL si no lo encuentra
{
	nodoListaTP* aux;
	aux=raizTP;
	while(aux!=NULL)
	{
		if(aux->pid==pid)
		{
			return aux;
		}
		aux=aux->sgte;
	}

	return NULL;
}
void tlbFlush(void)
{
	int i=0;
	if(configuracion.TLB_HABILITADA==1){
		//pthread_mutex_lock(&MUTEXTLB);
	for(i=0;i<configuracion.ENTRADAS_TLB;i++)
			{
				TLB[i].pid=-1;
				TLB[i].indice=-1;
				TLB[i].nPag=0;
				TLB[i].numMarco=-1;
			}
	//pthread_mutex_unlock(&MUTEXTLB);
	}
	printf("TLB BORRADA\n");
	return;
}
int marcosOcupadosMP()
{
	int cuenta=0;
	int i;
	for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
	{
		if(tMarcos[i].indice!=-1) cuenta++;
	}
	return cuenta;
}
void MPFlush(void)
{	int i;
	nodoListaTP* aux;
	nodoListaTP*nodo;
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
	/*printf("1\n");
	pthread_mutex_lock(&MUTEXLP);
	printf("2\n");
	pthread_mutex_lock(&MUTEXTM);
	printf("3\n");
	pthread_mutex_lock(&MUTEXTLB);
	printf("4\n");
	pthread_mutex_lock(&MUTEXLOG);
	printf("5\n");
	*/
	for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
	{

			if(tMarcos[i].pid!=-1 && tMarcos[i].indice!=-1 && tMarcos[i].modif==1) //LA ENTRADA HAY QUE GUARDARLA EN SWAP PRIMERO
			{	log_info(log,"Se envia el contenido del marco %d al SWAP. (Pagina: %d || PID: %d)",i,tMarcos[i].nPag,tMarcos[i].pid);
				mensajeParaSWAP.pid=tMarcos[i].pid;
				nodo=buscarProceso(tMarcos[i].pid);
				nodo->cantPaginasAcc++;
				mensajeParaSWAP.instruccion=ESCRIBIR;
				mensajeParaSWAP.parametro=tMarcos[i].nPag;
				mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
				//strcpy(mensajeParaSWAP.contenidoPagina,tMarcos[i].contenido);
				memcpy(mensajeParaSWAP.contenidoPagina,&memoria[i*configuracion.TAMANIO_MARCO],configuracion.TAMANIO_MARCO);
				enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO); //MANDA LAPAGINA A ESCRIBIRSE
				if(mensajeParaSWAP.contenidoPagina!=NULL)
				{
					free(mensajeParaSWAP.contenidoPagina);
					mensajeParaSWAP.contenidoPagina=NULL;
				}
				recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
				if(mensajeDeSWAP.contenidoPagina!=NULL) free(mensajeDeSWAP.contenidoPagina);
				mensajeDeSWAP.contenidoPagina=NULL;
			}
	tMarcos[i].indice=-1; //libero Marcos
	tMarcos[i].modif=0;
	}
	if(configuracion.TLB_HABILITADA==1){ //BORRO LA TLB PUES SE BORRARON TODAS LAS PAGINSA EN MEMORIA
	for(i=0;i<configuracion.ENTRADAS_TLB;i++)
			{
				TLB[i].pid=-1;
				TLB[i].indice=-1;
				TLB[i].nPag=0;
				TLB[i].numMarco=-1;
			}
	}
	aux=raizTP;
	while(aux!=NULL)
	{
		for(i=0;i<aux->cantPaginas;i++)
		{
			(aux->tabla)[i].valido=0; //Pone todas las paginas como no disponibles en memoria
		}
		aux->marcosAsignados=0;
		aux=aux->sgte;
	}
	printf("MEMORIA PRINCIPAL BORRADA\n");
	/*pthread_mutex_unlock(&MUTEXLOG);
	pthread_mutex_unlock(&MUTEXLP);
	pthread_mutex_unlock(&MUTEXTM);
	pthread_mutex_unlock(&MUTEXTLB);
*/
	flushMemoria=0;
	return;

}
void logearTMarcos()
{	int i=0;
	pthread_mutex_lock(&MUTEXLOG);
	log_info(log,"\t\t\t\t******Estado de Tabla de Marcos*******");
			for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
			{
				if(tMarcos[i].indice!=-1 && tMarcos[i].pid!=-1)
				{
					if(configuracion.ALGORITMO_REEMPLAZO==0 || configuracion.ALGORITMO_REEMPLAZO==1){
				log_info(log,"Marco: %d \t\t PID: %d  \t\t Pagina: %d \t\t  Indice: %d",i,tMarcos[i].pid,tMarcos[i].nPag,tMarcos[i].indice);
					}
					if(configuracion.ALGORITMO_REEMPLAZO==2)
					{
						log_info(log,"Marco: %d \t\t PID: %d  \t\t Pagina: %d \t\t  Bit Uso: %d \t\t Bit Modificado: %d",i,tMarcos[i].pid,tMarcos[i].nPag,tMarcos[i].indice,tMarcos[i].modif);
					}
					}
			}
			log_info(log,"\t\t\t\t\t***********FIN************");
			pthread_mutex_unlock(&MUTEXLOG);
}
int marcosLibres(void) //CUENTA MRACOS LIBRES PARA ERROR DE MARCO
{
	int i=0;
	int c=0;
	for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
	{
		if(tMarcos[i].indice==-1) c++;
	}
	return c;
}
void atenderDump(void)
{	int pid;
	/*pthread_mutex_lock(&MUTEXLP);
	pthread_mutex_lock(&MUTEXTM);
	pthread_mutex_lock(&MUTEXTLB);
	pthread_mutex_lock(&MUTEXLOG);
	*/
	pid=fork();
	if(pid==-1)
	{
		printf("Fallo la creacion del hijo\n");
		return;
	}
	if(pid==0)//PROCESO HIJO
	{	int i;
	char * contenido;
	contenido = malloc((configuracion.TAMANIO_MARCO*sizeof(char))+1);
	log_info(log,"\t\t\t\t******DUMP DE MEMORIA*******");
	log_info(log,"\t\t\t\t Marcos Libres: %d   Marcos Ocupados: %d",configuracion.CANTIDAD_MARCOS-marcosOcupadosMP(),marcosOcupadosMP());
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
		{
			if(tMarcos[i].indice!=-1)
			{
				memcpy(contenido,&memoria[i*configuracion.TAMANIO_MARCO],configuracion.TAMANIO_MARCO);
				contenido[configuracion.TAMANIO_MARCO]='\0';
			log_info(log,"Marco: %d \t\t PID: %d  \t\t Pagina: %d \t\t  Contenido: %s",i,tMarcos[i].pid,tMarcos[i].nPag,contenido);
			}
		}
		log_info(log,"\t\t\t\t\t***********FIN************");
		free(contenido);
		exit(0);
	} else {

		wait(NULL); //ESPERA A LA FINALIZACION DEL HIJO
		/*pthread_mutex_unlock(&MUTEXLOG); //ESPERA A QUE TERMINE EL DUMP PARA SEGUIR
		pthread_mutex_unlock(&MUTEXLP);
		pthread_mutex_unlock(&MUTEXTM);
		pthread_mutex_unlock(&MUTEXTLB);
		*/
		printf("DUMP REALIZADO CON EXITO\n");
		return;
	}
}
void rutinaInterrupciones(int n) //La rutina que se dispaa con las interrupciones
{	pthread_t hTLBFlush;

	switch(n){
	case SIGUSR1:
	pthread_mutex_lock(&MUTEXLOG);
	log_info(log,"Se recibio senial de TLB FLUSH");
	pthread_mutex_unlock(&MUTEXLOG);
	pthread_create(&hTLBFlush,NULL,tlbFlush,NULL);
	pthread_join(hTLBFlush,NULL);
	break;
	case SIGUSR2:
		pthread_mutex_lock(&MUTEXLOG);
			log_info(log,"Se recibio senial de MPFlush");
			pthread_mutex_unlock(&MUTEXLOG);
	if(zonaCritica==1) {
			flushMemoria=1;
	}
	else {
		pthread_create(&hMPFlush,NULL,MPFlush,NULL);
			pthread_join(hMPFlush,NULL);
	}
	break;

	case SIGPOLL:
		pthread_mutex_lock(&MUTEXLOG);
				log_info(log,"Se recibio senial de Dump de MP");
				pthread_mutex_unlock(&MUTEXLOG);
		atenderDump();
	break;
}
}

int estaEnTLB(int pid, int numPag) //DEVUELVE -1 si no esta y sino devuelve la posicion en la tlb en la que esta
{
	int i;
	if(TLB!=NULL && configuracion.TLB_HABILITADA==1)
	{
		for(i=0;i<configuracion.ENTRADAS_TLB;i++)
		{
			if((TLB[i].pid)==pid && (TLB[i]).nPag==numPag)
			{
				return i;
			}
		}
		return -1;
	}
	return -1;
}

int entradaTLBAReemplazar(void) //Devuelve que entrada hay que reemplazar, si devuelve -1 es porqeu no hay tlb.
{
	int i=0;
	int posMenor=0;
	if(TLB!=NULL)
	{
		for(i=0;i<configuracion.ENTRADAS_TLB;i++)
		{
			if(TLB[i].indice==-1) return i;
			if(TLB[i].indice<=TLB[posMenor].indice) posMenor=i;
		}
		return posMenor;
	}
	return -1;
}

int entradaTMarcoAReemplazar(int pid) //FALTA IMPLEMENTACION PARA CLOCK M
{	nodoListaTP* nodo;
	nodo = buscarProceso(pid); //Para clock-m ver el indice necesito el dato del nodo en la lista tp
	int i=0;
	int j=0; //Solo lo uso para correr 2 veces el for, si no encuentra hay un error.
	int posMenor=0;
	if(configuracion.ALGORITMO_REEMPLAZO==0 || configuracion.ALGORITMO_REEMPLAZO==1)//FIFO Y LRU
	{
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
		{
			if(nodo->marcosAsignados<configuracion.MAXIMO_MARCOS_POR_PROCESO && tMarcos[i].indice==-1) return i; //SI SE LE PUEDEN DAR LIBRES Y HAY LE DA
			//REEMPLAZO LOCAL
			if(tMarcos[i].pid==pid && tMarcos[i].indice<=tMarcos[posMenor].indice) posMenor=i;
		}
		return posMenor;
	}




	if(configuracion.ALGORITMO_REEMPLAZO==2) //CLOCK-M
	{	if(nodo->marcosAsignados<configuracion.MAXIMO_MARCOS_POR_PROCESO) { //BUSCA QUE PUEDAN DARLE UN MARCO LIBRE
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++) //PRIMERO CHEQUEO LAS LIBRES
				{
					if(tMarcos[i].indice==-1) return i;
				}
	}
		//NO HAY LIBRES, CORRO EL PRIMER RECORRIDO, SIN CAMBIAR U BUSCO U=0(INDICE) y M=0 (MODIF)
		for(j=0;j<2;j++){ //SI NO ENCUENTRA EL ULTIMO LOOP ENTRA DENUEVO PARA HACER EL 1 y 2, si con 2 repeticiones no encuentra, hay un erro de impelemtnacion
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
		{
			if(nodo->indiceClockM>=configuracion.CANTIDAD_MARCOS) nodo->indiceClockM=0; //Devuelve el indice a 0
			if(tMarcos[nodo->indiceClockM].pid==pid && tMarcos[nodo->indiceClockM].indice==0 && tMarcos[nodo->indiceClockM].modif==0)
			{	nodo->indiceClockM++; //INCREMENTA PARA EL PROXIMO USO
				return (nodo->indiceClockM-1);
			}
			nodo->indiceClockM++;
		}
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++) //NO HAY LIBRES BUSCO U=0 M=1 si no vale eso pongo u en 0
				{
					if(nodo->indiceClockM>=configuracion.CANTIDAD_MARCOS) nodo->indiceClockM=0; //Devuelve el indice a 0
					if(tMarcos[nodo->indiceClockM].pid==pid && tMarcos[nodo->indiceClockM].indice==0 && tMarcos[nodo->indiceClockM].modif==1)
					{	nodo->indiceClockM++; //INCREMENTA PARA EL PROXIMO USO
						return (nodo->indiceClockM-1);
					}
					tMarcos[nodo->indiceClockM].indice=0;
					nodo->indiceClockM++;
				}
		}

	}
	if(configuracion.ALGORITMO_REEMPLAZO==2) printf("ERROR DE IMPLEMENTACION DE CLOCK M, no se encuentra la pagina\n");
	return -1;
}

nodoListaTP* ultimoNodoTP(void)
{
	nodoListaTP*aux;
	aux=raizTP;
	while(aux!=NULL && aux->sgte!=NULL) aux=aux->sgte;
	return aux;
}

void agregarProceso(int pid, int cantPaginas)
{ sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA
	int i;
	nodoListaTP* ultimoNodo=ultimoNodoTP();

	if(raizTP==NULL)
	{
		raizTP=malloc(sizeof(nodoListaTP));
		raizTP->ant=NULL;
		ultimoNodo=raizTP;
	}
	else
	{
		ultimoNodo->sgte=malloc(sizeof(nodoListaTP));
		ultimoNodo->sgte->ant=ultimoNodo;
		ultimoNodo=ultimoNodo->sgte;
	}
	ultimoNodo->sgte=NULL;
	ultimoNodo->cantPaginas=cantPaginas;
	ultimoNodo->pid=pid;
	ultimoNodo->marcosAsignados=0;
	ultimoNodo->cantFallosPag=0;
	ultimoNodo->cantPaginasAcc=0;
	ultimoNodo->indiceClockM=0;
	ultimoNodo->tabla=malloc(sizeof(tablaPag)*cantPaginas);
	for(i=0;i<cantPaginas;i++)
	{
		ultimoNodo->tabla[i].valido=0;
	}
	return;
}

void eliminarProceso(int pid)
{
	sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA
	int i;
	nodoListaTP* aEliminar;
	aEliminar=buscarProceso(pid);
	if(aEliminar!=NULL)
	{	if(aEliminar==raizTP)
		{
		raizTP=aEliminar->sgte;
		}
		if(aEliminar->sgte!=NULL) aEliminar->sgte->ant=aEliminar->ant;
		if(aEliminar->ant!=NULL) aEliminar->ant->sgte=aEliminar->sgte;
		if(aEliminar->tabla!=NULL) free(aEliminar->tabla);
		aEliminar->tabla=NULL;
		free(aEliminar);
		aEliminar=NULL;
	}
	for(i=0;i<configuracion.CANTIDAD_MARCOS;i++) //BORRA LOS MARCOS DEL PROCESO
	{
		if(tMarcos[i].indice!=-1 && tMarcos[i].pid==pid)
		{
			tMarcos[i].indice=-1;
			tMarcos[i].modif=0;
			tMarcos[i].pid=-1;
		}
	}
	if(configuracion.TLB_HABILITADA==1)
	{
		for(i=0;i<configuracion.ENTRADAS_TLB;i++) //BORRA LAS ENTRADAS DE LA TLB DEL PROCESO
		{
			if(TLB[i].indice!=-1 && TLB[i].pid==pid)
			{
				TLB[i].indice=-1;
				TLB[i].pid=-1;
			}
		}
	}
	return;
}

void finalizarListaTP(void)
{
	nodoListaTP* aux;
	aux=ultimoNodoTP();
	while(aux!=NULL && aux->ant!=NULL)
	{
		aux=aux->ant;
		if(aux->sgte->tabla!=NULL) free(aux->sgte->tabla);
		aux->sgte->tabla=NULL;
		free(aux->sgte);
		aux->sgte=NULL;
	}

	if(aux!=NULL)
	{
		if(aux==raizTP) raizTP=NULL;
		free(aux->tabla);
		aux->tabla=NULL;
		free(aux);
		aux=NULL;
	}
	return;
}

int estaEnMemoria(int pid,int nPag) //Retorna el numero de marco si esta en memoria, sino -1 y -2 si hubo error, NO USA MUTEX!!!
{
	nodoListaTP* nodo;
	tablaPag* tabla;
	nodo=buscarProceso(pid);
	if(nodo!=NULL) //ENCONTRO EL NODO
	{
		tabla=nodo->tabla;
		if(tabla[nPag].valido==1)
			{
			//printf("PAGINA YA EN MEMORIA\n");
			return tabla[nPag].numMarco;
			}
		if(tabla[nPag].valido==0) return -1;
	}
	return -2;
}

void finalizarTablas(void)
{
	int i=0;
	if(TLB!=NULL) free(TLB);
	TLB=NULL;
	if(tMarcos!=NULL)
	{
		free(tMarcos);
		tMarcos=NULL;
	}
	if(memoria!=NULL) {
		free(memoria);
		memoria=NULL;
	}
	if(raizTP!=NULL) finalizarListaTP();
	//printf("Tablas finalizadas\n");
}

void agregarATLB(int pid,int pagina,int marco)
{
	int aAgregar;
	aAgregar=entradaTLBAReemplazar();
	TLB[aAgregar].nPag=pagina;
	TLB[aAgregar].numMarco=marco;
	TLB[aAgregar].pid=pid;
	TLB[aAgregar].indice=indiceTLB;
	indiceTLB++;
}

int reemplazarMarco(int pid,int pagina) //REEMPLAZA EL MARCO QUE HAYA QUE SACAR Y PONE LOS DATOS DEL NUEVO
{
	sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
	nodoListaTP* nodoProcesoViejo;
	nodoListaTP*nodoProceso;
	nodoProceso=buscarProceso(pid);
	int aReemplazar;
	int entradaTLBVieja;
	pthread_mutex_lock(&MUTEXLOG);
	log_info(log,"El estado de los marcos antes del reemplazo es:");
	pthread_mutex_unlock(&MUTEXLOG);
	logearTMarcos();
	aReemplazar=entradaTMarcoAReemplazar(pid);
	pthread_mutex_lock(&MUTEXLOG);
	log_info(log,"Se va a reemplazar el marco N: %d",aReemplazar);
	pthread_mutex_unlock(&MUTEXLOG);
	if(tMarcos[aReemplazar].indice!=-1 && tMarcos[aReemplazar].pid!=-1 && tMarcos[aReemplazar].modif==1) //LA ENTRADA HAY QUE GUARDARLA EN SWAP PRIMERO
	{
		//printf("VOY A REEMPLAXAR MARCO MODIFICADO\n");
		pthread_mutex_lock(&MUTEXLOG);
		log_info(log,"El marco se encuentra modificado. Se envia a SWAP");
		pthread_mutex_unlock(&MUTEXLOG);
		nodoProceso->cantPaginasAcc++;
		mensajeParaSWAP.pid=tMarcos[aReemplazar].pid;
		mensajeParaSWAP.instruccion=ESCRIBIR;
		mensajeParaSWAP.parametro=tMarcos[aReemplazar].nPag;
		mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
		//strcpy(mensajeParaSWAP.contenidoPagina,tMarcos[aReemplazar].contenido);
		memcpy(mensajeParaSWAP.contenidoPagina,&memoria[aReemplazar*configuracion.TAMANIO_MARCO],configuracion.TAMANIO_MARCO);
		enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO); //MANDA LAPAGINA A ESCRIBIRSE
		if(mensajeParaSWAP.contenidoPagina!=NULL)
		{
			free(mensajeParaSWAP.contenidoPagina);
			mensajeParaSWAP.contenidoPagina=NULL;
		}
		recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
		if(mensajeDeSWAP.contenidoPagina!=NULL) free(mensajeDeSWAP.contenidoPagina);
		nodoProcesoViejo=buscarProceso(tMarcos[aReemplazar].pid);
		nodoProcesoViejo->marcosAsignados--;
		nodoProcesoViejo->tabla[tMarcos[aReemplazar].nPag].valido=0; //MARCA COMO INVALIDO SU MARCO
		if((entradaTLBVieja=estaEnTLB(tMarcos[aReemplazar].pid,tMarcos[aReemplazar].nPag))!=-1)
		{
			TLB[entradaTLBVieja].indice=-1; //BORRA SU ENTRADA EN LA TLB
		}
	}
	if(tMarcos[aReemplazar].indice!=-1 && tMarcos[aReemplazar].modif==0) //SI NO HAY QUE GUARDAR LA PAG PERO SI BORRAR OS DATOS
	{
		pthread_mutex_lock(&MUTEXLOG);
		log_info(log,"El marco reemplazado no habia sido modificado");
		pthread_mutex_unlock(&MUTEXLOG);
		nodoProcesoViejo=buscarProceso(tMarcos[aReemplazar].pid);
		nodoProcesoViejo->marcosAsignados--;
		nodoProcesoViejo->tabla[tMarcos[aReemplazar].nPag].valido=0; //MARCA COMO INVALIDO SU MARCO
		if((entradaTLBVieja=estaEnTLB(tMarcos[aReemplazar].pid,tMarcos[aReemplazar].nPag))!=-1)
		{
			TLB[entradaTLBVieja].indice=-1; //BORRA SU ENTRADA EN LA TLB
		}
	}
	//PEDIMOS PAGINA BUSCADA AL SWAP
	pthread_mutex_lock(&MUTEXLOG);
	log_info(log,"Se solicita al SWAP la pagina %d del proceso de PID: %d",pagina,pid);
	pthread_mutex_unlock(&MUTEXLOG);
	nodoProceso->cantPaginasAcc++;
	mensajeParaSWAP.pid=pid;
	mensajeParaSWAP.instruccion=LEER;
	mensajeParaSWAP.parametro=pagina;
	mensajeParaSWAP.contenidoPagina=NULL;
	enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
	if(mensajeParaSWAP.contenidoPagina!=NULL)
	{
		free(mensajeParaSWAP.contenidoPagina);
		mensajeParaSWAP.contenidoPagina=NULL;
	}
	recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);

	//printf("ASGINADOS: %d MARCO %d BYTE: %d\n",nodoProceso->marcosAsignados,aReemplazar,(aReemplazar*configuracion.TAMANIO_MARCO));
	memcpy(&memoria[aReemplazar*configuracion.TAMANIO_MARCO],mensajeDeSWAP.contenidoPagina,configuracion.TAMANIO_MARCO);
	if(mensajeDeSWAP.contenidoPagina!=NULL)
	{
		free(mensajeDeSWAP.contenidoPagina);
	}
	tMarcos[aReemplazar].modif=0;
	if(configuracion.ALGORITMO_REEMPLAZO==0 || configuracion.ALGORITMO_REEMPLAZO==1) //FIFO Y LRU
	{
		tMarcos[aReemplazar].indice=indiceMarcos;
			indiceMarcos++;
	}
	if(configuracion.ALGORITMO_REEMPLAZO==2)//CLOCK M
	{
		tMarcos[aReemplazar].indice=1; //USO INDICE COMO BIT U
	}
	tMarcos[aReemplazar].pid=pid;
	tMarcos[aReemplazar].nPag=pagina;
	nodoProceso->marcosAsignados++;
	(nodoProceso->tabla)[pagina].valido=1; //CARGAMOS LA PAGINA COMO VALIDA  ////***
	(nodoProceso->tabla)[pagina].numMarco=aReemplazar;
	pthread_mutex_lock(&MUTEXLOG);
	log_info(log,"El estado de los marcos luego del reemplazdo es:");
	pthread_mutex_unlock(&MUTEXLOG);
	logearTMarcos();
	return aReemplazar;
}

int ubicarPagina(int pid, int numPag) //RETORNA -4 SI NO PUEDE TENER MAS MARCOS, UBICA UNA PAGINA EN MEMORIA O LA PIDE AL SWAP
{
	nodoListaTP* nodo;
	nodo=buscarProceso(pid); //APUNTA AL NODO EN LA LISTA DE LA TABLAD E PAGINAS
	int aux;
	int ubicada = -1;
	if(configuracion.TLB_HABILITADA==1) //BUSCA EN LA TLB Y SE FIJA SI ESTA O NO ALLI
	{	if(estaEnTLB(pid,numPag)>=0) {
		ubicada= TLB[estaEnTLB(pid,numPag)].numMarco;
	} else {
		ubicada = -1;
	}
		if(ubicada>=0)
		{
			aciertosTLB++;
			pthread_mutex_lock(&MUTEXLOG);
			log_info(log,"TLB HIT. Numero de Marco: %d",ubicada);
			pthread_mutex_unlock(&MUTEXLOG);
			if(configuracion.ALGORITMO_REEMPLAZO==1)//AL LEER MODIFICA EN KRU
			{
				tMarcos[ubicada].indice=indiceMarcos;
				indiceMarcos++;
			}
			if(configuracion.ALGORITMO_REEMPLAZO==2) //EN CLOCKM SE PONE EL BIT U EN ESTE CASO INDICE EN 1 SI LEO PAGINA
			{
				tMarcos[ubicada].indice=1;
			}
			return ubicada;
		}
		if(ubicada==-1)
		{
			fallosTLB++;
			pthread_mutex_lock(&MUTEXLOG);
			log_info(log,"TLB MISS");
			pthread_mutex_unlock(&MUTEXLOG);
		}
	}
	ubicada=estaEnMemoria(pid,numPag); //VE SI ESTA CARGADA EN MEMORIA
	sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA
	if(ubicada>0)
	{
		if(configuracion.TLB_HABILITADA==1) agregarATLB(pid,numPag,ubicada);
		if(configuracion.ALGORITMO_REEMPLAZO==1) //AL LEER MODIFICA EN LRU
		{
			tMarcos[ubicada].indice=indiceMarcos;
			indiceMarcos++;
		}
		if(configuracion.ALGORITMO_REEMPLAZO==2) //EN CLOCKM SE PONE EL BIT U EN ESTE CASO INDICE EN 1
		{
			tMarcos[ubicada].indice=1;
		}
		return ubicada;
	}
	if(ubicada==-1) //HAY QUE TRAERLA DEL SWAP
	{
		if(nodo->marcosAsignados==0 && marcosLibres()==0) return -4; //NO TIENE MARCOS Y NO HAY PARA DARLE, ERROR DE MARCO
		nodo->cantFallosPag++;
		ubicada=reemplazarMarco(pid,numPag); //CAMBIA EL MARCO Y DEVUELVE EL NUMERO
		if(configuracion.TLB_HABILITADA==1)
		{
			agregarATLB(pid,numPag,ubicada); //AGREGAMOS A ENTRADA EN LA TLB
			pthread_mutex_lock(&MUTEXLOG);
			log_info(log,"Agregado a TLB en marco: %d",ubicada);
			pthread_mutex_unlock(&MUTEXLOG);
		}
	}
	return ubicada;
}

void TasaAciertos(void)
{
	int tasaAciertos;
	while(1)
	{
		sleep(60);
		if((aciertosTLB+fallosTLB)>0) {
		tasaAciertos=(aciertosTLB)*100/(aciertosTLB+fallosTLB);
		} else {
			tasaAciertos=0;
		}
		printf("La tasa de aciertos de la TLB es del %d %%\n",tasaAciertos);
	}
}

int main()
{	zonaCritica=0;
flushMemoria=0;
	log= log_create(ARCHIVOLOG, "ADM", 0, LOG_LEVEL_INFO);
	aciertosTLB=0;
	fallosTLB=0;
	indiceTLB=0;
	indiceMarcos=0;
	indiceClockM=0;
	struct sigaction sa;
	int cargadaEnMemoria=-1; //Lo usamos para ver si la pagina ya esta en memoria asi no se manda a swap aunque este modificada
	int marcoAUsar;
	nodoListaTP* nodo;
	pthread_mutex_init(&MUTEXTLB,NULL);
	pthread_mutex_init(&MUTEXTM,NULL);
	pthread_mutex_init(&MUTEXLP,NULL);
	pthread_mutex_init(&MUTEXLOG,NULL);
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	printf("Administrador de Memoria \n \n");
	log_info(log,"Iniciando Administrador de memoria..");
	socketEscucha= crearSocketEscucha(10,configuracion.PUERTO_ESCUCHA);
	if(socketEscucha < 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",configuracion.PUERTO_ESCUCHA);
		log_error(log,"El socket en el puerto %s no pudo ser creado, no se puede iniciar el ADM\n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}

	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",configuracion.PUERTO_ESCUCHA);
		log_error(log,"El socket en el puerto %s no pudo ser creado, no se puede iniciar el ADM\n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}

	printf("Creando Socket de conexion al SWAP en puerto %s \n",configuracion.PUERTO_SWAP);
	if((socketSWAP = crearSocketCliente(configuracion.IP_SWAP,configuracion.PUERTO_SWAP))<0)
		{
			printf("No se pudo crear socket de conexion al SWAP \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			log_error(log,"No se pudo crear el socket de conexion al SWAP\n");
			return 0;
		}
	if(iniciarTablas()==-1)
	{
		printf("Fallo la creacion de las tablas\n");
		log_error(log,"Fallo la creacion de las tablas\n");
		return -1;
	}
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones.. \n");
	log_info(log,"Esperando conexiones..");
	int socketCPU = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	printf("Conectado al CPU en el puerto %s \n",configuracion.PUERTO_ESCUCHA);
	log_info(log,"Conectado al CPU en el puerto %s",configuracion.PUERTO_ESCUCHA);
//EMPIEZA EJECUCION*********************************************************************
	if(configuracion.TLB_HABILITADA==1)
	{
		pthread_create(&hTasaAciertos,NULL,TasaAciertos,NULL);
	}

	signal(SIGUSR1,rutinaInterrupciones);
	signal(SIGUSR2,rutinaInterrupciones);
	signal(SIGPOLL,rutinaInterrupciones);
	/* sa.sa_handler = rutinaInterrupciones;
	    sigemptyset(&sa.sa_mask);
	    sa.sa_flags = SA_RESTART; // resetea las syscalls cortadas con el sigpoll
sigaction(SIGPOLL, &sa, NULL);
*/
	mensaje_CPU_ADM mensajeARecibir;
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
	mensaje_ADM_CPU mensajeAMandar;//es el mensaje que le mandaremos al CPU
	int status = 1;		// Estructura que manjea el status de los recieve.
	while(status!=0)
	{
		//printf("Marcos ocupados: %d\n",marcosOcupadosMP());
		status = recibirInstruccionDeCPU(socketCPU, &mensajeARecibir);
		zonaCritica=1;
		if(status==0) break;
		//printf("Recibo: Ins: %d Parametro: %d Pid: %d", mensajeARecibir.instruccion,mensajeARecibir.parametro,mensajeARecibir.pid);
		//if(mensajeARecibir.tamTexto!=0) printf("  Mensaje: %s\n",mensajeARecibir.texto);
		//if(mensajeARecibir.tamTexto==0) printf("\n");
		pthread_mutex_lock(&MUTEXLP);
		//printf("LOCKEO\n");
		pthread_mutex_lock(&MUTEXTM);
		pthread_mutex_lock(&MUTEXTLB);
		//printf("Recibi instruccion: %d",mensajeARecibir.instruccion);
		if(mensajeARecibir.instruccion == INICIAR)
		{
			mensajeParaSWAP.pid=mensajeARecibir.pid;
			mensajeParaSWAP.instruccion=INICIAR;
			mensajeParaSWAP.parametro=mensajeARecibir.parametro;
			mensajeParaSWAP.contenidoPagina=NULL;
			enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
			recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
			if(mensajeDeSWAP.estado==0)
			{
				agregarProceso(mensajeARecibir.pid,mensajeARecibir.parametro);
				pthread_mutex_lock(&MUTEXLOG);
				log_info(log,"Mproc %d iniciado. Cantidad de Paginas Solicitadas: %d",mensajeParaSWAP.pid,mensajeParaSWAP.parametro);
				pthread_mutex_unlock(&MUTEXLOG);
			}
			mensajeAMandar.parametro = mensajeDeSWAP.estado;
			mensajeAMandar.tamanoMensaje = 0;
			mensajeAMandar.texto = NULL;
			enviarInstruccionACPU(socketCPU, &mensajeAMandar);
		}
		if(mensajeARecibir.instruccion == LEER)
		{	pthread_mutex_lock(&MUTEXLOG);
			log_info(log,"Solicitud de lectura recibida. PID: %d  || N Pag: %d",mensajeARecibir.pid,mensajeARecibir.parametro);
			pthread_mutex_unlock(&MUTEXLOG);
			marcoAUsar=ubicarPagina(mensajeARecibir.pid,mensajeARecibir.parametro);
			if(marcoAUsar!=-4) //EL MARCO ESTA O PUEDEN DARLE UNO
			{
				sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA A LEER
				mensajeAMandar.parametro=mensajeARecibir.parametro;
				mensajeAMandar.tamanoMensaje=configuracion.TAMANIO_MARCO+1;
				mensajeAMandar.texto=malloc(configuracion.TAMANIO_MARCO+1);
				memcpy(mensajeAMandar.texto,&memoria[marcoAUsar*configuracion.TAMANIO_MARCO],configuracion.TAMANIO_MARCO);
				mensajeAMandar.texto[configuracion.TAMANIO_MARCO]='\0'; // PONE UN BARRA CERO POR LAS DUDAS AL FINAL
				enviarInstruccionACPU(socketCPU, &mensajeAMandar);
			}
			else
			{
				//SOPORTAR ERROR AL LEER POR MAXIMO DE MARCOS DEVUELVE -1
				printf("Error de marcos pid %d\n",mensajeARecibir.pid);
				log_error(log,"Error de marcos pid %d",mensajeARecibir.pid);
				mensajeParaSWAP.pid=mensajeARecibir.pid;
				mensajeParaSWAP.instruccion=FINALIZAR;
				mensajeParaSWAP.parametro=0;
				mensajeParaSWAP.contenidoPagina=NULL;
				enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
				recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
				eliminarProceso(mensajeARecibir.pid);
				mensajeAMandar.parametro =-1;
				mensajeAMandar.tamanoMensaje =0;
				mensajeAMandar.texto =NULL;
				enviarInstruccionACPU(socketCPU, &mensajeAMandar);
			}
		}
		if(mensajeARecibir.instruccion == ESCRIBIR)
		{	cargadaEnMemoria=-1;
		nodo=buscarProceso(mensajeARecibir.pid);
			pthread_mutex_lock(&MUTEXLOG);
			log_info(log,"Solicitud de escritura recibida. PID: %d  || N Pag: %d",mensajeARecibir.pid,mensajeARecibir.parametro);
			pthread_mutex_unlock(&MUTEXLOG);
			cargadaEnMemoria=estaEnMemoria(mensajeARecibir.pid,mensajeARecibir.parametro); //Veo si ya estaba en memoria asi no la mando a swap si esta modificada
			marcoAUsar=ubicarPagina(mensajeARecibir.pid,mensajeARecibir.parametro);
			if(marcoAUsar!=-4)
			{
				if(tMarcos[marcoAUsar].modif==1 && tMarcos[marcoAUsar].indice!=-1 && tMarcos[marcoAUsar].pid!=-1 && cargadaEnMemoria<0) // si no era de este proceso
				{	nodo->cantPaginasAcc++;
					mensajeParaSWAP.pid=tMarcos[marcoAUsar].pid;
					mensajeParaSWAP.instruccion=ESCRIBIR;
					mensajeParaSWAP.parametro=tMarcos[marcoAUsar].nPag;
					mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
					memcpy(mensajeParaSWAP.contenidoPagina,&memoria[marcoAUsar*configuracion.TAMANIO_MARCO],configuracion.TAMANIO_MARCO);
					enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO); //MANDA LAPAGINA A ESCRIBIRSE
					if(mensajeParaSWAP.contenidoPagina!=NULL)
					{
						free(mensajeParaSWAP.contenidoPagina);
						mensajeParaSWAP.contenidoPagina=NULL;
					}
					recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
					if(mensajeDeSWAP.contenidoPagina!=NULL) free(mensajeDeSWAP.contenidoPagina);
					if(mensajeARecibir.tamTexto<=configuracion.TAMANIO_MARCO)
					{
						memcpy(&memoria[marcoAUsar*configuracion.TAMANIO_MARCO],mensajeARecibir.texto,mensajeARecibir.tamTexto);
					}
					else
					{
						memcpy(&memoria[marcoAUsar*configuracion.TAMANIO_MARCO],mensajeARecibir.texto,configuracion.TAMANIO_MARCO);
					}
					tMarcos[marcoAUsar].modif=1;
					sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA A ESCRIBIR
				}
				if(tMarcos[marcoAUsar].modif==0 || cargadaEnMemoria>=0) // Era de este proceso o no estaba modificada
				{
					if(mensajeARecibir.tamTexto<=configuracion.TAMANIO_MARCO)
					{
						memcpy(&memoria[marcoAUsar*configuracion.TAMANIO_MARCO],mensajeARecibir.texto,mensajeARecibir.tamTexto);
					}
					else
					{
						memcpy(&memoria[marcoAUsar*configuracion.TAMANIO_MARCO],mensajeARecibir.texto,configuracion.TAMANIO_MARCO);
					}
					tMarcos[marcoAUsar].modif=1;
					sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA A ESCRIBIR
				}
				mensajeAMandar.parametro=mensajeARecibir.parametro;
				mensajeAMandar.tamanoMensaje =0;
				mensajeAMandar.texto =NULL;
				enviarInstruccionACPU(socketCPU, &mensajeAMandar);
			}
			else
			{
				printf("Error de marcos pid %d\n",mensajeARecibir.pid);
							log_error(log,"Error de marcos pid %d",mensajeARecibir.pid);
							mensajeParaSWAP.pid=mensajeARecibir.pid;
							mensajeParaSWAP.instruccion=FINALIZAR;
							mensajeParaSWAP.parametro=0;
							mensajeParaSWAP.contenidoPagina=NULL;
							enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
							recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
							eliminarProceso(mensajeARecibir.pid);
							mensajeAMandar.parametro =-1;
							mensajeAMandar.tamanoMensaje =0;
							mensajeAMandar.texto =NULL;
							enviarInstruccionACPU(socketCPU, &mensajeAMandar);
			}
		}
		if(mensajeARecibir.instruccion ==FINALIZAR)
		{
			mensajeParaSWAP.pid=mensajeARecibir.pid;
			mensajeParaSWAP.instruccion=FINALIZAR;
			mensajeParaSWAP.parametro=mensajeARecibir.parametro;
			mensajeParaSWAP.contenidoPagina=NULL;
			enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
			recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
			nodo=buscarProceso(mensajeARecibir.pid);
			pthread_mutex_lock(&MUTEXLOG);
			log_info(log,"Mproc %d Finalizado. Cantidad de Fallos de Pagina: %d  || Cantidad de Paginas Accedidas: %d.",mensajeParaSWAP.pid,nodo->cantFallosPag,nodo->cantPaginasAcc);
			pthread_mutex_unlock(&MUTEXLOG);
			eliminarProceso(mensajeARecibir.pid); //FINALIZA EL PROCESO EN LA LISTA DE TABLAS DE PAG Y DEMAS
			mensajeAMandar.parametro = mensajeDeSWAP.estado;
			mensajeAMandar.tamanoMensaje = 0;
			mensajeAMandar.texto = NULL;
			enviarInstruccionACPU(socketCPU, &mensajeAMandar);
		}
		if(mensajeAMandar.texto!=NULL) free(mensajeAMandar.texto);
		if(mensajeARecibir.texto!=NULL) free(mensajeARecibir.texto); ///////
		if(mensajeDeSWAP.contenidoPagina!=NULL) free(mensajeDeSWAP.contenidoPagina);
		if(mensajeParaSWAP.contenidoPagina!=NULL) free(mensajeParaSWAP.contenidoPagina);
		mensajeARecibir.texto=NULL;
		mensajeDeSWAP.contenidoPagina=NULL;
		mensajeParaSWAP.contenidoPagina=NULL;
		pthread_mutex_unlock(&MUTEXLP);
		//printf("LIBERO\n");
		pthread_mutex_unlock(&MUTEXTM);
		pthread_mutex_unlock(&MUTEXTLB);
		zonaCritica=0;
		if(flushMemoria==1){
			pthread_create(&hMPFlush,NULL,MPFlush,NULL);
			pthread_join(hMPFlush,NULL);
		}
	}
	if(configuracion.TLB_HABILITADA==1)
	{
		printf("La cantidad de aciertos de TLB fue: %d, y la de errores: %d\n",aciertosTLB,fallosTLB);
		pthread_mutex_lock(&MUTEXLOG);
		log_info(log,"La cantidad de aciertos de TLB fue: %d, y la de errores: %d",aciertosTLB,fallosTLB);
		pthread_mutex_unlock(&MUTEXLOG);
		//pthread_kill(hTasaAciertos,9);
		//pthread_join(hTasaAciertos,NULL);
	}
	log_info(log,"Proceso finalizado\n");
	finalizarTablas();
	log_destroy(log);
	close(socketCPU);
	close(socketEscucha);
	pthread_mutex_destroy(&MUTEXLOG);
	pthread_mutex_destroy(&MUTEXTLB);
	pthread_mutex_destroy(&MUTEXTM);
	pthread_mutex_destroy(&MUTEXLP);
	return 0;
}
