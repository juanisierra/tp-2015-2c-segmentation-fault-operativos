/*
 * config.c
 *
 *  Created on: 6/9/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"

/* Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * Retorna -1 si el algoritmo esta mal especificado y lo dice por pantalla
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_pl cargarConfiguracionPL(char * ruta) {

	config_pl config;
	FILE * archivoConfiguracion;
	char linea[50];
	char caracter;
	int i=0;
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s \n",ruta);
		config.estado=0;
	return config;
	}
//Cargar Puerto ESCUCHA************************************************
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		config.PUERTO_ESCUCHA[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	config.PUERTO_ESCUCHA[i]=0;

	//Cargar Algoritmo Planificacion***************************************
	i=0;
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		linea[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	linea[i]=0;
	if(strcmp(linea,"FIFO")==0){
		config.ALGORITMO_PLANIFICACION=1;
	}
	if(strcmp(linea,"RR")==0){
			config.ALGORITMO_PLANIFICACION=2;
	}
	if(strcmp(linea,"FIFO")&& strcmp(linea,"RR")){
		config.ALGORITMO_PLANIFICACION=0;
		printf("El algoritmo de Planificacion no está correctamente configurado \n");
		config.estado=-1;
		config.QUANTUM=0;
		fclose(archivoConfiguracion);
		return config;
	}

//Setea el Quantum********************************************************
	i=0;
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){/* Retorna 0 en estado si no se pudo abrir el archivo de configuracion
	 * Retorna 1 si se cargo correctamente
	 * Retorna -1 si el algoritmo esta mal especificado y lo dice por pantalla
	 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
	 */
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		linea[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	linea[i]=0;
	config.QUANTUM = atoi(linea);
	config.estado=1;
	fclose(archivoConfiguracion);
	return config;
}

/* Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_CPU cargarConfiguracionCPU(char * ruta) {
	config_CPU config;
	FILE * archivoConfiguracion;
	char linea[50];
	char caracter;
	int i=0;
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s\n",ruta);
		config.estado=0;
	return config;
	}
//Cargar PUERTO PLANIFICADOR************************************************
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		config.PUERTO_PLANIFICADOR[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	config.PUERTO_PLANIFICADOR[i]=0;

//Cargar IP PLANIFICADOR************************************************
		i=0;
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
		caracter=fgetc(archivoConfiguracion);
		}
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
			config.IP_PLANIFICADOR[i]=caracter;
			caracter=fgetc(archivoConfiguracion);
		i++;
		}
		config.IP_PLANIFICADOR[i]=0;
//Cargar IP MEMORIA************************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
				caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					config.IP_MEMORIA[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
				i++;
				}
				config.IP_MEMORIA[i]=0;
//Cargar PUERTO MEMORIA************************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					config.PUERTO_MEMORIA[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
					i++;
				}
				config.PUERTO_MEMORIA[i]=0;

//CARGAR CANTIDAD DE HILOS********************************************************
	i=0;
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		linea[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	linea[i]=0;
	config.CANTIDAD_HILOS = atoi(linea);

//CARGAR RETARDO********************************************************
		i=0;
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
		caracter=fgetc(archivoConfiguracion);
		}
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
			linea[i]=caracter;
			caracter=fgetc(archivoConfiguracion);
		i++;
		}
		linea[i]=0;
		config.RETARDO = atoi(linea);

	config.estado=1;
	fclose(archivoConfiguracion);
	return config;
}


/* Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * Retorna -1 si se escribio mal en TLB habilitada
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_ADM cargarConfiguracionADM(char * ruta) {
	config_ADM config;
	FILE * archivoConfiguracion;
	char linea[50];
	char caracter;
	int i=0;
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s\n",ruta);
		config.estado=0;
	return config;
	}
//Cargar PUERTO ESCUCHA************************************************
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		config.PUERTO_ESCUCHA[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	config.PUERTO_ESCUCHA[i]=0;

//Cargar IP SWAP************************************************
		i=0;
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
		caracter=fgetc(archivoConfiguracion);
		}
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
			config.IP_SWAP[i]=caracter;
			caracter=fgetc(archivoConfiguracion);
		i++;
		}
		config.IP_SWAP[i]=0;
//Cargar PUERTO SWAP************************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
				caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					config.PUERTO_SWAP[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
				i++;
				}
				config.PUERTO_SWAP[i]=0;
//CARGAR MAXIMO MARCOS POR PROCESO********************************************************
	i=0;
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		linea[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	linea[i]=0;
	config.MAXIMO_MARCOS_POR_PROCESO = atoi(linea);

//CARGAR CANTIDAD MARCOS********************************************************
		i=0;
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
		caracter=fgetc(archivoConfiguracion);
		}
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
			linea[i]=caracter;
			caracter=fgetc(archivoConfiguracion);
		i++;
		}
		linea[i]=0;
		config.CANTIDAD_MARCOS = atoi(linea);

//CARGAR TAMANIO MARCO********************************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
				caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					linea[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
				i++;
				}
				linea[i]=0;
				config.TAMANIO_MARCO = atoi(linea);
//CARGAR ENTRADAS TLB********************************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
				caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					linea[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
				i++;
				}
				linea[i]=0;
				config.ENTRADAS_TLB = atoi(linea);

//Cargar TLB HABILITADA***************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
				caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					linea[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
				i++;
				}
				linea[i]=0;
				if(strcmp(linea,"SI")==0){
					config.TLB_HABILITADA=1;
				}
				if(strcmp(linea,"NO")==0){
						config.TLB_HABILITADA=0;
				}
				if(strcmp(linea,"SI")&& strcmp(linea,"NO")){
					config.TLB_HABILITADA=-1;
					printf("Ingrese si o no en el campo TLB Habilitada en el archivo de configuracion \n");
					config.estado=-1;
					fclose(archivoConfiguracion);
					return config;
				}
//CARGAR RETARDO MEMORIA********************************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
				caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					linea[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
				i++;
				}
				linea[i]=0;
				config.RETARDO_MEMORIA = atoi(linea);
	config.estado=1;
	fclose(archivoConfiguracion);
	return config;
}



/* Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_SWAP cargarConfiguracionSWAP(char * ruta) {
	config_SWAP config;
	FILE * archivoConfiguracion;
	char linea[50];
	char caracter;
	int i=0;
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s\n",ruta);
		config.estado=0;
	return config;
	}
//Cargar PUERTO ESCUCHA************************************************
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		config.PUERTO_ESCUCHA[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	config.PUERTO_ESCUCHA[i]=0;

//Cargar NOMBRE SWAP************************************************
		i=0;
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
		caracter=fgetc(archivoConfiguracion);
		}
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
			config.NOMBRE_SWAP[i]=caracter;
			caracter=fgetc(archivoConfiguracion);
		i++;
		}
		config.NOMBRE_SWAP[i]=0;

//CARGAR CANTIDAD DE PAGINAS********************************************************
	i=0;
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
	caracter=fgetc(archivoConfiguracion);
	}
	caracter=fgetc(archivoConfiguracion);
	while(caracter!='"'){
		linea[i]=caracter;
		caracter=fgetc(archivoConfiguracion);
	i++;
	}
	linea[i]=0;
	config.CANTIDAD_PAGINAS = atoi(linea);

//CARGAR TAMANIO PAGINA********************************************************
		i=0;
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
		caracter=fgetc(archivoConfiguracion);
		}
		caracter=fgetc(archivoConfiguracion);
		while(caracter!='"'){
			linea[i]=caracter;
			caracter=fgetc(archivoConfiguracion);
		i++;
		}
		linea[i]=0;
		config.TAMANIO_PAGINA = atoi(linea);

//CARGAR RETARDO COMPACTACION********************************************************
				i=0;
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
				caracter=fgetc(archivoConfiguracion);
				}
				caracter=fgetc(archivoConfiguracion);
				while(caracter!='"'){
					linea[i]=caracter;
					caracter=fgetc(archivoConfiguracion);
				i++;
				}
				linea[i]=0;
				config.RETARDO_COMPACTACION = atoi(linea);

	config.estado=1;
	fclose(archivoConfiguracion);
	return config;
}
