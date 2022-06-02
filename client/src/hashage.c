#include <stdio.h>
#include <crypt.h>
#include <stdlib.h>

char* genererHash(char *mdpClient){

    char md5[] = "$1$";
    char *res;
    
    res = crypt(mdpClient, md5);
    return res; 
}