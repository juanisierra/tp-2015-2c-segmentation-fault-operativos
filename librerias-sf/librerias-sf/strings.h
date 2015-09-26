/*
 * strings.h
 *
 *  Created on: 21/9/2015
 *      Author: utnso
 */

#ifndef LIBRERIAS_SF_STRINGS_H_
#define LIBRERIAS_SF_STRINGS_H_
void separarInstruccionParametro(char*renglon,char*instruccion,char parametro[]);
//Recibe una linea y separa la instruccion de sus parametros, devuelve ERROR o 5 si es una linea invalida.


instruccion_t interpretarMcod(char linea[],uint32_t *parametro1,char *parametro2 );
//Devuelve el valor de la ultima linea contando la primera como 0, al pasarle el path debe ser un string terminadno en\0.


int ultimaLinea(char *path); //Chequea la direccion del archivo y devuelve el numero de la ultima linea, si el archivo
							//no se puede abrir devuelve -1;


void leerLinea(FILE* archivo,char*buffer,int numeroLinea);//Recibe un archivo abierto, mueve el puntero al inicio y devuelve el contenido de la linea pedida,
															//Si el numero es mayor a las lineas del archivo devuelve la ultima.


#endif /* LIBRERIAS_SF_STRINGS_H_ */
