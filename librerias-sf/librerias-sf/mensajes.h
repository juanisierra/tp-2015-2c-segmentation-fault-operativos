/*
 * mensajes.h
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_MENSAJES_H_
#define LIBRERIAS_SF_MENSAJES_H_
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
  typedef struct mensaje_PL_CPU_t
  {
	  uint32_t pid;
	  uint32_t ip;
	  char path[50 +1];
	  uint32_t quantum;
  } mensaje_PL_CPU;

#endif /* LIBRERIAS_SF_MENSAJES_H_ */
