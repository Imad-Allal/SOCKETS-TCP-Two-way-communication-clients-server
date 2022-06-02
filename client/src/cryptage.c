#include<stdio.h>
#include<crypt.h>
#include<stdlib.h>
#include<string.h>

int main(){

    FILE *fic = fopen("file.txt", "r+");

    char md5[] = "$1$";
    char txt[] = "Hello";
    char *res;
    char mot[256];
    char crypted[256];

    res = crypt(txt, md5);
    printf("Le hache avec md5sum = %s\n", res);
    printf("Position du curseur : %d\n", ftell(fic)); // test

    fputs(res, fic);

    printf("Position du curseur : %d\n", ftell(fic));//test

    printf("Entrez un mot : ");
    scanf("%s",mot);

    res = crypt(mot, md5);
    printf("Le hache avec md5sum = %s\n", res);
    fseek(fic,0,SEEK_SET);// tres important: permet de mettre 

    printf("Position du curseur : %d\n", ftell(fic));//test
    if (fgets(crypted, 255, fic) == NULL)
    {
        printf("Error");
        exit(1);
    }

    printf("crypted = %s \n", crypted);
    if (strcmp(crypted,res) == 0)
        printf("Succes");
    else
        printf("Echec");

            
    fclose(fic);
    return 0;
}
