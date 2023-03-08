#sudo apt update && apt install git make qtbase5-dev g++

cd ~

mkdir -p Projects 

cd Projects

set -e

#git clone https://github.com/faerbersteve/qubes_debian_installer

rm -rf build-qubes-debian-installer

mkdir -p build-qubes-debian-installer

cd build-qubes-debian-installer

qmake ../qubes_debian_installer/qubes_debian_installer.pro

make -j$(nproc)

mkdir -p ~/Desktop/qubes-debian-installer 
 
cp qubes_debian_installer ~/Desktop/qubes-debian-installer/
