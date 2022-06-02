#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../include/hashage.h"

#define SIZE 256

sem_t sema;

int key = 0;

struct sockaddr_in *adresse(char *host, int port)
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

    /* version naive */
    /* (res->sin_addr).s_addr = ((u_long)ad[0]) + (((u_long)ad[1])<<8) + ((u_long)(ad[2])<<16) + ((u_long)(ad[3])<<24); */
    /* version plus maline */
    (res->sin_addr).s_addr = ((struct in_addr*)ad)->s_addr;

    return res;
}


void* envoyerMessage (void* arg){

    int long s = (int long)arg;
    char message[SIZE];
    int i = 0;
    while (1)
    { //
        read(0, message,SIZE);
        while(message[i] != '\n')
            i++;
        message[i] = '\0';
        i = 0;
        if (strcmp(message, "quitter") == 0 || strcmp(message, "Quitter") == 0)
        {
            send (s,message,SIZE,0);
            printf("Fermeture en cours...");
            key = 1;
            sem_post(&sema);
            break;
        }
        else{
            send (s,message,SIZE,0);
            }
    }
}

void* recevoirMessage (void* arg){

    int long s = (int long)arg;
    char message[SIZE];
    while(1){ //
    if(key)
        break;
    if (recv(s, message, SIZE, 0) == -1)
    {
        printf("Erreur, un probleme est servenu\n");
        exit(1);
        }
        printf("%s", message);
    }
}



int main()
{
    char *host="localhost";
    int port_serveur=8989;

    char choixClient = '2';
    char nomClient[SIZE];
    char mdp1Client[SIZE];
    char mdp2Client[SIZE];
    char message1Serveur[SIZE];
    char message2Serveur[SIZE];

    sem_init(&sema, 0, 0);

    struct sockaddr_in *addr_serveur = adresse(host, port_serveur);
    int s = socket(AF_INET,SOCK_STREAM,0);

    printf("Demande de connexion au serveur....\n");
    if(connect(s, (struct sockaddr*) addr_serveur, sizeof(struct sockaddr_in)) == -1)
    {
        fprintf(stderr, "Connexion échouée !\n");
        close(s);
        exit(1);
    }
    printf("-----Connexion au serveur etablie !-----\n\n");
    printf("Bienvenu sur l'application de messagerie\n");

    while (1){
        if (choixClient == '0'){ // Inscription
            send(s, &choixClient, 1, 0);
            recv(s, message1Serveur, SIZE, 0);
            printf(message1Serveur);
            scanf("%s", nomClient);
            send(s, nomClient, SIZE, 0);
            recv(s, &choixClient, 1, 0);

            if (choixClient == '3'){ // Le nom n'est pas prit, demander au client de creer un mot de passe
                while (1){
                    printf("Choisissez votre mot de passe : ");
                    scanf("%s", mdp1Client);
                    printf("Confirmez votre mot de passe : ");
                    scanf("%s", mdp2Client);

                    if (strcmp(mdp1Client,mdp2Client) == 0)
                        break;
                    else
                        printf("Les deux mots de passe ne sont pas identiques, reessayez s'il vous plait : \n");  
                }
                strcpy(mdp2Client,genererHash(mdp1Client));
                send(s, mdp2Client, SIZE, 0);
                recv(s, &choixClient, SIZE, 0); // choixClient = '2' ==> Retour au menu principal
                printf("Votre inscription est confirmee, vous avez accees à la salle de discussion\n");
            }

            else{
                printf("Erreur : cet identifiant existe deja.\n");
            }
        }

        else if(choixClient == '1'){
            /*
            Authentification du client
            */
            send(s, &choixClient, 1, 0); // Envoi du choix au serveur
            recv(s, message2Serveur, SIZE, 0); // Reception de confirmation du serveur
            printf(message2Serveur);
            scanf("%s", nomClient); 
            send(s, nomClient, SIZE, 0); // Envoi du nom au serveur
            recv(s, &choixClient, 1, 0); // Reception de confirmation du serveur
            if (choixClient == '3'){ // Le Client est valide, il ne lui reste plus qu'a rentrer son mot de passe

                printf("Entrez votre mot de passe : ");
                scanf("%s", mdp1Client);
                strcpy(mdp1Client,genererHash(mdp1Client));
                send(s, mdp1Client, SIZE, 0);
                recv(s, &choixClient, 1, 0);
                if (choixClient == '4')
                    break;
                else{
                    printf("Erreur identifiant ou mot de passe.\n");
                }
            }
        }
        else if(choixClient == '2'){
            /*
            Menu principal
            */
            printf("Menu Principal : \n0 s'inscrire\n1 s'identifier\nEntrez votre choix : ");
            while(1){
                //choixClient = getchar();
                scanf(" %c",&choixClient);
                if (choixClient != '0' && choixClient != '1')
                {
                    printf("Veuillez entrer un choix valide s'il vous plait : ");
                }
                else{
                    break;
                }
            }
        }
        else{
            break;
        }
    }

    printf("Bienvenue a la salle, merci de rester polit et de respecter les autres membres. Si vous voulez quitter, tappez 'Quitter'\n");


    pthread_t thread_env, thread_rec;
    if(pthread_create(&thread_env, NULL, envoyerMessage, (void*)(int long)s) != 0){
        printf("Erreur creation thread");
        exit(1);
    }
    if(pthread_create(&thread_rec, NULL, recevoirMessage, (void*)(int long)s) != 0){
        printf("Erreur creation thread recevoir");
        exit(1);
    }

    sem_wait(&sema);

    close(s);
    free(addr_serveur);
    sem_destroy(&sema);

    return 0;
}
