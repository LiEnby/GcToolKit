#!/bin/sh

# Build Kern
cd kern
mkdir build
cd build
cmake ..
make install

if [ $? -eq 1 ] 
then
    echo "error building kern"
	exit
fi


# Back to root
cd ../..

# Build App
cd app
mkdir build
cd build
cmake ..
make

if [ $? -eq 1 ] 
then
    echo "error building app"
	exit
fi

# Back to root
cd ../..

# Build backup_gc_network pc app
cd pc
mkdir build
cd build
cmake ..
make

if [ $? -eq 1 ] 
then
    echo "error building pc"
	exit
fi

cd ../..

mkdir build
cp app/build/GCToolKit.vpk build/GCToolKit.vpk
cp pc/build/gc_backup_network.exe build/gc_backup_network.exe
cp pc/build/gc_backup_network build/gc_backup_network
