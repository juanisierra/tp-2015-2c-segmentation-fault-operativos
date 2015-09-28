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
					datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) + 1;
					datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				else//no ok
				{
					sprintf(mensajeAGuardar, "mProc %d - Fallo \n", datos_CPU->pid);
					datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) + 1;
					datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				}
				break;
			}
			case LEER:
			{
				sprintf(mensajeAGuardar, "mProc %d - Pagina %d leida: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) + 1;
				datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				break;
			}
			case ESCRIBIR:
			{
				sprintf(mensajeAGuardar, "mProc %d - Pagina %d escrita: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) + 1;
				datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				break;
			}
			case ES:
			{
				sprintf(mensajeAGuardar, "mProc %d en entrada-salida de tiempo %d \n", datos_CPU->pid, mensaje.parametro);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) + 1;
				datos_CPU->listaRetornos->info.texto = strdup(mensajeAGuardar);
				break;
			}
			case FINALIZAR:
			{
				sprintf(mensajeAGuardar, "mProc %d finalizado \n", datos_CPU->pid);
				datos_CPU->listaRetornos->info.tamTexto = strlen(mensajeAGuardar) + 1;
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
							aux->sgte->info.tamTexto = strlen(mensajeAGuardar) + 1;
							aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						else//no ok
						{
							sprintf(mensajeAGuardar, "mProc %d - Fallo \n", datos_CPU->pid);
							aux->sgte->info.tamTexto = strlen(mensajeAGuardar) + 1;
							aux->sgte->info.texto = strdup(mensajeAGuardar);
						}
						break;
					}
					case LEER:
					{
						sprintf(mensajeAGuardar, "mProc %d - Pagina %d leida: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) + 1;
						aux->sgte->info.texto = strdup(mensajeAGuardar);
						break;
					}
					case ESCRIBIR:
					{
						sprintf(mensajeAGuardar, "mProc %d - Pagina %d escrita: %s \n", datos_CPU->pid, mensaje.parametro, mensaje.texto);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) + 1;
						aux->sgte->info.texto = strdup(mensajeAGuardar);
						break;
					}
					case ES:
					{
						sprintf(mensajeAGuardar, "mProc %d en entrada-salida de tiempo %d \n", datos_CPU->pid, mensaje.parametro);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) + 1;
						aux->sgte->info.texto = strdup(mensajeAGuardar);
						break;
					}
					case FINALIZAR:
					{
						sprintf(mensajeAGuardar, "mProc %d finalizado \n", datos_CPU->pid);
						aux->sgte->info.tamTexto = strlen(mensajeAGuardar) + 1;
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
	nodo_Retorno_Instruccion* aux; //auxiliar para recorrer la lista por primera vez
	aux = lista;
	printf("\n %d \n", tamTextoFinal);
	while(aux != NULL)
	{
		tamTextoFinal += aux->info.tamTexto;
		aux = aux->sgte;
		i++;
	}
	printf("%d \n", tamTextoFinal);
	(*mensaje) = malloc(tamTextoFinal);
	for(; j < i; j++)
	{
		aux = lista;
		memcpy((*mensaje) + tamContado, lista->info.texto, lista->info.tamTexto);
		tamContado += aux->info.tamTexto;// Vamos sumando los tamaños para que el memcpy sepa donde poner el dato
		lista = lista->sgte;
		free(aux->info.texto);//liberamos la memoria del texto
		free(aux);//liberamos el resto de la memoria de ese nodo
	}
	return tamTextoFinal; // retorna el tamaño final del payload
}
