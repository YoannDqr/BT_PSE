#include "pse.h"
#include "client.h"

int main(int argc, char *argv[])
{
    int choix;
    int compteur_thread = 0;
    int k;
    pthread_t *tab_thread = malloc(sizeof(pthread_t) * 100);
    Client *tab_client = malloc(sizeof(Client) * 100);

	for(k = 0; k < 100; k++)
	{
		taille_local[k] = 0;
	}
    printf("Télécharger une torrent : 1\nSupprimer un torrent : 2\n");
    scanf("%d", &choix);

    if(choix == 1)
    {
        int fichier_existe = -1;
        while(fichier_existe < 0)
        {
            printf("Nom du fichier à télécharger : ");
            scanf("%s", tab_client[compteur_thread].tracker.nom_fichier);
			strcpy(tab_client[compteur_thread].tracker.buff, tab_client[compteur_thread].tracker.nom_fichier);
			strcat(tab_client[compteur_thread].tracker.buff, ".torrent");
            tab_client[compteur_thread].fd_tracker = open(tab_client[compteur_thread].tracker.buff, O_RDONLY);
            if(tab_client[compteur_thread].fd_tracker < 0)
            {
                printf("Fichier introuvable\n");
            }
            else
            {
                compteur_thread += 1;
				fichier_existe = tab_client[compteur_thread].fd_tracker;
            }
        }
        printf("Dossier de téléchargement : ");
        scanf("%s", tab_client[compteur_thread - 1].dossier_dl);
		printf("Addresse IP de votre machine : ");
		scanf("%s", tab_client[compteur_thread - 1].adresse_ip);
		tab_client[compteur_thread - 1].numero_thread = compteur_thread - 1;
		pthread_create (&tab_thread[compteur_thread - 1], NULL, telechargement, &tab_client[compteur_thread - 1]);
    }

    if(choix == 2)
    {
        char dossier[200];
        char fichier[200];
        char commande[10000];
        printf("Nom du fichier a supprimer : ");
        scanf("%s", fichier);
        printf("Dossier du fichier : ");
        scanf("%s", dossier);

        strcpy(commande, "rm ");
        strcat(commande, dossier);
        strcat(commande, fichier);
        strcat(commande, "*.part");

    }

    for(k = 0; k < compteur_thread; k++)
    {
        pthread_join(tab_thread[k], NULL);
    }
    return EXIT_SUCCESS;
}
