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
    char port[20];
    int meilleur_peer;
    float *tableau_peer;
	char chemin_serveur[1000]; //emplacement du fichier dans les serveur
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
    FILE *fd_local; //fd contenant le fichier résumant les donné déjà téléchargé
    int indice;
    int taille_local;
    pthread_t tab_fichier_dl[100];
    int compteur_dl;
	int connection;
	int fd_fichier;
	int k;
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
	client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], "9091");
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
	ecrireLigne(client->tracker.socket_client, "1\n");
	client->tracker.socket_client = socket(AF_INET,SOCK_STREAM,0);
	//client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], "9091");
	client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], client->var);
	if(client->tracker.socket_client == -1 || client->tracker.socket_serveur == NULL)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	printf("port : %s\n", client->var);
	client->connection = connect(client->tracker.socket_client, (struct sockaddr *)client->tracker.socket_serveur, sizeof(struct sockaddr_in));
	printf("Déporté sur le port %s\n", client->var);
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
    info_tracker(client->fd_tracker, &client->tracker);
	Client *tableau_dl = malloc(sizeof(Client) * 50); //dl de 100 blocks en meme temps uniquement
    //meilleur_peer(&client->tracker);
    recuperation_local(client);
    client->compteur_dl = 0;
    srand(time(NULL));

    while(client->compteur_dl < client->tracker.taille_fichier/11) //condition a modifier
    {
		memcpy(&tableau_dl[client->compteur_dl], client, sizeof(Client));
		/*if(usee(client->fichier_local, client->tracker.fichier_dispo[client->tracker.meilleur_peer], client->nb_fichier_local, client->tracker.nb_fichier[client->tracker.meilleur_peer]) == 0)
			meilleur_peer(&client->tracker);*/
        //tableau_dl[client->compteur_dl].indice = rand()%client->tracker.nb_fichier[client->tracker.meilleur_peer];
		tableau_dl[client->compteur_dl].indice = client->compteur_dl + 1;
        /*while(est_dans(client->tracker.fichier_dispo[client->tracker.meilleur_peer][client->indice], client->fichier_local, client->nb_fichier_local))
        {
            tableau_dl[client->compteur_dl].indice = rand()%client->tracker.nb_fichier[client->tracker.meilleur_peer];
			printf("a\n");
        }*/
		tableau_dl[client->compteur_dl].indice = client->tracker.fichier_dispo[client->tracker.meilleur_peer][tableau_dl[client->compteur_dl].indice - 1];
		connection_socket(&tableau_dl[client->compteur_dl]);
        pthread_create(&client->tab_fichier_dl[client->compteur_dl], NULL, dl_fichier, &tableau_dl[client->compteur_dl]);
		client->taille_local += 11;
		sprintf(client->var, "%s%s.part", client->dossier_dl, client->tracker.nom_fichier);
		client->fd_local = fopen(client->var, "a");
		fprintf(client->fd_local, "%d\n", tableau_dl[client->compteur_dl].indice);
		fclose(client->fd_local);


		//client->fichier_local[client->nb_fichier_local] = client->tracker.fichier_dispo[client->tracker.meilleur_peer][client->indice];
		client->nb_fichier_local += 1;
        client->compteur_dl += 1;
		printf("&&%d\n", client->compteur_dl);
    }
	for(client->k = 0; client->k <= client->compteur_dl + 5; client->k++)
	{
		pthread_join(client->tab_fichier_dl[client->compteur_dl], NULL);
	}
	printf("&&b\n");
    pthread_exit(NULL);
}

void* dl_fichier(void* donnes)
{
    Client *client = (Client*)donnes;
	//sleep(3);
    sprintf(client->var, "%s%s%d.part\n", client->tracker.chemin_serveur, client->tracker.nom_fichier, client->indice);
	printf("*!!%d\n", client->indice);
    ecrireLigne(client->tracker.socket_client, client->var); //Envoie le fichier a dl
    lireLigne(client->tracker.socket_client, client->var); //Le fichier est-il présent dans le serveur ? 0 non 1 oui
	printf("***** buff : %s\n", client->var);
    if(strcmp(client->var, "0\n") == 0)
	{
		printf("aaaapppp\n");
        pthread_exit(NULL);
	}

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
		//printf("Il est ecris : %d \n", client->indice);
        ecrireLigne(client->fd_fichier, client->tracker.buff);
		lireLigne(client->tracker.socket_client, client->tracker.buff);
    }
	ecrireLigne(client->tracker.socket_client, "1\n");
    //maj(client);
    sprintf(client->var, "%d\n", client->indice); //maj des fichiers locals
	printf("Numero du fichier : %s", client->var);

	close(client->fd_fichier);
	printf("fichier %d fermé\n", client->indice);
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
    sprintf(client->var, "%s%s.part", client->dossier_dl, client->tracker.nom_fichier);
    client->fd_local = fopen(client->var, "r");
	if(client->fd_local == NULL)
	{
		client->fd_local = fopen(client->var, "w");
		fclose(client->fd_local);
		client->fd_local = fopen(client->var, "r");
	}
    client->tracker.var = 0;
	client->taille_local = 0;
    while(fscanf(client->fd_local, "%d", &client->fichier_local[client->tracker.var]) > 0 && client->fd_local > 0)
        client->tracker.var += 1;
    client->nb_fichier_local = client->tracker.var;
	fclose(client->fd_local);
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
        tracker->tableau_peer[k] = tracker->nb_fichier[k]/atol(tracker->buff);
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
	printf("taille: fichier = %d\n", tracker->taille_fichier);
    printf("nom fichier : %s\n", tracker->buff);

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
		strcpy(tracker->chemin_serveur, tracker->buff);
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
