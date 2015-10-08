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
#include <pthread.h>
#include <semaphore.h>
nodoPCB* ultimoNodoPCB(nodoPCB* raiz);
nodoPCB* crearNodoPCB(int pid,char path[]); // A partir de un pid y un path crea el nodo para la lista.
void agregarNodoPCB(nodoPCB** raiz,nodoPCB* nuevoPCB); // Agregar el nodo a una lista, hay que pasarle &raiz.
nodoPCB* sacarNodoPCB(nodoPCB**raiz);  // Saca el nodo de una lista y lo devuelve, hay que pasarle &raiz.
void almacenarEnListaRetornos(mensaje_ADM_CPU mensaje, proceso_CPU* datos_CPU, instruccion_t instruccion);
//funcion que almacena en la lista de retornos del CPU
uint32_t desempaquetarLista(retornoInstruccion* mensaje, nodo_Retorno_Instruccion* lista);
// funcion que pasa la lista a un array que sera casteado posteriormente, y retorna el tamaño del payload a enviar al PL
nodoPCB* buscarNodoPCB(nodoPCB*raiz,int pid); //Busca un PCB por su pid en una lista.
void eliminarListaPCB(nodoPCB**raiz); //Eliminar la lista haciendo los ree correspondientes.
nodo_Lista_CPU* ultimoNodoCPU(nodo_Lista_CPU* raiz); //Retorna el ultimo CPU en la lista;
void agregarCPU(pthread_mutex_t* mutexCPU,int cuentaCPU, int socket,nodo_Lista_CPU** raiz );
//Agregar un CPU a  la lista y ademas hace un signal para cpus libres.
nodo_Lista_CPU* buscarCPU(int id,nodo_Lista_CPU* raiz); // Retorna un CPU segun su Id
void eliminarCPU(int id, sem_t* SEMAFOROCPULIBRES,nodo_Lista_CPU** raiz);
// NOTA: NO HAC EL BLOCK DEL MUTEX DE LISTA DE CPU. Eliminar un cpu de la lista y si estaba ejecuntando imprime error en el proceso, ademas se encarga del wait de cpus libreas.
nodo_Lista_CPU* primerCPULibre(nodo_Lista_CPU* raiz);
//Retorna el Primer CPU libre
int cantidadCPUS(nodo_Lista_CPU* raiz); //Devuelve la cantidad de CPUS, hay que hgacerle el mutex afuera.
int socketCPUPosicion(nodo_Lista_CPU* raiz,int posicion); //Devuelve el socket en la posicion i de la lista, el mutex afuera.
nodo_Lista_CPU*CPUPosicion(nodo_Lista_CPU* raiz,int posicion);
void eliminarListaCPU(nodo_Lista_CPU**raiz); //Finaliza la lista de cpus con los free correspondientes.
void mostrarPCBS(nodoPCB*raizListos, nodoPCB*raizBloqueados,nodoPCB*siendoBloqueado,nodo_Lista_CPU*raizCPUS);
//Muestra lista de PCBS
#endif /* LIBRERIAS_SF_LISTAS_H_ */
