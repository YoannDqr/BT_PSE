#include "pse.h"

typedef struct
{
	int somme;
	int numero_thread;
}Worker;

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
    int indice;
    int taille_local;
    pthread_t tab_fichier_dl[100];
    int compteur_dl;
	int connection;
	int fd_fichier;
    Tracker tracker; //infos du tracker
}Client;

void* telechargement(void*);
void info_tracker(int fd_tracker, Tracker *tracker);
void meilleur_peer(Tracker *tracker);
void recuperation_local(Client *client);
int est_dans(int valeur, int *tableau_valeur, int taille_tableau);
void* dl_fichier(void* donnes);
int usee(int *tab_local,int *tab_server, int taille_local, int taille_server);
void change_peer(Client *client);
void connection_socket(Client *client);

void connection_socket(Client *client)
{
	sprintf(client->tracker.port,"%d", 9091);
	client->tracker.meilleur_peer = 0;
	client->tracker.socket_client = socket(AF_INET,SOCK_STREAM,0);
	//client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], "9091");
	client->tracker.socket_serveur = resolv("172.17.1.102", "9091");
	if(client->tracker.socket_client == -1 || client->tracker.socket_serveur == NULL)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	client->connection = connect(client->tracker.socket_client, (struct sockaddr *)client->tracker.socket_serveur, sizeof(struct sockaddr_in));
	if(client->connection < 0)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}
//################## Déportation de port############################################
	lireLigne(client->tracker.socket_client, client->var);
	printf("Déporté sur le port %s\n", client->var);

	client->tracker.socket_client = socket(AF_INET,SOCK_STREAM,0);
	//client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], "9091");
	client->tracker.socket_serveur = resolv("172.17.1.102", client->var);
	if(client->tracker.socket_client == -1 || client->tracker.socket_serveur == NULL)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	client->connection = connect(client->tracker.socket_client, (struct sockaddr *)client->tracker.socket_serveur, sizeof(struct sockaddr_in));
	if(client->connection < 0)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}
//###################################################################################
}

int usee(int *tab_local,int *tab_server, int taille_local, int taille_server)
{
	int k;
	for(k = 0; k < taille_server; k++)
	{
		if(est_dans(tab_server[k], tab_local, taille_local) == 0)
		{
			return 0;
		}
	}
	return 1;
}

void* telechargement(void* donnes)
{
    Client *client = (Client *)donnes;
    int k;
    info_tracker(client->fd_tracker, &client->tracker);
    //meilleur_peer(&client->tracker);
    recuperation_local(client);
    client->compteur_dl = 0;
    srand(time(NULL));
    //while(client->taille_local != client->tracker.taille_fichier) //condition a modifier
    //{
		/*if(usee(client->fichier_local, client->tracker.ficheir_dispo[client->tracker.meilleur_peer], client->nb_fichier_local, client->tracker.nb_fichier[client->tracker.meilleur_peer]) == 0)
			meilleur_peer(&client->tracker);*/
        client->indice = rand()%client->tracker.nb_fichier[client->tracker.meilleur_peer];
        while(est_dans(client->tracker.fichier_dispo[client->tracker.meilleur_peer][client->indice], client->fichier_local, client->nb_fichier_local))
        {
            client->indice = rand()%client->tracker.nb_fichier[client->tracker.meilleur_peer];
        }
        pthread_create(&client->tab_fichier_dl[client->compteur_dl], NULL, dl_fichier, client);
        client->compteur_dl += 1;
    //}

    for(k = 0; k < client->compteur_dl; k++)
    {
        pthread_join(client->tab_fichier_dl[k], NULL);
    }
    pthread_exit(NULL);
}

void* dl_fichier(void* donnes)
{
    Client *client = (Client*)donnes;
	connection_socket(client);
    sprintf(client->var, "%s%d.part", client->tracker.nom_fichier, client->tracker.fichier_dispo[client->tracker.meilleur_peer][client->indice]);
    ecrireLigne(client->tracker.socket_client, client->var); //Envoie le fichier a dl
    lireLigne(client->tracker.socket_client, client->var); //Le fichier est-il présent dans le serveur ? 0 non 1 oui
	printf("***** buff : %s\n", client->var);
    if(strcmp(client->var, "0") == 0)
        pthread_exit(NULL);

	sprintf(client->var, "%s%s%d.part", client->dossier_dl, client->tracker.nom_fichier, client->indice);
	client->fd_fichier = open(client->var, O_TRUNC|O_WRONLY|O_CREAT, 0777);
    if(client->fd_fichier < 0)
    {
        perror("fd_local error");
        exit(EXIT_FAILURE);
    }
	lireLigne(client->tracker.socket_client, client->tracker.buff);
    while(strcmp(client->tracker.buff, "@@@@!!@!//785") != 0)
    {
		printf("Il est ecris : %s \n", client->tracker.buff);
        ecrireLigne(client->fd_fichier, client->tracker.buff);
		lireLigne(client->tracker.socket_client, client->tracker.buff);
    }
    client->fichier_local[client->nb_fichier_local] = client->tracker.fichier_dispo[client->tracker.meilleur_peer][client->indice];
    client->nb_fichier_local += 1;
    //maj(client);
    close(client->tracker.socket_client);
    strcpy(client->var,client->tracker.nom_fichier);
	sprintf(client->var, "%s%s.part", client->dossier_dl, client->tracker.nom_fichier);
    client->fd_local = open(client->var, O_WRONLY|O_CREAT, 0777);
    if(client->fd_local < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    sprintf(client->var, "%d\n", client->indice); //maj des fichiers locals
	printf("Numero du fichier : %s", client->var);
    ecrireLigne(client->fd_local, client->var);
	client->taille_local += 11; //taille d'un ficheir split
    close(client->fd_local);
	close(client->fd_fichier);
    pthread_exit(NULL);
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
    client->fd_local = open(client->var, O_RDONLY);
    client->tracker.var = 0;
	client->taille_local = 0;
    while(lireLigne(client->fd_local, client->var) && client->fd_local > 0)
    {
        client->fichier_local[client->tracker.var] = atoi(client->var);
        client->tracker.var += 1;
    }
    client->nb_fichier_local = client->tracker.var;
}

void meilleur_peer(Tracker *tracker)
{
    tracker->tableau_peer = malloc(sizeof(float) * tracker->nb_ip);
    int k;
    tracker->var = -2;
    tracker->meilleur_peer = -1;
    for(k = 0; k < tracker->nb_ip; k++)
    {
        sprintf(tracker->port,"%d", 9090);
        tracker->socket_client = socket(AF_INET,SOCK_STREAM,0);
        tracker->socket_serveur = resolv(tracker->ip[k], "9090");
        if(tracker->socket_client == -1 || tracker->socket_serveur == NULL)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        int connection = connect(tracker->socket_client, (struct sockaddr *)tracker->socket_serveur, sizeof(struct sockaddr_in));
        if(connection < 0)
        {
            perror("connect");
            exit(EXIT_FAILURE);
        }

        ecrireLigne(tracker->socket_client, tracker->nom_fichier);
        lireLigne(tracker->socket_client, tracker->buff);
        tracker->tableau_peer[k] = atol(tracker->buff);
    }
    tracker->var = tracker->tableau_peer[0];
    tracker->meilleur_peer = 0;
    for(k = 0; k < tracker->nb_ip; k++)
    {
        if(tracker->tableau_peer[k] > tracker->var)
        {
            tracker->var = tracker->tableau_peer[k];
            tracker->meilleur_peer = k;
        }
    }
    close(tracker->socket_client);
    free(tracker->tableau_peer);
}

void info_tracker(int fd_tracker, Tracker *tracker)
{
    int compteur = 0;
    int k;
    lireLigne(fd_tracker, tracker->buff);
    strcpy(tracker->nom_fichier, tracker->buff);
	strcpy(tracker->buff, "");
	lireLigne(fd_tracker, tracker->buff);
	tracker->taille_fichier = atoi(tracker->buff);
    //printf("nom fichier : %s\n", tracker->buff);

    while(strcmp(tracker->buff,"...") != 0)
    {
        int numero_fichier = 0;
        int compteur_fichier = 0;
		strcpy(tracker->buff, "");
		lireLigne(fd_tracker, tracker->buff);
		//printf("ip : %s\n", tracker->buff);
        strcpy(tracker->ip[compteur], tracker->buff);
        strcpy(tracker->buff, "");
		lireLigne(fd_tracker, tracker->buff);
		//printf("nombre de fichier : %s\n", tracker->buff);
		tracker->nb_fichier[compteur] = atoi(tracker->buff);
		strcpy(tracker->buff,"");
        lireLigne(fd_tracker, tracker->buff);

        for(k = 0; k < strlen(tracker->buff); k++)
        {
            if(tracker->buff[k] == ',' && numero_fichier != 0)
            {
                tracker->fichier_dispo[compteur][compteur_fichier] = numero_fichier;
				//printf("fichier numero : %d\n", numero_fichier);
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

int main(int argc, char *argv[])
{
    int choix;
    int compteur_thread = 0;
    int k;
    pthread_t *tab_thread = malloc(sizeof(pthread_t) * 100);
    Client *tab_client = malloc(sizeof(Client) * 100);
    printf("Télécharger une torrent : 1\nSupprimer un torrent : 2\n Créer un torrent : 3\n");
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
		pthread_create (&tab_thread[compteur_thread - 1], NULL, telechargement, &tab_client[compteur_thread - 1]);
    }

    for(k = 0; k < compteur_thread; k++)
    {
        pthread_join(tab_thread[k], NULL);
    }
    return EXIT_SUCCESS;
}
