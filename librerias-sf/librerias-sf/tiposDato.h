/*
 * tiposDato.h
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_TIPOSDATO_H_
#define LIBRERIAS_SF_TIPOSDATO_H_
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

  typedef struct pcb_t
    {
        uint32_t pid;
        char path[50 +1];
        uint32_t ip;
        uint32_t estado;
        uint32_t bloqueo;
    } pcb;
  typedef struct NodoPCB_t
      {
          pcb info;
          struct NodoPCB_t* sgte;
          struct NodoPCB_t* ant;
      } nodoPCB;
  typedef struct mensaje_PL_CPU_t
  {
	  uint32_t pid;
	  uint32_t ip;
	  char path[50 +1];
	  uint32_t quantum;
  } mensaje_PL_CPU;
  typedef struct mensaje_CPU_PL_t
  {
	  uint32_t ip;
	  uint32_t tamMensaje;
	  char*retornos;
  } mensaje_CPU_PL;
typedef struct proceso_CPU_t
{
	uint32_t pid;
	uint32_t ip;
	char path[50+1];
} proceso_CPU;

#endif /* LIBRERIAS_SF_TIPOSDATO_H_ */
