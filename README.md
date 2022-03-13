# qubes debian installer

## About
This is a tool to help to install qubes components on any debian based linux.
Current function is downloading some of the qubes projects and creating the debian packages.

The project is work in progress. 


## Motivation/Goal
Learn on how qubes works

Get qubes running on a debian/ubuntu dom0

## Usage

Download project, open it with QtCreator and run it

If dependency issues appear please try to install them manually to get further

Developed & tested on Ubuntu 21.10

## Issues

Known issues:
  - dependencies needs to be installed manually to met requirements to build the package
  
  - qubes-core-admin is missing debian files, fixed in this [repo](https://github.com/faerbersteve/qubes-core-admin)
  - qubes-core-admin-client fails because of issue with sphinx `Sphinx error:
Undocumented arguments for command 'qvm-template': '--keep-cache, --verbose, -q, -v'`, fixed in this [repo](https://github.com/faerbersteve/qubes-core-admin-client)
  
   - qubes-gui-agent-linux fails with error: dh_install: warning: Cannot find (any matches for) "etc/xdg/Trolltech.conf" (tried in ., debian/tmp)


## Contribution

Your help is greatly apreciated.
