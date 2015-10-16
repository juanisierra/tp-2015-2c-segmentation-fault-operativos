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
#include <semaphore.h>
#include <librerias-sf/strings.h>
#include <librerias-sf/tiposDato.h>
#include <signal.h>
#include <commons/log.h>
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracion"
#define TAMANIOMAXIMOTEXTO 200
#define TAMANIOMAXIMOLINEA 200
#define RUTACONFIG "configuracion"
#define ARCHIVOLOG "ADM.log"
config_ADM configuracion;
int aciertosTLB;
int fallosTLB;
int indiceTLB;
int indiceMarcos;
nodoListaTP* raizTP;
tlb* TLB;
tMarco* tMarcos;
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
	printf("Cargando Configuracion..\n");
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
			tMarcos[i].contenido=malloc(configuracion.TAMANIO_MARCO);
			if (tMarcos[i].contenido==NULL) fallo=-1;
		}
	}
	else
	{
		fallo=-1;
	}
	raizTP=NULL;
	if(fallo==0) printf("Tablas iniciadas\n");
	return fallo;
}
void tlbFlush(void)
{
	int i=0;
	if(configuracion.TLB_HABILITADA==1){
		pthread_mutex_lock(&MUTEXTLB);
	for(i=0;i<configuracion.ENTRADAS_TLB;i++)
			{
				TLB[i].pid=-1;
				TLB[i].indice=-1;
				TLB[i].nPag=0;
				TLB[i].numMarco=-1;
			}
	pthread_mutex_unlock(&MUTEXTLB);
	}
	printf("BORRE LA TLB\n");
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
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
	pthread_mutex_lock(&MUTEXLP);
	pthread_mutex_lock(&MUTEXTM);
	pthread_mutex_lock(&MUTEXTLB);
	for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
	{

			if(tMarcos[i].indice!=-1 && tMarcos[i].modif==1) //LA ENTRADA HAY QUE GUARDARLA EN SWAP PRIMERO
			{
				mensajeParaSWAP.pid=tMarcos[i].pid;
				mensajeParaSWAP.instruccion=ESCRIBIR;
				mensajeParaSWAP.parametro=tMarcos[i].nPag;
				mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
				strcpy(mensajeParaSWAP.contenidoPagina,tMarcos[i].contenido);
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
	printf("MEMPRINCIPAL BORRADA\n");
	pthread_mutex_unlock(&MUTEXLP);
	pthread_mutex_unlock(&MUTEXTM);
	pthread_mutex_unlock(&MUTEXTLB);
	return;

}
void atenderDump(void)
{	printf("POR ATENDER DUMP\n");
	int pid;
	pthread_mutex_lock(&MUTEXLOG);
	pthread_mutex_lock(&MUTEXLP);
	pthread_mutex_lock(&MUTEXTM);
	pthread_mutex_lock(&MUTEXTLB);
	pid=fork();
	if(pid==-1){
		printf("Fallo la creacion del hijo\n");
		return;
	}
	if(pid==0)//PROCESO HIJO
	{	int i;
	log_info(log,"\t\t\t\t******DUMP DE MEMORIA*******");
	log_info(log,"\t\t\t\t Marcos Libres: %d   Marcos Ocupados: %d",configuracion.CANTIDAD_MARCOS-marcosOcupadosMP(),marcosOcupadosMP());
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
		{
			if(tMarcos[i].indice!=-1)
			{
			log_info(log,"Marco: %d \t\t PID: %d  \t\t Pagina: %d \t\t  Contenido: %s",i,tMarcos[i].pid,tMarcos[i].nPag,tMarcos[i].contenido);
			}
		}
		log_info(log,"\t\t\t\t\t***********FIN************");

		exit(0);
	} else {

		wait(NULL); //ESPERA A LA FINALIZACION DEL HIJO
		pthread_mutex_unlock(&MUTEXLP);
		pthread_mutex_unlock(&MUTEXTM);
		pthread_mutex_unlock(&MUTEXTLB);
		pthread_mutex_unlock(&MUTEXLOG); //ESPERA A QUE TERMINE EL DUMP PARA SEGUIR
		return;
	}
}
void rutinaInterrupciones(int n) //La rutina que se dispaa con las interrupciones
{	pthread_t hTLBFlush;

	pthread_t hMPFlush;
	switch(n){
	case SIGUSR1:
	pthread_create(&hTLBFlush,NULL,tlbFlush,NULL);
	pthread_join(hTLBFlush,NULL);
	break;
	case SIGUSR2:
	pthread_create(&hMPFlush,NULL,MPFlush,NULL);
	pthread_join(hMPFlush,NULL);
	break;

	case SIGPOLL:
		atenderDump();
	break;
}
}

int estaEnTLB(int pid, int numPag) //DEVUELVE -1 si no esta
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

int entradaTMarcoAReemplazar(void) //FALTA IMPLEMENTACION PARA CLOCK M
{
	int i=0;
	int posMenor=0;
	if(configuracion.ALGORITMO_REEMPLAZO==0 || configuracion.ALGORITMO_REEMPLAZO==1)
	{
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
		{
			if(tMarcos[i].indice==-1) return i;
			if(tMarcos[i].indice<=tMarcos[posMenor].indice) posMenor=i;
		}
		return posMenor;
	}
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
	ultimoNodo->tabla=malloc(sizeof(tablaPag)*cantPaginas);
	for(i=0;i<cantPaginas;i++)
	{
		ultimoNodo->tabla[i].valido=0;
	}
	return;
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

void eliminarProceso(int pid)
{ sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA
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
		printf("TERMINO Y ELMINO EL PROCEO %d\n",aEliminar->pid);///////////////////////
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
{	sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA
	nodoListaTP* nodo;
	tablaPag* tabla;
	nodo=buscarProceso(pid);
	if(nodo!=NULL) //ENCONTRO EL NODO
	{
		tabla=nodo->tabla;
		if(tabla[nPag].valido==1)
			{
			printf("PAGINA YA EN MEMORIA\n");
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
		for(i=0;i<configuracion.CANTIDAD_MARCOS;i++)
		{
			if(tMarcos[i].contenido!=NULL)
				{
					free(tMarcos[i].contenido);
					tMarcos[i].contenido=NULL;
				}
		}
		free(tMarcos);
		tMarcos=NULL;
	}
	if(raizTP!=NULL) finalizarListaTP();
	printf("Tablas finalizadas\n");
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

int reemplazarMarco(int pid,int pagina)
{	sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
	nodoListaTP* nodoProcesoViejo;
	nodoListaTP*nodoProceso;
	nodoProceso=buscarProceso(pid);
	int aReemplazar;
	int entradaTLBVieja;
	aReemplazar=entradaTMarcoAReemplazar();
	if(tMarcos[aReemplazar].indice!=-1 && tMarcos[aReemplazar].modif==1) //LA ENTRADA HAY QUE GUARDARLA EN SWAP PRIMERO
	{			printf("VOY A REEMPLAXAR MARCO MODIFICADO\n");
				mensajeParaSWAP.pid=tMarcos[aReemplazar].pid;
				mensajeParaSWAP.instruccion=ESCRIBIR;
				mensajeParaSWAP.parametro=tMarcos[aReemplazar].nPag;
				mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
				strcpy(mensajeParaSWAP.contenidoPagina,tMarcos[aReemplazar].contenido);
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
	{	printf("VOY A REEMPLAXAR MARCO NO MODIFICADO\n");
		nodoProcesoViejo=buscarProceso(tMarcos[aReemplazar].pid);
		nodoProcesoViejo->marcosAsignados--;
		nodoProcesoViejo->tabla[tMarcos[aReemplazar].nPag].valido=0; //MARCA COMO INVALIDO SU MARCO
		if((entradaTLBVieja=estaEnTLB(tMarcos[aReemplazar].pid,tMarcos[aReemplazar].nPag))!=-1)
		{
			TLB[entradaTLBVieja].indice=-1; //BORRA SU ENTRADA EN LA TLB
		}
	}
	printf("USO MARCO LIBRE\n");
	//PEDIMOS PAGINA BUSCADA AL SWAP
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
	strcpy(tMarcos[aReemplazar].contenido,mensajeDeSWAP.contenidoPagina); ///ESCRIOBIMOS LA PAGINA
	if(mensajeDeSWAP.contenidoPagina!=NULL) free(mensajeDeSWAP.contenidoPagina);
	tMarcos[aReemplazar].modif=0;
	tMarcos[aReemplazar].indice=indiceMarcos;   //REVISAR PARA CLOCK-M
	indiceMarcos++;
	tMarcos[aReemplazar].pid=pid;
	tMarcos[aReemplazar].nPag=pagina;
	nodoProceso->marcosAsignados++;
	(nodoProceso->tabla)[pagina].valido=1; //CARGAMOS LA PAGINA COMO VALIDA  ////***
	(nodoProceso->tabla)[pagina].numMarco=aReemplazar;

	return aReemplazar;
}

int ubicarPagina(int pid, int numPag) //RETORNA -4 SI NO PUEDE TENER MAS MARCOS
{
	nodoListaTP* nodo;
	nodo=buscarProceso(pid); //APUNTA AL NODO EN LA LISTA DE LA TABLAD E PAGINAS
	int aux;
	int ubicada = -1;
	if(configuracion.TLB_HABILITADA==1) //BUSCA EN LA TLB Y SE FIJA SI ESTA O NO ALLI
	{

		ubicada= estaEnTLB(pid,numPag);

		if(ubicada>=0)
		{
			aciertosTLB++;

			nodo->cantPaginasAcc++;

			if(configuracion.ALGORITMO_REEMPLAZO==1)//AL LEER MODIFICA
			{
				tMarcos[ubicada].indice=indiceMarcos;
				indiceMarcos++;

			}
			return ubicada;
		}
		if(ubicada==-1) fallosTLB++;
	}
	ubicada=estaEnMemoria(pid,numPag); //VE SI ESTA CARGADA EN MEMORIA
	if(ubicada>0)
	{
		nodo->cantPaginasAcc++;


		if(configuracion.TLB_HABILITADA==1) agregarATLB(pid,numPag,ubicada);

		if(configuracion.ALGORITMO_REEMPLAZO==1) //AL LEER MODIFICA
		{

			tMarcos[ubicada].indice=indiceMarcos;
			indiceMarcos++;

		}
		return ubicada;
	}
	if(ubicada==-1) //HAY QUE TRAERLA DEL SWAP
	{
		if(nodo->marcosAsignados>=configuracion.MAXIMO_MARCOS_POR_PROCESO)
		{	aux=entradaTMarcoAReemplazar(); //Vemos cual seria la proxima que se reemplazaria
			if(tMarcos[aux].pid!=pid) return -4; //SI EL PROCESO NO PUEDE TENER MAS DEVUELVE ERROR, chequeo que la proxima a sacar no sea de este proceso.
		}

		nodo->cantFallosPag++;
		ubicada=reemplazarMarco(pid,numPag); //CAMBIA EL MARCO Y DEVUELVE EL NUMERO

		if(configuracion.TLB_HABILITADA==1) agregarATLB(pid,numPag,ubicada); //AGREGAMOS A ENTRADA EN LA TLB
	}
	return ubicada;
}
void TasaAciertos(void)
{	int tasaAciertos;
while(1)
	{
	sleep(60);
	tasaAciertos=(aciertosTLB)*100/(aciertosTLB+fallosTLB);
	printf("La tasa de aciertos de la TLB es del %d %%\n",tasaAciertos);
	}

}
int main()
{	log= log_create(ARCHIVOLOG, "ADM", 0, LOG_LEVEL_INFO);
	aciertosTLB=0;
	fallosTLB=0;
	indiceTLB=0;
	indiceMarcos=0;
	int marcoAUsar;
	pthread_mutex_init(&MUTEXTLB,NULL);
	pthread_mutex_init(&MUTEXTM,NULL);
	pthread_mutex_init(&MUTEXLP,NULL);
	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	printf("Administrador de Memoria \nEstableciendo conexion.. \n");
	socketEscucha= crearSocketEscucha(10,configuracion.PUERTO_ESCUCHA);
	if(socketEscucha < 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}

	if(listen(socketEscucha,10)< 0)
	{
		printf("El socket en el puerto %s no pudo ser creado, no se puede iniciar el Administrador de Memoria \n",configuracion.PUERTO_ESCUCHA);
		return -1;
	}

	printf("Creando Socket de conexion al SWAP en puerto %s \n",configuracion.PUERTO_SWAP);
	if((socketSWAP = crearSocketCliente(configuracion.IP_SWAP,configuracion.PUERTO_SWAP))<0)
		{
			printf("No se pudo crear socket de conexion al SWAP \n"); //AGREGAR SOPOTE PARA -2 SI NO SE CONECTA
			return 0;
		}
	if(iniciarTablas()==-1)
	{
		printf("Fallo la creacion de las tablas\n");
		return -1;
	}

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	printf("Esperando conexiones.. \n");
	int socketCPU = accept(socketEscucha, (struct sockaddr *) &addr, &addrlen);
	printf("Conectado al CPU en el puerto %s \n",configuracion.PUERTO_ESCUCHA);
//EMPIEZA EJECUCION*********************************************************************
	if(configuracion.TLB_HABILITADA==1)
	{
	pthread_create(&hTasaAciertos,NULL,TasaAciertos,NULL);
	}
	signal(SIGUSR1,rutinaInterrupciones);
	signal(SIGUSR2,rutinaInterrupciones);
	signal(SIGPOLL,rutinaInterrupciones);
	mensaje_CPU_ADM mensajeARecibir;
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
	mensaje_ADM_CPU mensajeAMandar;//es el mensaje que le mandaremos al CPU
	int status = 1;		// Estructura que manjea el status de los recieve.
	while(status!=0)
	{	printf("Marcos ocupados: %d\n",marcosOcupadosMP());
		status = recibirInstruccionDeCPU(socketCPU, &mensajeARecibir);
		if(status==0) break;
		printf("Recibo: Ins: %d Parametro: %d Pid: %d", mensajeARecibir.instruccion,mensajeARecibir.parametro,mensajeARecibir.pid);
		if(mensajeARecibir.tamTexto!=0) printf("Mensaje: %s\n",mensajeARecibir.texto);
		if(mensajeARecibir.tamTexto==0) printf("\n");
		pthread_mutex_lock(&MUTEXLP);
		pthread_mutex_lock(&MUTEXTM);
		pthread_mutex_lock(&MUTEXTLB);
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
			}
			mensajeAMandar.parametro = mensajeDeSWAP.estado;
			mensajeAMandar.tamanoMensaje = 0;
			mensajeAMandar.texto = NULL;
			enviarInstruccionACPU(socketCPU, &mensajeAMandar);
		}
		if(mensajeARecibir.instruccion == LEER)
		{
			marcoAUsar=ubicarPagina(mensajeARecibir.pid,mensajeARecibir.parametro);
			if(marcoAUsar!=-4) //EL MARCO ESTA O PUEDEN DARLE UNO
			{	sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA A LEER
				mensajeAMandar.parametro=mensajeARecibir.parametro;
				printf("EL CONTENIDO DE LA PAGINA ES %s \n",tMarcos[marcoAUsar].contenido);
				mensajeAMandar.tamanoMensaje = strlen(tMarcos[marcoAUsar].contenido) +1;
				mensajeAMandar.texto=malloc(mensajeAMandar.tamanoMensaje);
				strcpy(mensajeAMandar.texto,tMarcos[marcoAUsar].contenido); /// NO SIRVE CON LAS A PORUQE NO LLEVAN /0
				enviarInstruccionACPU(socketCPU, &mensajeAMandar);
			}
			else
			{
				//SOPORTAR ERROR AL LEER POR MAXIMO DE MARCOS DEVUELVE -1
				mensajeAMandar.parametro =-1;
				mensajeAMandar.tamanoMensaje =0;
				mensajeAMandar.texto =NULL;
				enviarInstruccionACPU(socketCPU, &mensajeAMandar);
			}

		}
		if(mensajeARecibir.instruccion == ESCRIBIR)
		{
			marcoAUsar=ubicarPagina(mensajeARecibir.pid,mensajeARecibir.parametro);
			if(marcoAUsar!=-4)
			{

				if(tMarcos[marcoAUsar].modif==1)
				{
					mensajeParaSWAP.pid=tMarcos[marcoAUsar].pid;
					mensajeParaSWAP.instruccion=ESCRIBIR;
					mensajeParaSWAP.parametro=tMarcos[marcoAUsar].nPag;
					mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
					strcpy(mensajeParaSWAP.contenidoPagina,tMarcos[marcoAUsar].contenido);
					enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO); //MANDA LAPAGINA A ESCRIBIRSE
					if(mensajeParaSWAP.contenidoPagina!=NULL)
						{
							free(mensajeParaSWAP.contenidoPagina);
							mensajeParaSWAP.contenidoPagina=NULL;
						}
					recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
					if(mensajeDeSWAP.contenidoPagina!=NULL) free(mensajeDeSWAP.contenidoPagina);
					strcpy(tMarcos[marcoAUsar].contenido,mensajeARecibir.texto);
					tMarcos[marcoAUsar].modif=1;
					sleep(configuracion.RETARDO_MEMORIA); //ESPERA PORQUE ENTRO A MEMORIA A ESCRIBIR
				}
				if(tMarcos[marcoAUsar].modif==0)
				{
				strcpy(tMarcos[marcoAUsar].contenido,mensajeARecibir.texto);
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
				//MANEJAR ERROR DE CANT MARCOS
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
		pthread_mutex_unlock(&MUTEXTM);
		pthread_mutex_unlock(&MUTEXTLB);
	}

	if(configuracion.TLB_HABILITADA==1)
		{printf("La cantidad de aciertos de TLB fue: %d, y la de errores: %d\n",aciertosTLB,fallosTLB);
	pthread_kill(hTasaAciertos,9);
	pthread_join(hTasaAciertos,NULL);
		}
	finalizarTablas();
	log_destroy(log);
	close(socketCPU);
	close(socketEscucha);
	pthread_mutex_destroy(&MUTEXTLB);
	pthread_mutex_destroy(&MUTEXTM);
	pthread_mutex_destroy(&MUTEXLP);
	return 0;
}
