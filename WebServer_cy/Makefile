webserver_cy:webserver_cy.o assist_cy.o dealRequest_cy.o
	gcc -o webserver_cy webserver_cy.o assist_cy.o dealRequest_cy.o -Wall

webserver_cy.o:webserver_cy.c utility_cy.h dealRequest_cy.h
	gcc -o webserver_cy.o webserver_cy.c -c -ansi -pedantic -Wall

assist_cy.o:assist_cy.c utility_cy.h assist_cy.h
	gcc -o assist_cy.o assist_cy.c -c -ansi -pedantic -Wall

dealRequest_cy.o:dealRequest_cy.c dealRequest_cy.h utility_cy.h
	gcc -o dealRequest_cy.o dealRequest_cy.c -c -ansi -pedantic -Wall

install:
	cp webserver_cy /usr/bin

unistall:
	$(RM) *.o webserver_cy /usr/bin/webserver_cy
clean:
	$(RM) *.o webserver_cy
