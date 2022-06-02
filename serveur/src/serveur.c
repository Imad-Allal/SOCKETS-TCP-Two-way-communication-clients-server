#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../include/check.h"


#define SIZE 256
#define CLIENTS 20

int numeroClient = 0;

struct sockaddr_in *
adresse(char *host, int port)
{
  if(port != 0 && port < 1024)
    {
      fprintf(stderr,"Error adresse: port must be 0 or larger than 1024!\n");
      exit(EXIT_FAILURE);
    }

  struct hostent *hp = gethostbyname(host);
  if(hp == NULL)
    {
      fprintf(stderr,"Error adresse: wrong host name!\n");
      exit(EXIT_FAILURE);
    }
  
  struct sockaddr_in *res = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
  if(res ==0)
    {
      fprintf(stderr,"Error adresse: malloc failed!\n");
      exit(EXIT_FAILURE);
    }

  memset((char*) res,0,sizeof(struct sockaddr_in));

  res->sin_family = AF_INET;
  res->sin_port = htons(port);

  char *ad = hp->h_addr_list[0];
  (res->sin_addr).s_addr = ((struct in_addr*)ad)->s_addr;
  
  return res;
}

struct info_client
{
    int socket;
    struct sockaddr_in adresse;
    char nom[SIZE];
    int num;
};
struct info_client *clients[CLIENTS];

void listClients(struct info_client *client){

	for(int i=0; i < CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = client;
			break;
		}
  }
}

void supprimerClient(struct info_client *client, int numero){

	for(int i=0; i < CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->num == numero){
        clients[i] = NULL;
        numeroClient--;
        break;
      }
		}
  }
}

void envoyerMessage(char* msg, int numero){
  for (int i = 0; i < CLIENTS; i++)
  {
    if (clients[i])
      if (clients[i]->num != numero)
      {
        if (send(clients[i]->socket, msg, SIZE, 0) == -1)
        {
          printf("Erreur lors de l'envoi du message au client %d\n", clients[i]->num);
          break;
        }
      }
  }
}

void *fcn_thread(void *arg)
{
  int quitter = 0;
  char message[SIZE];
  char nomClient[SIZE];
  char messageClient[SIZE];

  int s = ((struct info_client*) arg)->socket;
  int numeroC = ((struct info_client *)arg)->num;
  strcpy(nomClient, ((struct info_client *)arg)->nom);
  struct sockaddr_in addr = ((struct info_client*) arg)->adresse;

  printf("(***)%s a rejoint le chat(***)\n", nomClient);

  sprintf(message, "%s vient d'atterir sur la salle :D\n", nomClient);
  printf("%s", message);
  envoyerMessage(message, numeroC);

  while(1){
    if(recv(s, messageClient, SIZE, 0) == -1){
      printf("Erreur lors de la reception du message du client %d\n", numeroC);
      break;
    }

    printf("--Message reçu de la part de %s--\n",nomClient);


    if (strcmp(messageClient,"Quitter") == 0 || strcmp(messageClient,"quitter") == 0){
      printf("(***)%s a quitté le chat(***)\n");
      sprintf(message, "%s a quitte la salle.\n", nomClient);
      envoyerMessage(message,numeroC);
      break;
    }

    else{
      sprintf(message,"%s : %s\n",nomClient, messageClient);
      printf("client %d :: Message : %s\n", numeroC, message);
      envoyerMessage(message, numeroC);
      printf("-Message envoyé-\n");
    }
  }
  supprimerClient((struct info_client*)arg,numeroC);
  close(s);
  free(arg);
  numeroClient--;
  pthread_exit((void*)NULL);
}


int main (){

    char *host = "localhost";
    int port_serveur = 8989;
    char choix1 = '1';
    char choix2 = '2';
    char choix3 = '3';
    char choix4 = '4';
    char nomClient[SIZE];
    char mdpClient[SIZE];
    char choixClient;
    char messageInscription[SIZE] = "Choisissez votre identifiant : ";
    char messageConnexion[SIZE] = "Entrez votre identifiant : ";

    struct sockaddr_in *addr_serveur = adresse(host, port_serveur);

    int se = socket(AF_INET, SOCK_STREAM, 0); // Creation de la socket d'ecoute
    if (se == -1)
        {
        fprintf(stderr,"Erreur socket!\n");
        exit(EXIT_FAILURE);
        }

    if (bind(se, (struct sockaddr*)addr_serveur, sizeof(struct sockaddr_in)) == -1)
    {
      fprintf(stderr,"Erreur socket!\n");
      exit(EXIT_FAILURE);
    }
    printf("En attente de clients...\n");
    while (1)
    {
      if (listen(se, CLIENTS) == -1)
      {
        fprintf(stderr, "Erreur listen !\n");
        exit(1);
	    }

      struct sockaddr_in addr_client;
      socklen_t taille;

      int sc = accept(se, (struct sockaddr *)&addr_client, &taille);
      printf("------ Nouveau Client------\n");

      while(1){

        /*
        Code choix du client
        */
        if (recv(sc, &choixClient, 1, 0) == -1)
        {
          fprintf(stderr, "Erreur recv!\n");
          exit(EXIT_FAILURE);
              }

          if (choixClient == '0'){
              /*
              Code inscription du client
              */
              printf("(*)Le client souhaite s'inscrire(*)\n");
              send(sc, messageInscription, SIZE, 0);

              if (recv(sc, nomClient, SIZE, 0) == -1) // Recevoir le nom du client
              {
                  fprintf(stderr,"Erreur recv!\n");
                  exit(EXIT_FAILURE);
              }

              printf("(*)Verification si '%s' existe dans la base de donnees(*)\n",nomClient);

              if (checkNom(nomClient)){
                  printf("(*)Le nom existe dans la liste, renvoi du client au menu principal(*)\n");
                  send(sc, &choix2, 1, 0);
                  //printf("Done\n");
              }
              else{
                  printf("(*)Le nom n'existe pas dans la BDD, demande au client de creer un mot de passe\n(*)");
                  send(sc, &choix3, 1, 0);
                  if (recv(sc, mdpClient, SIZE, 0) == -1) // Recevoir le mot de passe du client
                  {
                      fprintf(stderr,"Erreur recv!\n");
                      exit(EXIT_FAILURE);
                  }
                  if(ajouterClient(nomClient, mdpClient)){
                      printf("(*)Client ajoute avec succes, renvoi du client au menu principal(*)\n");
                      send(sc, &choix2, 1, 0);
                  }
              }
          }
          else if (choixClient == '1'){
              /*
              Code connexion du client
              */
              printf("(**)Le client souhaite se connecter(**)\n");
              send(sc, messageConnexion, SIZE, 0);

              if (recv(sc, nomClient, SIZE, 0) == -1) // Recevoir le nom du client
              {
                  fprintf(stderr,"Erreur recv!\n");
                  exit(EXIT_FAILURE);
              }

              printf("(**)Verification si '%s' existe dans la base de donnees(**)\n",nomClient);

              if (checkNom(nomClient)){
                  printf("(**)Le nom existe dans la liste, demande au client d'entrer son mot de passe(**)\n");
                  send(sc, &choix3, 1, 0);
                  if (recv(sc, mdpClient, SIZE, 0) == -1) // Recevoir le mot de passe du client
                  {
                    fprintf(stderr,"Erreur recv!\n");
                    exit(EXIT_FAILURE);
                  }
                  if (checkMdp(mdpClient, nomClient))
                  {
                    printf("------Authentifacion reussie------\n");
                    send(sc, &choix4, 1, 0);
                    break;
                  }
                  else{
                    printf("(**)Mot de passe entre par le client incorrecte !(**)\n");
                    send(sc, &choix1, 1, 0); // Demander au client d'entrer ses infos de connexion
                  }
              }
              else{
                  printf("(**)Le nom n'existe pas dans la BDD, renvoi du client au menu principal(**)\n");
                  send(sc, &choix2, 1, 0);
              }
          }
      }
      struct info_client *client = (struct info_client *)malloc(sizeof(struct info_client));
      if (client == NULL){
      printf("Erreur malloc");
      exit(1);
      }

      client->socket = sc;
      client->adresse = addr_client;
      strcpy(client->nom,nomClient);
      client->num = numeroClient;
      listClients(client);

      numeroClient++;

      printf("Numero client = %d\n",numeroClient);
      pthread_t thread;
      pthread_create(&thread, 0, fcn_thread, (void*)client);
    }
    close(se);
    return 0;
}