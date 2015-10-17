chmod +x cambiarpath.sh #Pone el LD_LIBRARY_PATH en la consola
source cambiarpath.sh
cd Planificador/Release #COMPILA TODOS LOS PROYECTOS
make clean
make all
cd ..
cd ..
cd CPU/Release
make clean
make all
cd ..
cd ..
cd swap/Release
make clean
make all
cd ..
cd ..
cd adm/Release
make clean
make all
cd ..
cd ..
echo "Instalacion lista"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
