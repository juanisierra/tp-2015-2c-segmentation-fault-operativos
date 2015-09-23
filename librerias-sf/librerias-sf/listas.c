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
	nuevoNodo->info.bloqueo=0;
	nuevoNodo->info.estado=1;
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

