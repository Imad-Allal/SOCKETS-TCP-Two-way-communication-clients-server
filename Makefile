main:
	gcc client/src/client.c client/src/hashage.c -lcrypt -lpthread -o  clientComp 
	gcc serveur/src/serveur.c serveur/src/check.c -lpthread -o serveurComp
