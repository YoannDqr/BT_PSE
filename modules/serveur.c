#include "pse.h"
#include "serveur.h"

void* maj_tracker(void* donnes)
{
    Server *server = (Server *)donnes;
    server->canal = accept(server->socket, (struct sockaddr*)&server->socket_client, &server->len);

    if(server->canal == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
//###############################Deportage de port################################################
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server->socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server->socket_server.sin_family = AF_INET;
    server->socket_server.sin_port = htons((short)atoi(server->port));
    server->socket_server.sin_addr.s_addr = INADDR_ANY;

    if(bind(server->socket, (struct sockaddr*)&server->socket_server, sizeof(server->socket_server)) != 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if(listen(server->socket, 1) != 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    server->len = sizeof(server->socket_client);
    ecrireLigne(server->canal, server->port);
    strcpy(server->buff, "");
    lireLigne(server->canal, server->buff);

    close(server->canal);
    server->canal = accept(server->socket, (struct sockaddr*)&server->socket_client, &server->len);

    if(server->canal == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    lireLigne(server->canal, server->buff);
    strcat(server->buff, ".torrent");
    server->fd_tracker = fopen(server->buff, "a");
    if(server->fd_tracker == NULL);
        ecrireLigne(server->canal, "0\n");
    ecrireLigne(server->canal, "1\n");
    strcpy(server->buff,"");
    lireLigne(server->canal, server->buff);
    fprintf(server->fd_tracker, "%s\n", server->buff);
    ecrireLigne(server->canal, " \n");
    strcpy(server->buff,"");
    lireLigne(server->canal, server->buff);
    fprintf(server->fd_tracker, "%s\n", server->buff);
    ecrireLigne(server->canal, " \n");
    strcpy(server->buff,"");
    lireLigne(server->canal, server->buff);
    fprintf(server->fd_tracker, "%s\n", server->buff);
    ecrireLigne(server->canal, " \n");
    strcpy(server->buff,"");
    lireLigne(server->canal, server->buff);
    fprintf(server->fd_tracker, "%s\n", server->buff);
    ecrireLigne(server->canal, " \n");

    fclose(server->fd_tracker);
    close(server->canal);
    pthread_exit(NULL);
}

int est_dans(int valeur, int *tab_valeur, int taille_tab)
{
    int k;
    for(k = 0; k < taille_tab; k++)
    {
        if(valeur == tab_valeur[k])
            return 1;
    }
    return 0;
}
void up_donnes(Server *server)
{    /*Upload lun fichier demandé par un cleint*/
    strcpy(server->buff, "");
    lireLigne(server->canal, server->buff);/*Quelle est le fichier à téléchargé ?*/
    server->fd_fichier = open(server->buff, O_RDONLY);
    printf("nom_fichier : %s\nfd_fichier = %d\n", server->buff, server->fd_fichier);
    if(server->fd_fichier < 0)
        ecrireLigne(server->canal, "0\n");/*Je ne l'ai pas*/
    else
    {
        ecrireLigne(server->canal, "1\n"); /*Youpi je l'ai*/
    }
    if(server->fd_fichier > 0)
    {
        while(lireLigne(server->fd_fichier, server->buff))
        {
            ecrireLigne(server->canal, server->buff);
            printf("Il est ecris : %s\n", server->buff);
            strcpy(server->buff, "");
        }
        ecrireLigne(server->canal, "@@@@!!@!//785\n");
        do{
            strcpy(server->buff, "");
            lireLigne(server->canal, server->buff);
        }while(strcmp(server->buff, "1") != 0);

    }
    close(server->canal);

}

void* upload(void *donnees)
{
    up_donnes((Server*)donnees);
    pthread_exit(NULL);
}
