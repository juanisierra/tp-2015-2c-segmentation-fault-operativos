/*
 * config.h
 *
 *  Created on: 6/9/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_CONFIG_H_
#define LIBRERIAS_SF_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>


typedef struct {
	int estado;
	char PUERTO_ESCUCHA[5];
	int ALGORITMO_PLANIFICACION; //1 es FIFO y 2 es RR
	int QUANTUM;
} config_pl;

typedef struct {
	int estado;
	char PUERTO_PLANIFICADOR[5];
	char IP_PLANIFICADOR[17];
	char IP_MEMORIA[17];
	char PUERTO_MEMORIA[5];
	int CANTIDAD_HILOS;
	int RETARDO;
} config_CPU;

typedef struct {
	int estado;
	char PUERTO_ESCUCHA[5];
	char IP_SWAP[17];
	char PUERTO_SWAP[5];
	int MAXIMO_MARCOS_POR_PROCESO;
	int CANTIDAD_MARCOS;
	int TAMANIO_MARCO;
	int ENTRADAS_TLB;
	int TLB_HABILITADA; //0 -> NO   1-> SI
	int RETARDO_MEMORIA;
	int ALGORITMO_REEMPLAZO; //0 FIFO 1 LRU 2 CLOCK-M
} config_ADM;

typedef struct {
	int estado;
	char PUERTO_ESCUCHA[5];
	char NOMBRE_SWAP[20];
	int CANTIDAD_PAGINAS;
	int TAMANIO_PAGINA;
	int RETARDO_SWAP;
	int RETARDO_COMPACTACION;
} config_SWAP;
void separarCampoYValor(char*linea,char* campo, char* valor);
config_pl cargarConfiguracionPL(char * ruta);
config_CPU cargarConfiguracionCPU(char * ruta);
config_ADM cargarConfiguracionADM(char * ruta);
config_SWAP cargarConfiguracionSWAP(char * ruta);
#endif /* LIBRERIAS_SF_CONFIG_H_ */
