#SSL_LIBS=-lssl -lcrypto
#SSL_FLAGS=-DWITH_OPENSSL

SSL_LIBS=
SSL_FLAGS=

all: soapC.c client server

soapC.c:
	soapcpp2 -b -c conecta4.h

client:
	gcc $(SSL_FLAGS) -o client client.c soapC.c soapClient.c game.c -lgsoap -g $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE)

server:	
	gcc $(SSL_FLAGS) -o server server.c soapC.c soapServer.c game.c -lgsoap -g -lpthread $(SSL_LIBS) -L$(GSOAP_LIB) -I$(GSOAP_INCLUDE)

run_server:
	./server 22500

run_client:
	./client http://localhost:22500

memcheck_server:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind.log ./server 22500

clean:	
	rm client server game.o *.xml *.nsmap *.wsdl *.xsd soapStub.h soapServerLib.* soapH.h soapServer.* soapClientLib.* soapClient.* soapC.* *.log
