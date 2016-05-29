# Tp Sistemas operativos 2015 2C - Grupo Segmentation Fault
##Enunciado del tp:    
https://github.com/sisoputnfrba/2015-2c-cache13-enunciado    
https://www.gitbook.com/book/sisoputnfrba/2c2015-enunciado-cache13/details    
##Instalacion de Cache 13: Segmentation Fault    
1- hacer el clone:    
	`git clone https://github.com/juanisierra/tp-2015-2c-segmentation-fault-operativos.git`    
2- Instalamos las librerias primero pidiendo permiso sudo    
	`sudo -s`    
	`source instalarLibrerias.sh`    
3- compilamos:    
	`source compilar.sh`    
4- en cada una a ejecutar usamos pra poner el ld library path    
	`source cambiarpath.sh`     

##Otros comandos:    
`ldd archivoEjecutable` //Nos dice las librerias que usa y muestra si las encontro    
 `chmod +x archivosh`  //Le da permiso al archivo de ser ejecutado    
 `sudo -s`  //Crea una consola con permisos de sudo para instalar cosas luego    
 `ls -l` //muestra los archivos y permisos, -x es de ejecucion.    
 `rm -rf` //Borra el directorio con todas las cosas    
 `echo $LD_LIBRARY_PATH` // Muestra el ld libreary path    
