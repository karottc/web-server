## WEBSERVER_CY ##

(c) Copright Chen Yang 2011

Email:karottc@gmail.com


Includes:

* README.md (this file)
* Makefile
* wevserver_cy.c
* utility_cy.h
* dealRequest_cy.h
* dealRequest_cy.c
* assist_cy.h
* assist_cy.c


### Function ###

Implements a simple webserver

### Usage ###

This are some sources codes.You need install and run.But it is missing sercurity 
features.


### Installation ###

If you run under Linux,just type:

		~$ make
		~$ make install


then you can type:
		
		~$ webserver_cy

web server has been setted up now.


### Test web server ###

The root directory is "/home/administrator/httpd" that this web server uses.So,You need to place your website or webpage to this web server root directory.

Then,you open browsers and type:

		localhost:<port number>/<web page that you want to visit>

so,this web werver is finished.


### Uninstallation ###

You need to type:
	
		~$ make uninstall

now,the web server has been uninstalled.
