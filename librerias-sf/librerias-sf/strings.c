/*
 * strings.c
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */
#include <strings.h>
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


