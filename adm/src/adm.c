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
#include <librerias-sf/strings.h>
#include <librerias-sf/tiposDato.h>
#define TAMANOPAQUETE 4
#define RUTACONFIG "configuracion"
#define TAMANIOMAXIMOTEXTO 200
#define TAMANIOMAXIMOLINEA 200
#define RUTACONFIG "configuracion"


config_ADM configuracion;
int aciertosTLB;
int fallosTLB;
nodoListaTP* raizTP;
tlb* TLB;
tMarco* tMarcos;
int iniciarConfiguracion(config_ADM* configuracion)
{
	printf("Cargando Configuracion..\n");
	(*configuracion) = cargarConfiguracionADM(RUTACONFIG);
	if(configuracion->estado==-1 || configuracion->estado==0){
		printf("Cerrando ADM..\n");
		return -1;
	}
	if (configuracion->estado==1){
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
{	int i=0;
	int fallo=0;
if(configuracion.TLB_HABILITADA==1){
	TLB=malloc(sizeof(tlb)*(configuracion.ENTRADAS_TLB));
	if(TLB==NULL) fallo=-1;
	for(i=0;i<configuracion.ENTRADAS_TLB;i++) TLB[i].indice=-1;
} else {
	TLB=NULL;
}
	tMarcos=malloc(sizeof(tMarco)*(configuracion.CANTIDAD_MARCOS));
	if(tMarcos!=NULL) {
	for(i=0;i<(configuracion.CANTIDAD_MARCOS);i++) //Inicia los marcos con el tamanio de cada uno.
	{	tMarcos[i].indice=-1; //Inicializamos todos los marcos como libres
		tMarcos[i].contenido=malloc(configuracion.TAMANIO_MARCO);
		if (tMarcos[i].contenido==NULL) fallo=-1;
	}
	} else {
		fallo=-1;
	}
	raizTP=NULL;

	if(fallo==0) printf("Tablas iniciadas\n");
	return fallo;
}
int estaEnTLB(int pid, int numPag) //DEVUELVE -1 si no esta
{ 	int i;
	if(TLB!=NULL){
	for(i=0;i<configuracion.ENTRADAS_TLB;i++)
	{
		if(TLB[i].pid==pid && TLB[i].nPag==numPag) return i;
	}
}
return -1;
}
int entradaTLBAReemplazar(void) //Devuelve que entrada hay que reemplazar, si devuelve -1 es porqeu no hay tlb.
{	int i=0;
	int posMenor=0;
	if(TLB!=NULL) {
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
	{	if(tMarcos[i].indice==-1) return i;
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
{	int i;
	nodoListaTP* ultimoNodo=ultimoNodoTP();
	if(ultimoNodo==raizTP)
		{
		ultimoNodo=malloc(sizeof(nodoListaTP));
		ultimoNodo->ant=NULL;
		raizTP=ultimoNodo;
		} else {
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
{ nodoListaTP* aux;
	aux=raizTP;
	while(aux!=NULL && aux->sgte!=NULL)
	{	if(aux->pid==pid) return aux;
		aux=aux->sgte;
	}
	return NULL;
}
void eliminarProceso(int pid)
{	int i;
	nodoListaTP* aEliminar;
	aEliminar=buscarProceso(pid);
	if(aEliminar!=NULL)
	{
		if(aEliminar->sgte!=NULL) aEliminar->sgte->ant=aEliminar->ant;
		if(aEliminar->ant!=NULL) aEliminar->ant->sgte=aEliminar->sgte;
		if(aEliminar->tabla!=NULL) free(aEliminar->tabla);
		free(aEliminar);
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
{	nodoListaTP* aux;
	aux=ultimoNodoTP();
	while(aux!=NULL && aux->ant!=NULL)
	{
	aux=aux->ant;
	if(aux->sgte->tabla!=NULL) free(aux->sgte->tabla);
	free(aux->sgte);
	}
	if(aux!=NULL)
	{	if(aux==raizTP) raizTP=NULL;
		free(aux->tabla);
		free(aux);
	}
	return;
}
int estaEnMemoria(int pid,int nPag) //Retorna el numero de marco si esta en memoria, sino -1 y -2 si hubo error
{	nodoListaTP* nodo;
	tablaPag* tabla;
	nodo=buscarProceso(pid);
	if(nodo!=NULL) //ENCONTRO EL NODO
	{	tabla=nodo->tabla;
		if(tabla[nPag].valido==1) return tabla->numMarco;
		if(tabla[nPag].valido==0) return -1;
	}
	return -2;
}
void finalizarTablas(void) //FALTA AGREGAR LIBERAR LISTA TP
{	int i=0;
	if(TLB!=NULL) free(TLB);
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
	}
	if(raizTP!=NULL) finalizarListaTP();
	printf("Tablas finalizadas\n");
}
int main()
{

	if(iniciarConfiguracion(&configuracion)==-1) return -1;
	printf("Administrador de Memoria \nEstableciendo conexion.. \n");
	int socketSWAP;
	int socketEscucha;
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

	mensaje_CPU_ADM mensajeARecibir;
	mensaje_ADM_SWAP mensajeParaSWAP;
	mensaje_SWAP_ADM mensajeDeSWAP;
	mensaje_ADM_CPU mensajeAMandar;//es el mensaje que le mandaremos al CPU
	int status = 1;		// Estructura que manjea el status de los recieve.
	while(status!=0)
	{
	status = recibirInstruccionDeCPU(socketCPU, &mensajeARecibir);
	if(status==0) break;
	printf("Recibo: Ins: %d Parametro: %d Pid: %d", mensajeARecibir.instruccion,mensajeARecibir.parametro,mensajeARecibir.pid);
	if(mensajeARecibir.tamTexto!=0) printf("Mensaje: %s\n",mensajeARecibir.texto);
	if(mensajeARecibir.tamTexto==0) printf("\n");

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
	mensajeParaSWAP.pid=mensajeARecibir.pid;
		mensajeParaSWAP.instruccion=LEER;
		mensajeParaSWAP.parametro=mensajeARecibir.parametro;
		mensajeParaSWAP.contenidoPagina=NULL;
		enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
		recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
		mensajeAMandar.parametro=mensajeDeSWAP.estado;
		mensajeAMandar.tamanoMensaje = strlen(mensajeDeSWAP.contenidoPagina) +1;
		mensajeAMandar.texto=malloc(mensajeAMandar.tamanoMensaje);
		strcpy(mensajeAMandar.texto,mensajeDeSWAP.contenidoPagina); /// NO SIRVE CON LAS A PORUQE NO LLEVAN /0
		enviarInstruccionACPU(socketCPU, &mensajeAMandar);

	}
	if(mensajeARecibir.instruccion == ESCRIBIR)
	{
	mensajeParaSWAP.pid=mensajeARecibir.pid;
	mensajeParaSWAP.instruccion=ESCRIBIR;
		mensajeParaSWAP.parametro=mensajeARecibir.parametro;
		mensajeParaSWAP.contenidoPagina=malloc(configuracion.TAMANIO_MARCO);
		strcpy(mensajeParaSWAP.contenidoPagina,mensajeARecibir.texto);
		enviarDeADMParaSwap(socketSWAP,&mensajeParaSWAP,configuracion.TAMANIO_MARCO);
		recibirMensajeDeSwap(socketSWAP,&mensajeDeSWAP,configuracion.TAMANIO_MARCO);
		mensajeAMandar.parametro = mensajeDeSWAP.estado;
		mensajeAMandar.tamanoMensaje =0;
		mensajeAMandar.texto =NULL;
		enviarInstruccionACPU(socketCPU, &mensajeAMandar);
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
	}
	finalizarTablas();
	close(socketCPU);
	close(socketEscucha);
	return 0;
}
