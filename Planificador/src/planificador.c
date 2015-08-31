#include <stdlib.h>
#include <stdio.h>
#include <commons/temporal.h> //Prueba incluir Biblioteca de Commons
#include <librerias-sf/sockets.h>
int main() {
	printf("Funciona \n");
	char*n1;
	int n;
	n= crearSocketEscucha(5,"6677");
	n1=temporal_get_string_time();
	return 0;
}
