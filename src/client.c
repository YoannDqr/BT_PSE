#include "pse.h"

typedef struct
{
    char nom_fichier[100]; //nom du fichier à télécharger
    char ip[20][100]; //ip des dépots
    int fichier_dispo[100][100]; //fichier dispos dans le dépot k
    int nb_fichier[100]; //nb de fichier dispo dans le dépot k
    int taille_fichier; //taille total du fichier à télécharger
    char buff[200]; //buffer pour lire les fichier
    int nb_ip; //nb de dépot total
    struct sockaddr_in *socket_serveur;
    int socket_client;
    char port[10];
    int meilleur_peer;
    float *tableau_peer;
    int var;
}Tracker;

typedef struct
{
    int fd_tracker; //fd du tracker
    int nb_fichier; //nombre de fichier constituant le fichier à télécharger
    int nb_dl; //nombre de fichié téléchargé
    int fichier_local[100]; //numéro des block déjà téléchargé
    int nb_fichier_local; //nombre des fichier en local
    char dossier_dl[200]; //dossier de téléchargement
    char var[2000]; //chaine de caractère variable
    int fd_local; //fd contenant le fichier résumant les donné déjà téléchargé
    Tracker tracker; //infos du tracker
}Client;

void info_tracker(int fd_tracker, Tracker *tracker);
void client(void *donnees);
void meilleur_peer(Tracker *tracker);
//int test_vie(char *ip);
void recuperation_local(Client *client);
int est_dans(int valeur, int *tableau_valeur, int taille_tableau);
int telecharger(Client *client, int fichier_dl);


int telecharger(Client *client, int fichier_dl)
{
    sprintf(client->tracker.port,"%d", 9091);
    client->tracker.socket_client = socket(AF_INET,SOCK_STREAM,0);
    client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], client->tracker.port);
    if(client->tracker.socket_client == -1 || client->tracker.socket_serveur == NULL)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int connection = connect(client->tracker.socket_client, (struct sockaddr *)&client->tracker.socket_serveur, sizeof(client->tracker.socket_serveur));
    if(connection == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    sprintf(client->var, "%s%d.part", client->tracker.nom_fichier, fichier_dl);
    ecrireLigne(client->tracker.socket_client, client->var);
    client->fd_local = open(client->var, O_TRUNC|O_WRONLY|O_CREAT, 0777);
    if(client->fd_local < 0)
    {
        perror("fd_local error");
        exit(EXIT_FAILURE);
    }
    while(lireLigne(client->tracker.socket_client, client->tracker.buff) > 0)
    {
        ecrireLigne(client->fd_local, client->tracker.buff);
    }
    client->fichier_local[client->nb_fichier_local] = fichier_dl;
    client->nb_fichier_local += 1;
    //maj(client);
    close(client->tracker.socket_client);
    return EXIT_SUCCESS;
}

int est_dans(int valeur, int *tableau_valeur, int taille_tableau)
{
    int k;
    int yest = 0;
    for(k = 0; k < taille_tableau; k++)
    {
        if(tableau_valeur[k] == valeur)
        {
            yest = 1;
        }
    }
    return yest;
}

void recuperation_local(Client *client)
{
    strcpy(client->var,client->tracker.nom_fichier);
    strcat(client->var, ".part");
    client->fd_local = open(client->var, O_RDONLY|O_CREAT);
    client->tracker.var = 0;
    while(lireLigne(client->fd_local, client->var))
    {
        client->fichier_local[client->tracker.var] = atoi(client->var);
        client->tracker.var += 1;
    }
    client->nb_fichier_local = client->tracker.var;
}

/*int test_vie(char *ip)
{*************************************************mettre un mutex ici****************************************************************
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
}*/

void meilleur_peer(Tracker *tracker)
{
    tracker->tableau_peer = malloc(sizeof(float) * tracker->nb_ip);
    int k;
    tracker->var = -2;
    tracker->meilleur_peer = -1;
    for(k = 0; k < tracker->nb_ip; k++)
    {
        /*if(test_vie(tracker->ip[k]))
        {*/
        sprintf(tracker->port,"%d", 9090);
        tracker->socket_client = socket(AF_INET,SOCK_STREAM,0);
        tracker->socket_serveur = resolv(tracker->ip[k], tracker->port);
        if(tracker->socket_client == -1 || tracker->socket_serveur == NULL)
        {
            tracker->tableau_peer[k] = -1;
            perror("socket");
            exit(EXIT_FAILURE);
        }
        int connection = connect(tracker->socket_client, (struct sockaddr *)&tracker->socket_serveur, sizeof(tracker->socket_serveur));
        if(connection == -1)
        {
            tracker->tableau_peer[k] = -1;
            perror("connect");
            exit(EXIT_FAILURE);
        }
        ecrireLigne(tracker->socket_client, tracker->nom_fichier);
        lireLigne(tracker->socket_client, tracker->buff);
        tracker->tableau_peer[k] = atol(tracker->buff);
        /*}
        else
        {
        }*/
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
    int k;
    Client *donnes = (Client *)donnees;
    info_tracker(donnes->fd_tracker, &donnes->tracker);
    meilleur_peer(&donnes->tracker);
    for(k = 0; k < donnes->tracker.nb_fichier[donnes->tracker.meilleur_peer]; k++)
    {
        if(est_dans(donnes->tracker.fichier_dispo[donnes->tracker.meilleur_peer][k], donnes->fichier_local, donnes->nb_fichier_local) == 0)//Si le fichier doit etre téléchargé et est présent chez le serveur, aors le télécharger
        {
            telecharger(donnes, donnes->tracker.fichier_dispo[donnes->tracker.meilleur_peer][k]);
        }
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
