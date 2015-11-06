/*
 * listas.c
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "tiposDato.h"
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
nodoPCB* ultimoNodoPCB(nodoPCB* raiz)
{
	nodoPCB* aux=raiz;
	if(aux!=NULL){
    while(aux->ant)
    {
        aux=aux->ant;
    }
	}
    return aux;
}
nodoPCB* crearNodoPCB(int pid,char path[])
{
	nodoPCB* nuevoNodo=malloc(sizeof(nodoPCB));
	nuevoNodo->sgte=NULL;
	nuevoNodo->ant=NULL;
	nuevoNodo->info.t_es.tv_sec=0;
	nuevoNodo->info.t_es.tv_usec=0;
	nuevoNodo->info.t_espera=0;
	gettimeofday(&(nuevoNodo->info.t_inicio),NULL);
	gettimeofday(&(nuevoNodo->info.t_entrada_listo),NULL);
	nuevoNodo->info.bloqueo=0;
	nuevoNodo->info.estado=LISTO;
	nuevoNodo->info.ip=0;
	nuevoNodo->info.pid=pid;
	strcpy(nuevoNodo->info.path,path);
	return nuevoNodo;
}
void agregarNodoPCB(nodoPCB** raiz,nodoPCB* nuevoPCB)
{
	nodoPCB* ultimo=ultimoNodoPCB(*raiz);
	if(ultimo!=NULL){
	ultimo->ant=nuevoPCB;
	nuevoPCB->sgte=ultimo;
	nuevoPCB->ant=NULL;
	} else {
		*raiz=nuevoPCB;
		nuevoPCB->sgte=ultimo;
		nuevoPCB->ant=NULL;
	}
	return;
}
nodoPCB* sacarNodoPCB(nodoPCB**raiz)
{	nodoPCB*sacado;
	if(*raiz==NULL){
		return NULL;
	}
	sacado=*raiz;
	if((*raiz)->ant!=NULL){
	(*raiz)->ant->sgte=NULL;
	(*raiz)=(*raiz)->ant;
	sacado->ant=NULL;
	sacado->sgte=NULL;
	}
	else
	{
		*raiz=NULL;


	}
	return sacado;
}
nodoPCB* buscarNodoPCB(nodoPCB*raiz,int pid)
{
	nodoPCB*indice;
	indice=raiz;
	while(indice!=NULL)
	{
		if(indice->info.pid==pid) return indice;
		indice=indice->ant;
	}
	return NULL;
}
void eliminarListaPCB(nodoPCB**raiz)
{
	nodoPCB* aux;
	aux=ultimoNodoPCB(*raiz);
	while(aux!=NULL && aux->sgte!=NULL)
	{
		aux=aux->sgte;
		free(aux->ant);
		aux->ant=NULL;
	}
	if(*raiz!=NULL) free(*raiz);
	(*raiz)=NULL;
	aux=NULL;
	return;
}
void almacenarEnListaRetornos(mensaje_ADM_CPU mensaje, proceso_CPU* datos_CPU, instruccion_t instruccion)//funcion que almacena en la lista de retornos del CPU
{
	char mensajeAGuardar[50+mensaje.tamanoMensaje];
	if(datos_CPU->listaRetornos == NULL)
	{
		datos_CPU->listaRetornos = malloc(sizeof(nodo_Retorno_Instruccion));
		datos_CPU->listaRetornos->info.instruccion = instruccion;
		datos_CPU->listaRetornos->info.parametro = mensaje.parametro;
		switch(instruccion)
		{
			case INICIAR:
			{
				if(mensaje.parametro == 0)//OK
				{
					sprintf(mensajeAGuardar, "mProc %d - Iniciado \n", datos_CPU->pid);
					datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
					datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				else//no ok
				{
					sprintf(mensajeAGuardar, "mProc %d - Fallo \n", datos_CPU->pid);
					datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
					datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				break;
			}
			case LEER:
			{
				if(mensaje.parametro == -1)//NO OK PARA LEER
				{
					sprintf(mensajeAGuardar, "mProc %d - Fallo de Marco \n", datos_CPU->pid);
					datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
					datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				else//ok
				{
				sprintf(mensajeAGuardar, "mProc %d - Pagina %d leida: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
				datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				break;
			}
			case ESCRIBIR:
			{
				if(mensaje.parametro == -1)//no ok para escribir
				{
					sprintf(mensajeAGuardar, "mProc %d - Fallo de Marco \n", datos_CPU->pid);
					datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
					datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				else//ok
				{
				sprintf(mensajeAGuardar, "mProc %d - Pagina %d escrita: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
				datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				break;
			}
			case ES:
			{
				sprintf(mensajeAGuardar, "mProc %d en entrada-salida de tiempo %d \n", datos_CPU->pid, mensaje.parametro);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
				datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				break;
			}
			case FINALIZAR:
			{
				sprintf(mensajeAGuardar, "mProc %d finalizado \n", datos_CPU->pid);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) ;
				datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				break;
			}
			case ERROR:
			{
				break;
			}
			case CERRAR:
			{
				break;
			}
		}
		datos_CPU->listaRetornos->sgte = NULL;
		datos_CPU->listaRetornos->ant = NULL;
	}
	else
	{
		nodo_Retorno_Instruccion* aux = datos_CPU->listaRetornos;
		while(aux->sgte != NULL) aux= aux->sgte;
		aux->sgte=malloc(sizeof(nodo_Retorno_Instruccion));
		aux->sgte->info.instruccion = instruccion;
		aux->sgte->info.parametro = mensaje.parametro;
		switch(instruccion)
				{
					case INICIAR:
					{
						if(mensaje.parametro == 0)//OK
						{
							sprintf(mensajeAGuardar, "mProc %d - Iniciado \n", datos_CPU->pid);
							aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
							aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						else//no ok
						{
							sprintf(mensajeAGuardar, "mProc %d - Fallo \n", datos_CPU->pid);
							aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
							aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						break;
					}
					case LEER:
					{
						if(mensaje.parametro == -1)//no ok para leer
						{
							sprintf(mensajeAGuardar, "mProc %d - Error de marco \n", datos_CPU->pid);
							aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
							aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						else//ok
						{
						sprintf(mensajeAGuardar, "mProc %d - Pagina %d leida: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
						aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						break;
					}
					case ESCRIBIR:
					{
						if(mensaje.parametro == -1)//no ok para escribir
						{
							sprintf(mensajeAGuardar, "mProc %d - Error de marco \n", datos_CPU->pid);
							aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
							aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						else//ok
						{
						sprintf(mensajeAGuardar, "mProc %d - Pagina %d escrita: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
						aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						break;
					}
					case ES:
					{
						sprintf(mensajeAGuardar, "mProc %d en entrada-salida de tiempo %d \n", datos_CPU->pid, mensaje.parametro);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
						aux->sgte->info.texto = strdup(mensajeAGuardar);
						break;
					}
					case FINALIZAR:
					{
						sprintf(mensajeAGuardar, "mProc %d finalizado \n", datos_CPU->pid);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) ;
						aux->sgte->info.texto = strdup(mensajeAGuardar);
						break;
					}
					case ERROR:
					{
						break;
					}
					case CERRAR:
					{
						break;
					}
				}
		aux->sgte->sgte = NULL;
		aux ->sgte->ant = aux;
	}

}

uint32_t desempaquetarLista(void** mensaje, nodo_Retorno_Instruccion* lista) // funcion que pasa la lista a un array que sera casteado posteriormente, y retorna el tamaño del payload a enviar al PL
{
	int j=0; // VARIABLE CONTADORA PARA IR LLENANDO LOS ESPACIOS DEL MENSAJE
	int i = 0;//VARIABLE UTILIZADA PARA CONTAR LOS NODOS
	uint32_t tamTextoFinal = 0;// Aca tengo el tamaño final de lo que voy a mandar
	int tamContado = 0; //Aca voy poniendo los tamaños y sumandolos, para el memcpy
	char* nulo;
	nulo = malloc(1);
	*nulo = '\0';
	nodo_Retorno_Instruccion* aux; //auxiliar para recorrer la lista por primera vez
	aux = lista;
	while(aux != NULL)
	{
		tamTextoFinal += aux->info.tamTexto;
		aux = aux->sgte;
		i++;
	}
	(*mensaje) = malloc(tamTextoFinal + 1);
	for(; j < i; j++)
	{
		aux = lista;
		memcpy((*mensaje) + tamContado, lista->info.texto, lista->info.tamTexto);
		tamContado += aux->info.tamTexto;// Vamos sumando los tamaños para que el memcpy sepa donde poner el dato
		lista = lista->sgte;
		free(aux->info.texto);//liberamos la memoria del texto
		free(aux);//liberamos el resto de la memoria de ese nodo
	}
	memcpy((*mensaje) + tamTextoFinal, nulo, 1); //agregamos el nulo al final
	free(nulo);//libero la memoria del nulo
	return tamTextoFinal + 1; // retorna el tamaño final del payload con el nulo incluido
}

nodo_Lista_CPU* ultimoNodoCPU(nodo_Lista_CPU* raiz)
{
	nodo_Lista_CPU* aux;
	aux=raiz;
    while(aux && aux->sgte)
    {
        aux=aux->sgte;
    }
    return aux;
}
void agregarCPU(pthread_mutex_t* mutexCPU,int cuentaCPU, int socket,nodo_Lista_CPU** raiz )
{
	nodo_Lista_CPU* ultimoNodo;
	nodo_Lista_CPU* aAgregar;
	aAgregar=malloc(sizeof(nodo_Lista_CPU));
	aAgregar->id=cuentaCPU;
	aAgregar->socket=socket;
	aAgregar->uso=-1;
	pthread_mutex_lock(mutexCPU);
	ultimoNodo=ultimoNodoCPU(*raiz);
	if(ultimoNodo!=NULL){
	ultimoNodo->sgte=aAgregar;
	} else {
		(*raiz)=aAgregar;
	}
	aAgregar->ant=ultimoNodo;
	aAgregar->sgte=NULL;
	aAgregar->ejecutando=NULL;
	aAgregar->finalizar=0;
	pthread_mutex_unlock(mutexCPU);
	//printf("Posteo de semaforoCPULIBRES\n");
	return;
}
nodo_Lista_CPU* buscarCPU(int id,nodo_Lista_CPU* raiz)
{
	nodo_Lista_CPU*aux;
	aux=raiz;
	while(aux!=NULL && aux->id!=id) aux=aux->sgte;
	return aux;
}
void eliminarCPU(int id, sem_t* SEMAFOROCPULIBRES,nodo_Lista_CPU** raiz)
{
	nodo_Lista_CPU*aBorrar;
	aBorrar=buscarCPU(id,*raiz);
	if(aBorrar!=NULL && aBorrar->ant!=NULL)
	{
	aBorrar->ant->sgte=aBorrar->sgte;
	}
	if(aBorrar!=NULL && aBorrar->sgte!=NULL)
	{
		aBorrar->sgte->ant=aBorrar->ant;
	}
	if(*raiz==aBorrar) (*raiz)=NULL;
	if(aBorrar->ejecutando==NULL)
	{
		sem_wait(SEMAFOROCPULIBRES); //Si ya estaba ejecutando no hace falta bajar los cpus libreas.
	}
	if(aBorrar->ejecutando!=NULL)
	{
		printf("El Proceso %d finaliza fallidamente debido a una falla en el CPU\n",aBorrar->ejecutando->info.pid);
		free(aBorrar->ejecutando);
	}
	if(aBorrar!=NULL) free(aBorrar);
}
nodo_Lista_CPU* primerCPULibre(nodo_Lista_CPU* raiz)
{	nodo_Lista_CPU* aux;
	aux=raiz;
	while(aux!=NULL && aux->ejecutando!=NULL) aux=aux->sgte;
	return aux;
}
int cantidadCPUS(nodo_Lista_CPU* raiz)
{	int i=1;
	nodo_Lista_CPU*aux;
	aux=raiz;
	if(aux==NULL) return 0;
	while(aux->sgte!=NULL)
	{
		i++;
		aux=aux->sgte;
	}
	return i;
}
int socketCPUPosicion(nodo_Lista_CPU* raiz,int posicion)
{	int i;
	nodo_Lista_CPU* aux;
	aux=raiz;
	for(i=0; i<posicion;i++) aux=aux->sgte;
	return aux->socket;
}
nodo_Lista_CPU*CPUPosicion(nodo_Lista_CPU* raiz,int posicion)
{	int i;
	nodo_Lista_CPU* aux;
	aux=raiz;
	for(i=0; i<posicion;i++) aux=aux->sgte;
	return aux;
}
void eliminarListaCPU(nodo_Lista_CPU**raiz)
{
	nodo_Lista_CPU* aux;
	//printf("Entrando a ultimonodoCPU\n");
	aux=ultimoNodoCPU(*raiz);
	while(aux!=NULL && aux->ant!=NULL)
	{	//printf("POR ELIMINAR CPU\n");
		aux=aux->ant;
		if(aux->sgte->ejecutando!=NULL)
		{
		free(aux->sgte->ejecutando);
		aux->sgte->ejecutando=NULL;
		}
		close(aux->sgte->socket);
		free(aux->sgte);
		aux->sgte=NULL;
	}
	if((*raiz)!=NULL)
	{	close((*raiz)->socket);
		if((*raiz)->ejecutando!=NULL) free((*raiz)->ejecutando);
		free(*raiz);
	}
	(*raiz)=NULL;
	aux=NULL;
	return;
}
void mostrarPCBS(nodoPCB*raizListos, nodoPCB*raizBloqueados,nodoPCB*siendoBloqueado,nodo_Lista_CPU*raizCPUS)
{
	nodoPCB*auxPCB;
	nodo_Lista_CPU*CPUAux;
	int i;
	int cuenta=0;
	if(raizListos!=NULL)
	{	auxPCB=raizListos;
		while(auxPCB!=NULL && auxPCB->ant!=NULL)
		{
			printf("mProc %d: %s -> Listo\n",auxPCB->info.pid,auxPCB->info.path);
			auxPCB=auxPCB->ant;
			cuenta++;
		}
		if(auxPCB!=NULL)
			{
			printf("mProc %d: %s -> Listo\n",auxPCB->info.pid,auxPCB->info.path);
			cuenta++;
			}
	}
	if(raizBloqueados!=NULL)
		{	auxPCB=raizBloqueados;
			while(auxPCB!=NULL && auxPCB->ant!=NULL)
			{
				printf("mProc %d: %s -> Bloqueado\n",auxPCB->info.pid,auxPCB->info.path);
				auxPCB=auxPCB->ant;
				cuenta++;
			}
			if(auxPCB!=NULL)
				{
				printf("mProc %d: %s -> Bloqueado\n",auxPCB->info.pid,auxPCB->info.path);
				cuenta++;
				}
		}
	if(siendoBloqueado!=NULL)
		{
		printf("mProc %d: %s -> Bloqueado\n",siendoBloqueado->info.pid,siendoBloqueado->info.path);
		cuenta++;
		}
	for(i=0;i<cantidadCPUS(raizCPUS);i++)
		{
			CPUAux=CPUPosicion(raizCPUS,i);
			if( CPUAux->ejecutando!=NULL)
			{
				printf("mProc %d: %s -> Ejecutando\n",CPUAux->ejecutando->info.pid,CPUAux->ejecutando->info.path);
				cuenta++;
			}

		}
	if(cuenta==0) printf("No hay procesos en el sistema\n");
	return;
}
