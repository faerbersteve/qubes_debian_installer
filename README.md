# qubes debian installer

## About
This is a tool to help to install qubes components on any debian based linux.
Current function is downloading some of the qubes projects and creating the debian packages.

The project is work in progress. 


## Motivation/Goal
Learn on how qubes works

Get qubes running on a debian/ubuntu dom0

## Usage

Download project, run make and execute it

Currently it creates the basic packages for installing qubes manager.
Qubes Manager starts after solving the issues mentioned below.
It is not functional yet, but hopefully soon.

![alt text](https://github.com/faerbersteve/qubes_debian_installer/blob/42d0d2dbbef65314cd6372376029570397b4dd36/proof.png)
If dependency issues appear please try to install them manually to get further

Developed & tested on Ubuntu 21.10

## Issues

Known issues:
  - qubes-qubesdb-dom0 service is missing when installing
  - dependencies missing when installing qubes-core-dom0


## Contribution

Your help is greatly apreciated.
