/*
 * config.c
 *
 *  Created on: 6/9/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
/*Recibe un string linea, y un string campo y valor, devuelve campo y valor separados
 * debo pasarle por parametro valor y campo
 * requiere el formato CAMPO = "valor"
 */
void separarCampoYValor(char linea[],char campo[], char valor[]){
 int i=0;
 int x=0;
 while(linea[i]!='='&& linea[i]!=0)
 {
	 if(linea[i]!=' ' && linea[i]!=0)
	 {
		 campo[x]=linea[i];
		 x++;
	 }
	 i++;
 }
 campo[x]=0;
 x=0;
 while(linea[i]!='"') i++;
 i++;
 while(linea[i]!='"' && linea[i]!=0)
 {
	 if(linea[i]!=' ')
	 {
		 valor[x]=linea[i];
		 x++;
	 }
	 i++;
 }
 valor[x]=0;
}

/* cargarConfiguracionPL
 * Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * Retorna -1 si el algoritmo esta mal especificado y lo dice por pantalla
 * NOTA: El archivo de configuracion contiene los comentarios con #
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_pl cargarConfiguracionPL(char * ruta) {

	config_pl config;
	FILE * archivoConfiguracion;
	char linea[200];
	char campo[50];
	char valor[50];
	char cargados[3]; //Vector que avisa si se cargo el dato, 0 PE , 1 AP, 2 QUANTUM
	int verifica; // Cuando vale 1 se cargaron todos los datos
	int m=0;
	for(m=0;m<3;m++) cargados[m]=0; //Lo inicializo en 0 sin datos cargados
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s \n",ruta);
		config.estado=0;
	return config;
	}
//COMIENZA CARGA DE LINEAS**********************************************
	//Saltea las lineas que empiezan con #
	while(fgets(linea,200,archivoConfiguracion)!=NULL){
		if(linea[0]!='#' && linea[0]!='\n') {

	valor[0]=0;
	campo[0]=0;
	separarCampoYValor(linea,campo,valor);
//Cargar Puerto ESCUCHA************************************************
	if(strcmp(campo,"PUERTO_ESCUCHA")==0)
	{
		strcpy(config.PUERTO_ESCUCHA,valor);
		cargados[0]=1;
	}
//Cargar Algoritmo Planificacion***************************************
	if(strcmp(campo,"ALGORITMO_PLANIFICACION")==0)
	{
	if(strcmp(valor,"FIFO")==0){
		config.ALGORITMO_PLANIFICACION=1;
		cargados[1]=1;
	}
	if(strcmp(valor,"RR")==0){
			config.ALGORITMO_PLANIFICACION=2;
			cargados[1]=1;
	}
	if(strcmp(valor,"FIFO")&& strcmp(valor,"RR")){
		config.ALGORITMO_PLANIFICACION=0;
		printf("El algoritmo de Planificacion no está correctamente configurado \n");
		config.estado=-1;
		config.QUANTUM=0;
		fclose(archivoConfiguracion);
		return config;
	}
	}
//Setea el Quantum********************************************************
/* Retorna 0 en estado si no se pudo abrir el archivo de configuracion
	 * Retorna 1 si se cargo correctamente
	 * Retorna -1 si el algoritmo esta mal especificado y lo dice por pantalla
	 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
	 */
	if(strcmp(campo,"QUANTUM")==0)
	{
	config.QUANTUM = atoi(valor);
	cargados[2]=1;
	}

	}
	}  //VERIFICACION DEL INGRESO DE TODOS LOS PARAMETROS***************************
	verifica=1;
	for(m=0;m<3;m++) {
		if(cargados[m]==0)
			{verifica=0;
			}
	}
	if(verifica==1)
	{
	config.estado=1;
	} else
	{config.estado=-1;
	printf("Faltan especificar parametros en el archivo de configuracion\n");
	}
	fclose(archivoConfiguracion);
	return config;
}

/* cargarConfiguracionCPU
 * Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * NOTA: El archivo de configuracion contiene los comentarios con #
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_CPU cargarConfiguracionCPU(char * ruta) {
	config_CPU config;
	FILE * archivoConfiguracion;
	char linea[200];
		char campo[50];
		char valor[50];
		char cargados[6]; //Vector que avisa si se cargo el dato, 0 PE , 1 AP, 2 QUANTUM
		int verifica; // Cuando vale 1 se cargaron todos los datos
		int m=0;
		for(m=0;m<6;m++) cargados[m]=0; //Lo inicializo en 0 sin datos cargados
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s\n",ruta);
		config.estado=0;
	return config;
	}
//COMIENZA CARGA DE LINEAS**********************************************
	//Saltea las lineas que empiezan con #
		while(fgets(linea,200,archivoConfiguracion)!=NULL){
			if(linea[0]!='#' && linea[0]!='\n') {

		valor[0]=0;
		campo[0]=0;
		separarCampoYValor(linea,campo,valor);
//Cargar PUERTO PLANIFICADOR************************************************
	if(strcmp(campo,"PUERTO_PLANIFICADOR")==0)
	{
		strcpy(config.PUERTO_PLANIFICADOR,valor);
		cargados[0]=1;
	}
//Cargar IP PLANIFICADOR************************************************
	if(strcmp(campo,"IP_PLANIFICADOR")==0)
	{
		strcpy(config.IP_PLANIFICADOR,valor);
		cargados[1]=1;
		}

//Cargar IP MEMORIA************************************************
	if(strcmp(campo,"IP_MEMORIA")==0)
	{
			strcpy(config.IP_MEMORIA,valor);
			cargados[2]=1;
	}
//Cargar PUERTO MEMORIA************************************************
	if(strcmp(campo,"PUERTO_MEMORIA")==0){
			strcpy(config.PUERTO_MEMORIA,valor);
			cargados[3]=1;
	}

//CARGAR CANTIDAD DE HILOS********************************************************
	if(strcmp(campo,"CANTIDAD_HILOS")==0)
	{
	config.CANTIDAD_HILOS = atoi(valor);
	cargados[4]=1;
	}
//CARGAR RETARDO********************************************************
if(strcmp(campo,"RETARDO")==0)
	{
		config.RETARDO = atoi(valor);
		cargados[5]=1;
	}
}
}  //VERIFICACION DEL INGRESO DE TODOS LOS PARAMETROS***************************
	verifica=1;
	for(m=0;m<6;m++) {
		if(cargados[m]==0)
			{
			verifica=0;
			}
	}
	if(verifica==1)
	{
	config.estado=1;
	} else
	{
	config.estado=-1;
	printf("Faltan especificar parametros en el archivo de configuracion\n");
	}
	fclose(archivoConfiguracion);
	return config;
}


/* cargarConfiguracionADM
 * Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * Retorna -1 si se escribio mal en TLB habilitada
 * NOTA: El archivo de configuracion contiene los comentarios con #
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_ADM cargarConfiguracionADM(char * ruta) {
	config_ADM config;
	FILE * archivoConfiguracion;
	char linea[200];
		char campo[50];
		char valor[50];
		char cargados[10]; //Vector que avisa si se cargo el dato, 0 PE , 1 AP, 2 QUANTUM
		int verifica; // Cuando vale 1 se cargaron todos los datos
		int m=0;
		for(m=0;m<10;m++) cargados[m]=0; //Lo inicializo en 0 sin datos cargados
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s\n",ruta);
		config.estado=0;
	return config;
	}
//COMIENZA CARGA DE LINEAS**********************************************
	//Saltea las lineas que empiezan con #
		while(fgets(linea,200,archivoConfiguracion)!=NULL){
			if(linea[0]!='#' && linea[0]!='\n') {

		valor[0]=0;
		campo[0]=0;
		separarCampoYValor(linea,campo,valor);
//Cargar PUERTO ESCUCHA************************************************
	if(strcmp(campo,"PUERTO_ESCUCHA")==0)
	{
		strcpy(config.PUERTO_ESCUCHA,valor);
		cargados[0]=1;
	}

//Cargar IP SWAP************************************************
		if(strcmp(campo,"IP_SWAP")==0)
		{
			strcpy(config.IP_SWAP,valor);
		cargados[1]=1;
		}

//Cargar PUERTO SWAP************************************************
		if(strcmp(campo,"PUERTO_SWAP")==0)
		{
			strcpy(config.PUERTO_SWAP,valor);
			cargados[2]=1;
		}

//CARGAR MAXIMO MARCOS POR PROCESO********************************************************
	if(strcmp(campo,"MAXIMO_MARCOS_POR_PROCESO")==0)
	{
	config.MAXIMO_MARCOS_POR_PROCESO = atoi(valor);
	cargados[3]=1;
	}
//CARGAR CANTIDAD MARCOS********************************************************
	if(strcmp(campo,"CANTIDAD_MARCOS")==0)
	{
		config.CANTIDAD_MARCOS = atoi(valor);
		cargados[4]=1;
	}
//CARGAR TAMANIO MARCO********************************************************
	if(strcmp(campo,"TAMANIO_MARCO")==0)
	{
		config.TAMANIO_MARCO = atoi(valor);
		cargados[5]=1;
	}
//CARGAR ENTRADAS TLB********************************************************
	if(strcmp(campo,"ENTRADAS_TLB")==0)
	{
		config.ENTRADAS_TLB = atoi(valor);
		cargados[6]=1;
	}
//Cargar TLB HABILITADA***************************************
	if(strcmp(campo,"TLB_HABILITADA")==0)
	{
				if(strcmp(valor,"SI")==0){
					config.TLB_HABILITADA=1;
					cargados[7]=1;
				}
				if(strcmp(valor,"NO")==0){
						config.TLB_HABILITADA=0;
						cargados[7]=1;
				}
				if(strcmp(valor,"SI")&& strcmp(valor,"NO")){
					config.TLB_HABILITADA=-1;
					printf("Ingrese si o no en el campo TLB Habilitada en el archivo de configuracion \n");
					config.estado=-1;
					fclose(archivoConfiguracion);
					return config;
				}
	}
//CARGAR RETARDO MEMORIA********************************************************
		if(strcmp(campo,"RETARDO_MEMORIA")==0)
		{
				config.RETARDO_MEMORIA = atoi(valor);
				cargados[8]=1;
		}
//CARGAR ALGORITMO REEMPLAZO *********************
			if(strcmp(campo,"ALGORITMO_REEMPLAZO")==0)
			{
						if(strcmp(valor,"FIFO")==0){
							config.ALGORITMO_REEMPLAZO=0;
							cargados[9]=1;
						}
						if(strcmp(valor,"LRU")==0){
								config.ALGORITMO_REEMPLAZO=1;
								cargados[9]=1;
						}
						if(strcmp(valor,"CLOCK-M")==0) {
							config.ALGORITMO_REEMPLAZO=2;
							cargados[9]=1;
						}
						if(strcmp(valor,"FIFO")&& strcmp(valor,"LRU") && strcmp(valor,"CLOCK-M")){
							config.ALGORITMO_REEMPLAZO=-1;
							printf("Ingrese un algoritmo de reemplazo adecuado\n");
							config.estado=-1;
							fclose(archivoConfiguracion);
							return config;
						}
			}
			}
} //VERIFICACION DEL INGRESO DE TODOS LOS PARAMETROS***************************
	verifica=1;
		for(m=0;m<10;m++) {
			if(cargados[m]==0)
				{verifica=0;
				}
		}
		if(verifica==1)
		{
		config.estado=1;
		} else
		{config.estado=-1;
		printf("Faltan especificar parametros en el archivo de configuracion\n");
		}
		fclose(archivoConfiguracion);
		return config;
}



/* cargarConfiguracionSWAP
 * Retorna 0 en estado si no se pudo abrir el archivo de configuracion
 * Retorna 1 si se cargo correctamente
 * NOTA: El archivo de configuracion contiene los comentarios con #
 * EJEMPLO: cargarConfiguracionPL("configuracion"); lo busca en la carpeta que contiene luego a DEBUG y src
 */
config_SWAP cargarConfiguracionSWAP(char * ruta) {
	config_SWAP config;
	FILE * archivoConfiguracion;
	char linea[200];
		char campo[50];
		char valor[50];
		char cargados[6]; //Vector que avisa si se cargo el dato, 0 PE , 1 AP, 2 QUANTUM
		int verifica; // Cuando vale 1 se cargaron todos los datos
		int m=0;
		for(m=0;m<6;m++) cargados[m]=0; //Lo inicializo en 0 sin datos cargados
	archivoConfiguracion= fopen(ruta,"r");
	if(archivoConfiguracion==NULL)
	{
		printf("No se encuentra el archivo de configuracion en la ruta %s\n",ruta);
		config.estado=0;
	return config;
	}
//COMIENZA CARGA DE LINEAS**********************************************
//Saltea las lineas que empiezan con #
	while(fgets(linea,200,archivoConfiguracion)!=NULL){
		if(linea[0]!='#' && linea[0]!='\n') {

	valor[0]=0;
	campo[0]=0;
	separarCampoYValor(linea,campo,valor);

//Cargar PUERTO ESCUCHA************************************************
	if(strcmp(campo,"PUERTO_ESCUCHA")==0)
	{
		strcpy(config.PUERTO_ESCUCHA,valor);
		cargados[0]=1;
	}

//Cargar NOMBRE SWAP************************************************
	if(strcmp(campo,"NOMBRE_SWAP")==0)
	{
			strcpy(config.NOMBRE_SWAP,valor);
			cargados[1]=1;
		}

//CARGAR CANTIDAD DE PAGINAS********************************************************
	if(strcmp(campo,"CANTIDAD_PAGINAS")==0)
	{
	config.CANTIDAD_PAGINAS = atoi(valor);
	cargados[2]=1;
	}
//CARGAR TAMANIO PAGINA********************************************************
	if(strcmp(campo,"TAMANIO_PAGINA")==0)
	{
		config.TAMANIO_PAGINA = atoi(valor);
		cargados[3]=1;
	}
	//CARGAR RETARDO SWAP********************************************************
		if(strcmp(campo,"RETARDO_SWAP")==0)
		{
			config.RETARDO_SWAP = atoi(valor);
			cargados[4]=1;
		}
//CARGAR RETARDO COMPACTACION********************************************************
		if(strcmp(campo,"RETARDO_COMPACTACION")==0)
		{
				config.RETARDO_COMPACTACION = atoi(valor);
			cargados[5]=1;
		}
	}
	}
 //VERIFICACION DEL INGRESO DE TODOS LOS PARAMETROS***************************
			verifica=1;
			for(m=0;m<6;m++) {
				if(cargados[m]==0)
					{verifica=0;
					}
			}
			if(verifica==1)
			{
			config.estado=1;
			} else
			{config.estado=-1;
			printf("Faltan especificar parametros en el archivo de configuracion\n");
			}
			fclose(archivoConfiguracion);
			return config;
}
