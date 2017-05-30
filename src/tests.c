#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pse.h"

int main(void)
{
    int fichier;
    int compteur = 0;
    char tableau[1000][1000];
    system("ping 81.8.8.8 -c 4 > test");
    fichier = open("test", O_RDONLY);
    while(lireLigne(fichier, tableau[compteur]))
    {
        compteur += 1;
    }
    printf("%c\n", tableau[compteur - 2][35]);
}
