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
int agregarNodoPCB(nodoPCB* raiz, pcb nuevoPcb);
pcb sacarNodoPCB(nodoPCB*raiz);


#endif /* LIBRERIAS_SF_LISTAS_H_ */
