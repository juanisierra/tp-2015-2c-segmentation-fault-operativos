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
nodoPCB* crearNodoPCB(int pid,char path[]); // A partir de un pid y un path crea el nodo para la lista.
void agregarNodoPCB(nodoPCB** raiz,nodoPCB* nuevoPCB); // Agregar el nodo a una lista, hay que pasarle &raiz.
nodoPCB* sacarNodoPCB(nodoPCB**raiz);  // Saca el nodo de una lista y lo devuelve, hay que pasarle &raiz.



#endif /* LIBRERIAS_SF_LISTAS_H_ */
