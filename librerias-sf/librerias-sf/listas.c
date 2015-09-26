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

void almacenarEnListaRetornos(mensaje_ADM_CPU mensaje, proceso_CPU* datos_CPU, instruccion_t instruccion)//funcion que almacena en la lista de retornos del CPU
{
	if(datos_CPU->listaRetornos == NULL)
	{
		datos_CPU->listaRetornos = malloc(sizeof(mensaje_ADM_CPU));
		datos_CPU->listaRetornos->info.instruccion = instruccion;
		datos_CPU->listaRetornos->info.parametro = mensaje.parametro;
		datos_CPU->listaRetornos->info.tamTexto = mensaje.tamanoMensaje;
		datos_CPU->listaRetornos->info.texto = strdup(mensaje.texto);
		datos_CPU->listaRetornos->sgte = NULL;
		datos_CPU->listaRetornos->ant = NULL;
	}
	else
	{
		nodo_Retorno_Instruccion* aux = datos_CPU->listaRetornos;
		while(aux != NULL) aux= aux->sgte;
		aux->sgte=malloc(sizeof(mensaje_ADM_CPU));
		aux->sgte->info.instruccion = instruccion;
		aux->sgte->info.parametro = mensaje.parametro;
		aux->sgte->info.tamTexto = mensaje.tamanoMensaje;
		aux->sgte->info.texto = strdup(mensaje.texto);
		aux->sgte->sgte = NULL;
		aux ->sgte->ant = aux;
	}
}

uint32_t desempaquetarLista(retornoInstruccion* mensaje, nodo_Retorno_Instruccion lista) // funcion que pasa la lista a un array que sera casteado posteriormente, y retorna el tamaño del payload a enviar al PL
{
	int j=0; // VARIABLE CONTADORA PARA IR LLENANDO LOS ESPACIOS DEL MENSAJE
	int i = 0;//VARIABLE UTILIZADA PARA CONTAR LOS NODOS
	nodo_Retorno_Instruccion aux; //auxiliar para recorrer la lista por primera vez
	aux = lista;
	while(aux != NULL)
	{
		aux = aux->sgte;
		i++;
	}
	mensaje = malloc(sizeof(retornoInstruccion)*i);
	for(; j < i; j++)
	{
		aux = lista;
		mensaje[j].instruccion = lista->info.instruccion;
		mensaje[j].parametro = lista->info.parametro;
		mensaje[j].tamTexto = lista->info.tamTexto;
		mensaje[j].texto = strdup(lista->info.texto);
		lista = lista->sgte; // pasamos a leer el siguiente
		free(aux->info.texto);//liberamos la memoria del texto
		free(aux);//liberamos el resto de la memoria de ese nodo
	}
	return (sizeof(retornoInstruccion)*i);
}
