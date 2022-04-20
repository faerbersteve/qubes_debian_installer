# qubes debian installer

## About
This is a tool to help to install qubes components on any debian based linux.
Current function is downloading some of the qubes projects and creating the debian packages.

The project is work in progress. 


## Motivation/Goal
Learn on how qubes works

Get qubes running on a debian/ubuntu dom0

## Usage

Download project, run make and execute it.

The project is currently in development.
Please test it first on a virtual machine before running on a real machine.
```
git clone https://github.com/faerbersteve/qubes_debian_installer
cd qubes_debian_installer
make
```

first create the packages with
```
sudo ./qubes_debian_installer -create
```
then install them
```
sudo ./qubes_debian_installer -install
```

after installing, reboot
Now you can install a qubes template like fedora-34

```
qvm-template install fedora-34
```

Qubes VMs can be started, but it is not possible to start the apps.


![alt text](https://github.com/faerbersteve/qubes_debian_installer/blob/42d0d2dbbef65314cd6372376029570397b4dd36/proof.png)
If dependency issues appear please try to install them manually to get further

Developed & tested on Ubuntu 21.10

## Issues

Known issues:
  - can't start applications or run commands 
  - vm settings page devices is deactivated because of missing pci.ids in /usr/share/hwdata/ The file can be copied from /usr/share/misc


## Contribution

Your help is greatly apreciated.
