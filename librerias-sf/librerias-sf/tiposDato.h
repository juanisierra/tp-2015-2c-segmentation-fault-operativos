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
typedef enum{INICIAR, LEER, ESCRIBIR, ES, FINALIZAR,ERROR, CERRAR}instruccion_t; //CERRAR es el mensaje para apagar.
typedef enum{LISTO, EJECUTANDO, BLOQUEADO , AFINALIZAR,INVALIDO, ERRORINICIO}estado_t; //INVALIDO es si alguna instruccion no estaba bien.
  typedef struct pcb_t
    {
        uint32_t pid;
        char path[50 +1];
        uint32_t ip;
        estado_t estado;
        uint32_t bloqueo; // Tiempo de bloqueo
    } pcb;
  typedef struct NodoPCB_t
      {
          pcb info;
          struct NodoPCB_t* sgte;
          struct NodoPCB_t* ant;
      } nodoPCB;
  typedef struct mensaje_PL_CPU_t
  {
	  uint32_t pid; //PID=-256 indica que hay que apagar el sistema, se sigue el mensaje con CERRAR en los otros.
	  uint32_t ip;
	  char path[50 +1];
	  uint32_t quantum; // Si no hay quantum mandamos negativo
  } mensaje_PL_CPU;
  typedef struct mensaje_CPU_PL_t
  {
	  uint32_t ip;
	  estado_t nuevoEstado;// Si dice FINALIZAR se debe borrar del PCB y ERROR es si una instruccion no era valida, tambien finaliza.
	  uint32_t tiempoBloqueo;
	  uint32_t tamPayload; // Depende de la cantidad de instrucciones ejecutadas
	  char*payload;
  } mensaje_CPU_PL;
  typedef struct mensaje_CPU_ADM_t
  {	instruccion_t instruccion;
	  uint32_t pid;
	 uint32_t parametro; // cant paginas, nunero pagina,
	 uint32_t tamTexto;
	  char* texto;
  }mensaje_CPU_ADM;
  typedef struct mensaje_ADM_CPU_T
  {
	  uint32_t parametro; // 0 ok 1 no ok,   si leyo o escribio devolvera el numero de pagina
	  uint32_t tamanoMensaje;
	  char*texto;
  }mensaje_ADM_CPU;
  typedef struct mensaje_ADM_SWAP_t
  {
	  instruccion_t instruccion;
	  uint32_t pid;
	  uint32_t parametro; // cantidad de paginas a crear, pagina a leer o a escribir.
	  char*contenidoPagina; //Suponemos que el tamanio de la pagina ya lo sabe el swap y que se manda de a una pagina.
  }mensaje_ADM_SWAP;
  typedef struct mensaje_SWAP_ADM_t
  {
	  uint32_t estado; //0 ok 1 no ok
	  char*contenidoPagina;
  } mensaje_SWAP_ADM;
typedef struct retornoInstruccion_t //Se almacenan en una lista en CPU y luego van al planificador para ser logeados o impresos.
{
	instruccion_t instruccion;
	uint32_t parametro; // 0 bien 1 mal , cant paginas, numero pagina, tiempo e/s
	uint32_t tamTexto;
	char*texto; //texto escrito-****************************************** CAPAZ CONVIENE DARLE LONGITUD FIJA*****************
} retornoInstruccion;
typedef struct nodo_Retorno_Instruccion_t // Nodos de  la lista de retornos de instrucciones.
{
	struct nodo_Retorno_Instruccion_t* ant;
	struct nodo_Retorno_Instruccion_t* sgte;
	retornoInstruccion info;
}nodo_Retorno_Instruccion;
typedef struct proceso_CPU_t //Contiene las estructuras para cada hilo dentro del CPU, va en un array.
{
	uint32_t pid;
	uint32_t ip;
	char path[50+1];
	uint32_t id;//El ID del cpu
	int socket; //El socket en que se conecta a su manejador de CPU en el planificador.
	pthread_t thread;
	nodo_Retorno_Instruccion* listaRetornos;
}proceso_CPU;

typedef struct nodo_Lista_CPU_t //Cada uno contiene los datos para el manejador de CPU en el planificador
{
	struct nodo_Lista_CPU_t*sgte;
	struct nodo_Lista_CPU_t*ant;
	int id;
	int socket;
	int finalizar; //Si vale 1 el PID se cambia al ultimo al terminar la rafaga.
	pthread_mutex_t MUTEXCPU; //Para cambiar finalizar.
	pthread_t thread;
	nodoPCB *ejecutando;
} nodo_Lista_CPU;

#endif /* LIBRERIAS_SF_TIPOSDATO_H_ */
