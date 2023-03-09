#!/bin/bash

getcode() {

	#change to Projects
	cd ~/Projects

	if [[ ! -d "$1" ]];
	then
		echo "$1 not downloaded, clone"
		git clone "https://github.com/faerbersteve/$1"
	fi
	
	echo "$1 - pull changes"
	cd "$1"

	git pull
}

if [[ $(command -v qmake-qt5) ]];
then
	echo "found qmake-qt5" 
else
	echo "qmake-qt5 not found, install needed packages for building"
	sudo apt update && apt install -y git make qtbase5-dev g++
fi

cd ~

if [[ ! -d "Projects" ]];
then 
	echo "Project folder doesn't exists"
	mkdir Projects
fi

cd Projects

set -e

getcode 'qubes_debian_installer' 

rm -rf build-qubes-debian-installer

mkdir -p build-qubes-debian-installer

cd build-qubes-debian-installer

qmake ../qubes_debian_installer/qubes_debian_installer.pro

make -j$(nproc)

mkdir -p ~/Desktop/qubes-debian-installer 
 
cp qubes_debian_installer ~/Desktop/qubes-debian-installer/
