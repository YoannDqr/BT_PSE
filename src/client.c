#include "pse.h"

typedef struct
{
    char nom_fichier[100];
    char ip[20][100];
    int fichier_dispo[100][100];
    int nb_fichier[100];
    int taille_fichier;
    char buff[200];
    int nb_ip;
    struct sockaddr_in *socket_serveur;
    int socket_client;
    char port[10];
    int meilleur_peer;
    float *tableau_peer;
    int var;
}Tracker;

typedef struct
{
    int fd_tracker;
    int nb_fichier;
    int nb_dl;
    int fichier_local[100];
    int nb_fichier_local;
    char dossier_dl[200];
    char var[2000]
    int fichier_local[100];
    Tracker tracker;
}Client;

void info_tracker(int fd_tracker, Tracker *tracker);
void client(void *donnees);
void meilleur_peer(Tracker *tracker);
int test_vie(char *ip);
void recuperation_local(Client *client);

void recupération_local(Client *client)
{
    int fd;
    int compteur = 0;
    int k;
    char tableau_fichier[200][200];
    strcpy(client->var, "ls -l ");
    strcat(client->var, dossier_local);
    strcat(client->var, client->tracker.nom_fichier);
    strcat(client->var, "?.part > .temp2");
    fd = open(".temp2", O_RDONLY);
    while(lireLigne(fd, client->var) > 0)
    {
        strcpy(tableau_fichier, client->var);
        compteur += 1;
    }
    close(fd);
    for(k = 0; k < compteur; k++)
    {
        client->fichier_local[k] = (int)tableau_fichier[strlen(client->tracker.nom_fichier)] - 48;//*******************************************************
    }


}

int test_vie(char *ip)
{//*************************************************mettre un mutex ici****************************************************************
    char cmd[200] = "ping ";
    char tableau[20][500];
    int fd;
    int compteur = 0;
    strcat(cmd, ip);
    strcat(cmd, " -c 4 > .temp");
    system(cmd);
    fd = open(".temp", O_RDONLY);
    while(lireLigne(fd, tableau[compteur]) > 0)
    {
        compteur += 1;
    }
    if(tableau[compteur-2][37] == '0')
        return 0;
    else
        return 1;
}

void meilleur_peer(Tracker *tracker)
{
    tracker->tableau_peer = malloc(sizeof(float) * tracker->nb_ip);
    int k;
    tracker->var = -2;
    tracker->meilleur_peer = -1;
    for(k = 0; k < tracker->nb_ip; k++)
    {
        if(test_vie(tracker->ip[k]))
        {
            sprintf(tracker->port,"%d", 9090);
            tracker->socket_client = socket(AF_INET,SOCK_STREAM,0);
            tracker->socket_serveur = resolv(tracker->ip[k], tracker->port);
            if(tracker->socket_client == -1 || tracker->socket_serveur == NULL)
            {
                perror("socket");
                exit(EXIT_FAILURE);
            }
            int connection = connect(tracker->socket_client, (struct sockaddr *)&tracker->socket_serveur, sizeof(tracker->socket_serveur));
            if(connection == -1)
            {
                perror("connect");
                exit(EXIT_FAILURE);
            }
            ecrireLigne(tracker->socket_client, tracker->nom_fichier);
            lireLigne(tracker->socket_client, tracker->buff);
            tracker->tableau_peer[k] = atol(tracker->buff);
        }
        else
        {
            tracker->tableau_peer[k] = -1;
        }
    }
    for(k = 0; k < tracker->nb_ip; k++)
    {
        if(tracker->tableau_peer[k] > tracker->var)
        {
            tracker->var = tracker->tableau_peer[k];
            tracker->meilleur_peer = k;
        }
    }
    close(tracker->socket_client);
    tracker->meilleure_peer = indice;
}

void info_tracker(int fd_tracker, Tracker *tracker)
{
    int compteur = 0;
    int k;
    lireLigne(fd_tracker, tracker->buff);
    strcpy(tracker->nom_fichier, tracker->buff);
    strcpy(tracker->buff, "");
    lireLigne(fd_tracker, tracker->buff);
    while(strcmp(tracker->buff,"...") != 0)
    {
        int numero_fichier = 0;
        int compteur_fichier = 0;
        strcpy(tracker->ip[compteur], tracker->buff);
        strcpy(tracker->buff, "");
        lireLigne(fd_tracker, tracker->buff);
        for(k = 0; k < strlen(tracker->buff); k++)
        {
            if(tracker->buff[k] == ',' && numero_fichier != 0)
            {
                tracker->fichier_dispo[compteur][compteur_fichier] = numero_fichier;
                compteur_fichier += 1;
                numero_fichier = 0;
            }
            else
            {
                numero_fichier = 10*numero_fichier + (int)tracker->buff[k] - 48;
            }
        }
        tracker->nb_fichier[compteur] = compteur_fichier;
        compteur += 1;
        lireLigne(fd_tracker, tracker->buff);
    }
    tracker->nb_ip = compteur;
}
void client(void *donnees)
{
    Client *donnes = (Client *)donnees;
    info_tracker(donnes->fd_tracker, &donnes->tracker);
    meilleur_peer(donnes->tracker);
    for(k = 0; k < donnes->nb_fichier; k++)
    {
        //Si le fichier doit etre téléchargé et est présent chez le serveur, aors le télécharger
    }

}

int main(void)
{
    Tracker tracker;
    int fichier = open("tracker.torrent", O_RDONLY);
    int k;
    int i;
    if(fichier == -1)
        erreur_IO("read");
    info_tracker(fichier, &tracker);
    for(k = 0; k < tracker.nb_ip; k++)
    {
        printf("%s\n", tracker.ip[k]);
        for(i = 0; i < tracker.nb_fichier[k]; i++)
        {
            printf("%d,", tracker.fichier_dispo[k][i]);
        }
        printf("\n");
    }
    exit(EXIT_SUCCESS);
}
