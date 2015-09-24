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

// typedef enum{LISTO, EJECUTANDO, BLOQUEADO }estado;
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
	  char*payload;
  } mensaje_CPU_PL;
  typedef struct mensaje_CPU_ADM_t
  {	uint32_t tipoInst; // 1 iniciar 2 leer 3 escribir 5 finalizar
	  uint32_t pid;
	 uint32_t parametro; // cant paginas, nunero pagina,
	  char* texto;
  }mensaje_CPU_ADM;
  typedef struct mensaje_ADM_CPU_T
  {
	  uint32_t parametro; // 0 ok 1 no ok
	  char*texto;
  }mensaje_ADM_CPU;
  typedef struct mensaje_ADM_SWAP_t
  {
	  uint32_t tipoInst; //1 iniciar 2 leer pagina 3 escribir pagina 5 finaliza
	  uint32_t pid;
	  char*texto;
  }mensaje_ADM_SWAP;
  typedef struct mensaje_SWAP_ADM_t
  {
	  int estado; //0 ok 1 no ok
	  char*texto;
  } mensaje_SWAP_ADM;
typedef struct retornoInstruccion_t
{
	uint32_t tipoInst; //1 iniciar 2 leer 3 escribir 4 e/s 5 finalizar
	uint32_t parametro; // 0 bien 1 mal , cant paginas, numero pagina, tiempo e/s, 0 bien 1 mal
	char*texto; //texto escrito
} retornoInstruccion;
typedef struct proceso_CPU_t
{
	uint32_t pid;
	uint32_t ip;
	char path[50+1];
} proceso_CPU;

typedef struct nodo_CPU_t
{
	struct nodo_Lista_CPU_t*sgte;
	struct nodo_Lista_CPU_t*ant;
	int id;
	int socket;
	pthread_t thread;
	nodoPCB *ejecutando;
} nodo_CPU;

#endif /* LIBRERIAS_SF_TIPOSDATO_H_ */
