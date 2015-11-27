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
#include <sys/time.h>
typedef enum{INICIAR, LEER, ESCRIBIR, ES, FINALIZAR,ERROR,CERRAR}instruccion_t; //CERRAR es el mensaje para apagar.
typedef enum{LISTO, EJECUTANDO, BLOQUEADO , AFINALIZAR,INVALIDO, ERRORINICIO,ERRORMARCO,USOCPU}estado_t; //INVALIDO es si alguna instruccion no estaba bien.
  typedef struct pcb_t
    {
        uint32_t pid;
        char path[50 +1];
        uint32_t ip;
        estado_t estado;
        uint32_t bloqueo; // Tiempo de bloqueo
        struct timeval t_inicio; //HORA A LA QUE EL PROESO SE PONE EN LA COLA DE LISTOS
        struct timeval t_es; //HORA A LA QE HACE SU PRIMERA E/S, si esta en 0 no hizo ninguna
        double suma_t_cpu; //SUMA DE TIEMPOS DE CPU
        double t_espera; //SUMA DE TIEMPO ESPERANDO E/S
        struct timeval t_entrada_listo; //Hora en la que entra a la cola de listos
        struct timeval t_entrada_cpu; //Hora en la que entra a CPU
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
	  uint32_t ip; //EN USOCPU ES El ID DEL CPU
	  estado_t nuevoEstado;// Si dice FINALIZAR se debe borrar del PCB y ERROR es si una instruccion no era valida, tambien finaliza.
	  uint32_t tiempoBloqueo; //EN USOCPU es el porcentaje
	  uint32_t tamPayload; // Depende de la cantidad de instrucciones ejecutadas,
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
	  uint32_t parametro; // 0 ok 1 no ok, esos son los OK para iniciar, para leer o escribir ante el fallo de marco el error sera: si es OK el parametro valdra el numero de la pagina y si es NO OK el parametro valdra -1.
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
  {	instruccion_t instruccion;
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
	int uso; //Porcentaje de uso, en -1 no llega el primer log todavia
	int finalizar; //Si vale 1 el PID se cambia al ultimo al terminar la rafaga.
	nodoPCB *ejecutando;
} nodo_Lista_CPU;

typedef struct espacioLibre_t
{
	struct espacioLibre_t* sgte;
	struct espacioLibre_t* ant;
	uint32_t comienzo;
	uint32_t cantPag;
}espacioLibre;

typedef struct espacioOcupado_t
{
	struct espacioOcupado_t* sgte;
	struct espacioOcupado_t* ant;
	uint32_t comienzo;
	uint32_t cantPag;
	uint32_t pid;
	uint32_t leyo;
	uint32_t escribio;
}espacioOcupado;

typedef struct tablaPag_t
{
	int valido; //0 si no esta en memoria 1 si si
	int numMarco;//marco en el que se encuentra si valido vale 1
} tablaPag;
typedef struct nodoListaTP_t //Nodos de la lista de tablas de paginas del adm
{
	struct nodoListaTP_t* ant;
	struct nodoListaTP_t* sgte;
	int pid;
	int cantPaginas;
	int marcosAsignados; //Chequear si es menor al tamanio de marcos maximo.
	int cantPaginasAcc;
	int cantFallosPag;
	int indiceClockM;
	tablaPag* tabla; //aca va la tabla en si malloc(sizeof(tablaPag)*cantPaginas)
} nodoListaTP;

typedef struct tlb_t //Registros de la tlb
{	int indice; //Cada vez que se asigna una se le da un indice, el indice menor es el que se saca. (-1 si esta libre)
	int pid;
	int nPag;
	int numMarco;
} tlb;
typedef struct tMarco_t { //Registro de la tabla de marcos
	int pid;
	int nPag;
	int indice; //-1 si esta libre
	int modif; // 0 no, 1 si
} tMarco;
#endif /* LIBRERIAS_SF_TIPOSDATO_H_ */
