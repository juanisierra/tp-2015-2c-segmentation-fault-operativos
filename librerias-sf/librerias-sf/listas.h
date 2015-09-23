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

nodoPCB* ultimoNodo(nodoPCB* raiz);
nodoPCB* crearNodoPCB(pcb informacion); // A partir de una estructura de tipo pcb crea el nodo para la lista
void agregarNodoPCB(nodoPCB* raiz,nodoPCB* nuevoPCB); // Agregar el nodo a una lista
nodoPCB* sacarNodoPCB(nodoPCB*raiz); // Saca el nodo de una lista y lo devuelve



#endif /* LIBRERIAS_SF_LISTAS_H_ */
