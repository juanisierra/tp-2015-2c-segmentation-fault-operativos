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

nodoPCB* ultimoNodo(nodoPCB* raiz)
{
	nodoPCB* aux=raiz;
    while(aux->sgte)
    {
        aux=aux->sgte;
    }
    return aux;
}

int agregarNodoPCB(nodoPCB* raiz, pcb nuevoPcb)
{
    if(!raiz)
    {
    	raiz= malloc(sizeof(nodoPCB));
    	if(!raiz) return -1;
    	raiz->info=nuevoPcb;
        raiz->ant=NULL;
        raiz->sgte=NULL;
        return 0;
    }
    else
    {
    	nodoPCB* ultimo_p;
        ultimo_p=ultimoNodo(raiz);
        nodoPCB* aMeter=malloc(sizeof(nodoPCB));
        if(!aMeter) return -1;
        aMeter->info=nuevoPcb;
        aMeter->ant=ultimo_p;
        aMeter->sgte=NULL;
        ultimo_p->sgte=(aMeter);
        return 0;
    }
}


pcb sacarNodoPCB(nodoPCB*raiz)
{
  nodoPCB* aux;
  aux=raiz;
  pcb info;
  info=raiz->info;
  raiz->sgte->ant=NULL;  //si no anda es esto
  raiz=raiz->sgte;
  free(aux);
  return info;
}

