/*
 * strings.c
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */
#include <strings.h>

#define TAMANIOMAXIMOTEXTO 200
#define TAMANIOMAXIMOLINEA 200
typedef enum{INICIAR, LEER, ESCRIBIR, ES, FINALIZAR,ERROR}instruccion_t;
void separarInstruccionParametro(char*linea,char*instruccion,char parametro[])
{
	int i=0;
	int x=0;
	while(linea[i]!=' '&& linea[i]!=0 &&linea[i]!='\n')
		{
			instruccion[i]=linea[i];
			i++;
		}
	instruccion[i]=0;
	while(linea[i]!=0 && linea[i]!='\n')
		{
			if(linea[i]!=' ')
			{
				parametro[x]=linea[i];
				x++;
			}
			i++;
		}
	parametro[x]=0;
	return;
}
instruccion_t interpretarMcod(char linea[],int *parametro1,char *parametro2 )
{	instruccion_t instruccion;
	int i=0;
	int x=0;
	char buffer1[50];
	char buffer2[200];
	while(linea[i]!=' ' && linea[i]!='\n' &&linea[i]!=0)
	{
		buffer1[i]=linea[i];
		i++;
	}
	buffer1[i]=0;
	while(linea[i]==' ') i++; //Avanza los espacios
	if(strcmp(buffer1,"iniciar")==0)
	{
		instruccion= INICIAR;
	while(linea[i]!=' ' && linea[i]!='\n')
	{
		buffer2[x]=linea[i];
		x++;
		i++;
	}
	buffer2[x]=0;
	*parametro1=atoi(buffer2);
	return instruccion;
	}
	if(strcmp(buffer1,"leer")==0)
		{
			instruccion= LEER;
		while(linea[i]!=' ' && linea[i]!='\n')
		{
			buffer2[x]=linea[i];
			x++;
			i++;
		}
		buffer2[x]=0;
		*parametro1=atoi(buffer2);
		return instruccion;
		}
	if(strcmp(buffer1,"escribir")==0)
			{
				instruccion= ESCRIBIR;
				x=0;
			while(linea[i]!=' ' && linea[i]!='\n')
			{
				buffer2[x]=linea[i];
				x++;
				i++;
			}
			buffer2[x]=0;
			*parametro1=atoi(buffer2);
			x=0;
			while(linea[i]!='"' && linea[i]!=0 && linea[i]!='\n') i++;
			i++;
			while(linea[i]!='"' && linea[i]!=0 && linea[i]!='\n'){
				buffer2[x]=linea[i];
				i++;
				x++;
			}
			buffer2[x]=0;
			strcpy(parametro2,buffer2);
			return instruccion;
			}
	if(strcmp(buffer1,"entrada-salida")==0)
		{
			instruccion= ES;
		while(linea[i]!=' ' && linea[i]!='\n')
		{
			buffer2[x]=linea[i];
			x++;
			i++;
		}
		buffer2[x]=0;
		*parametro1=atoi(buffer2);
		return instruccion;
		}
	if(strcmp(buffer1,"finalizar")==0)
		{
			instruccion= FINALIZAR;
		return instruccion;
		}

	return ERROR;
}
int ultimaLinea(char *path)
{	int i=0;
char buffer[TAMANIOMAXIMOLINEA-1];
	FILE* archivo;
	archivo=fopen(path,"r");
	while(fgets(buffer,TAMANIOMAXIMOLINEA,archivo)!=0) i++;
	return i;
}
