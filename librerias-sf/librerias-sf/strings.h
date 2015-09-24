/*
 * strings.h
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_STRINGS_H_
#define LIBRERIAS_SF_STRINGS_H_
typedef enum{INICIAR, LEER, ESCRIBIR, ES, FINALIZAR,ERROR}instruccion_t;
void separarInstruccionParametro(char*renglon,char*instruccion,char parametro[]);
//Recibe una linea y separa la instruccion de sus parametros, devuelve ERROR o 5 si es una linea invalida.
instruccion_t interpretarMcod(char linea[],int *parametro1,char *parametro2 );
//Devuelve el valor de la ultima linea contando la primera como 0, al pasarle el path debe ser un string terminadno en\0.
int ultimaLinea(char *path);

#endif /* LIBRERIAS_SF_STRINGS_H_ */
