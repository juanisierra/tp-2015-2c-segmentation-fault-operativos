#Ejecutar primero chmod+x instalarCommons.sh
#Luego source instalarCommons.sh
chmod +x cambiarpath.sh #Pone el LD_LIBRARY_PATH en la consola
source cambiarpath.sh
echo "Empezando a instalar commons"
cd so-commons-library
make install
cd ..
echo "Instalando Librerias"
cd librerias-sf/Release
make clean
make all
cd ..
cd ..
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

