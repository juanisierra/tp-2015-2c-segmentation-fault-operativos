/*
 * listas.h
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_LISTAS_H_
#define LIBRERIAS_SF_LISTAS_H_
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "tiposDato.h"

nodoPCB* ultimoNodoPCB(nodoPCB* raiz);
nodoPCB* crearNodoPCB(int pid,char path[]); // A partir de un pid y un path crea el nodo para la lista.
void agregarNodoPCB(nodoPCB** raiz,nodoPCB* nuevoPCB); // Agregar el nodo a una lista, hay que pasarle &raiz.
nodoPCB* sacarNodoPCB(nodoPCB**raiz);  // Saca el nodo de una lista y lo devuelve, hay que pasarle &raiz.
void almacenarEnListaRetornos(mensaje_ADM_CPU mensaje, proceso_CPU* datos_CPU, instruccion_t instruccion);
//funcion que almacena en la lista de retornos del CPU
uint32_t desempaquetarLista(retornoInstruccion* mensaje, nodo_Retorno_Instruccion* lista);
// funcion que pasa la lista a un array que sera casteado posteriormente, y retorna el tamaño del payload a enviar al PL



#endif /* LIBRERIAS_SF_LISTAS_H_ */
