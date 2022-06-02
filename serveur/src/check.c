#include <stdio.h>
#include <crypt.h>
#include <stdlib.h>
#include <string.h>
#include "../include/check.h"


int checkNom(char* nom){

    char nomRecup[256]; 
    FILE *fic = fopen("passwords.txt", "a+");
    if(fic == NULL){
        printf("Erreur: Impossible d'ouvrir le fichier dans checkNom");
        exit(1);
    }
    
    fseek(fic, 0, SEEK_SET);
    while(fscanf(fic,"%s",nomRecup) != EOF){
        //printf("Nom : %s\n",nomRecup);
        if (strcmp(nom, nomRecup) == 0)
        {
            printf("Nom disponible dans la liste\n");
            fclose(fic);
            return 1;
        }
    }
    fclose(fic);
    return 0;
}

int checkMdp(char* mdp, char *nom){

    char nomRecup[256]; 
    char mdpRecup[256]; 
    FILE *fic = fopen("passwords.txt", "a+");
    if(fic == NULL){
        printf("Erreur: Impossible d'ouvrir le fichier dans checkMdp");
        exit(1);
    }
    //printf("Mot de passe client : %s\n",mdp);
    fseek(fic, 0, SEEK_SET);
    while(fscanf(fic,"%s %s",nomRecup, mdpRecup) != EOF){
        //printf("Nom : %s, Mot de passe = %s\n",nomRecup, mdpRecup);
        if (strcmp(mdp, mdpRecup) == 0 && strcmp(nom, nomRecup) == 0)
        {

            printf("Client valide\n");
            fclose(fic);
            return 1;
        }
    }
    fclose(fic);
    printf("Client invalide\n");
    return 0;
}

int ajouterClient(char *nomClient, char *mdpClient){

    FILE *fic = fopen("passwords.txt", "a+");
    if(fic == NULL){
        printf("Erreur: Impossible d'ouvrir le fichier dans ajouterClient");
        exit(1);
    }    
    if(fprintf(fic,"%s\t\t\t\t%s\n",nomClient, mdpClient) < 0){
        printf("Erreur aout du client au serveur\n");
        exit(1);
    }
    fclose(fic);
    return 1;
}


/*
La fonction checkNom prend en parametre le nom du client (entré par le client), et verifie s'il est déja inscrit sur la base de données ou pas 

Deja inscrit ? 
OUI : Renvoi 1 ==> Si le client avait décidé de s'incrire, 1 sera interpreté comme étant une erreur d'indisponibilité du nom, du au fait que le nom est deja disponible dans la BDD, et qu'il ne pourra pas l'utiliser pour s'inscrire. Sinon, il sera interpreté comme étant un indicateur de disponibilité du client dans la BDD ==> le client n'aura plus qu'à entrer son mot de passe valide.

NON : Renvoi 0 ==> Le client devra s'inscrire (car son nom n'est pas disponible dans la BDD).
*/