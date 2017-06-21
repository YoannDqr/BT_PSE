#include "pse.h"
#include "client.h"


void maj_tracker(Client* client)
{	/*Rajoute un block dans le tracker du torrent et l'envoie au serveur. Cela à pour but de transmettre le tracker actualisé dans le réseau
	lorsque qu'un client se connecte au serveur, son tracker est actualisé avec celui du serveur*/
	connection_socket(client, 9092);
	sprintf(client->var, "%s\n", client->tracker.nom_fichier);
	ecrireLigne(client->tracker.socket_client, client->var);
	do{
		lireLigne(client->tracker.socket_client, client->var);
	}while(strcmp(client->var, "1") != 0);
	strcpy(client->var, "");
	for(client->k = 0; client->k < client->nb_fichier_local; client->k++)
	{
		sprintf(client->tracker.buff, "%d,", client->fichier_local[client->k]);
		strcat(client->var, client->tracker.buff);
	}
	strcat(client->var, "\n");
	ecrireLigne(client->tracker.socket_client, client->adresse_ip);
	lireLigne(client->tracker.socket_client, client->tracker.buff);/*reprend la syntaxe du fichier .torrent*/
	ecrireLigne(client->tracker.socket_client, client->dossier_dl);
	lireLigne(client->tracker.socket_client, client->tracker.buff);
	sprintf(client->tracker.buff, "%d\n", client->nb_fichier_local);
	ecrireLigne(client->tracker.socket_client, client->tracker.buff);
	lireLigne(client->tracker.socket_client, client->tracker.buff);
	ecrireLigne(client->tracker.socket_client, client->var);
	lireLigne(client->tracker.socket_client, client->tracker.buff);

}
void join(Client *client, char *nom, int nombre_fichier)
{/*Recrée le fichier téléchargé avec les morceau téléchargé*/
	int k;
	client->fichier = fopen(nom, "a");
	if(client->fichier == NULL)
	{
		perror("fopen");
	}
	for(k = 1; k <= nombre_fichier; k++)
	{
		sprintf(client->tracker.nom_fichier, "%s%d.part", nom, k);
		printf("nom fichier : %s\n", client->tracker.nom_fichier);
		client->fichier_split = open(client->tracker.nom_fichier, O_RDONLY);
		if(client->fichier_split < 0)
		{
			perror("open");
		}
		while(lireLigne(client->fichier_split, client->var) > 0)
		{
			fprintf(client->fichier, "%s", client->var);
		}
		close(client->fichier_split);
		//sprintf(client->var, "rm %d",client->tracker.nom_fichier);
		//system(client->var);
	}
	fclose(client->fichier);
}

void connection_socket(Client *client, int port)
{/*Se connecte au serveur spécifié dans client et sur le port port avec un déportage de port automatique*/
	sprintf(client->tracker.port,"%d", port);
	client->tracker.meilleur_peer = 0;
	client->tracker.socket_client = socket(AF_INET,SOCK_STREAM,0);
	client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], client->tracker.port);
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
	lireLigne(client->tracker.socket_client, client->var);/*Récupère le nouveau port auprès du serveur*/
	ecrireLigne(client->tracker.socket_client, "1\n");
	client->tracker.socket_client = socket(AF_INET,SOCK_STREAM,0);
	client->tracker.socket_serveur = resolv(client->tracker.ip[client->tracker.meilleur_peer], client->var);
	if(client->tracker.socket_client == -1 || client->tracker.socket_serveur == NULL)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	client->connection = connect(client->tracker.socket_client, (struct sockaddr *)client->tracker.socket_serveur, sizeof(struct sockaddr_in));
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
{/*Gère le téléchargement d'un fichier
Threadifie les téléchargements afin de les effectuer en parallèle*/
    Client *client = (Client *)donnes;
    info_tracker(client->fd_tracker, &client->tracker);
	Client *tableau_dl = malloc(sizeof(Client) * 50); //dl de 100 blocks en meme temps uniquement
    //meilleur_peer(&client->tracker);
    recuperation_local(client);
    client->compteur_dl = 0;
    srand(time(NULL));
	sprintf(client->var, "%s%s", client->dossier_dl, client->tracker.nom_fichier);
	strcpy(client->var_cte, client->var);
	strcat(client->var, ".part");
	client->fd_local = fopen(client->var, "a");
	taille_local[client->numero_thread] = client->taille_local;
    while(taille_local[client->numero_thread] != client->tracker.taille_fichier/11)
    {
		memcpy(&tableau_dl[client->compteur_dl], client, sizeof(Client));
		//meilleur_peer(&client->tracker);
        tableau_dl[client->compteur_dl].indice = rand()%(client->tracker.nb_fichier[client->tracker.meilleur_peer]);//On choisi un fichier au hasard parmis les fichiers split

		if(est_dans(client->tracker.fichier_dispo[client->tracker.meilleur_peer][tableau_dl[client->compteur_dl].indice], client->fichier_local, client->nb_fichier_local) == 0)
		{
			tableau_dl[client->compteur_dl].indice = client->tracker.fichier_dispo[client->tracker.meilleur_peer][tableau_dl[client->compteur_dl].indice];
			if(tableau_dl[client->compteur_dl].indice * 11 <= client->tracker.taille_fichier)
			{	/*Quelques problème on été rencontré ici car des fois le client demandé des fichiers split qui n'existe pas, pourtant le serveur répondait un contenue pour ce fichiers
				Cette condition evite le téléchargement de ce fichiers parasite*/
				connection_socket(&tableau_dl[client->compteur_dl], 9091);
		        pthread_create(&client->tab_fichier_dl[client->compteur_dl], NULL, dl_fichier, &tableau_dl[client->compteur_dl]);
				client->taille_local += 11; /*Les fichiers font une taille de 11octet*/
				client->fichier_local[client->nb_fichier_local] = tableau_dl[client->compteur_dl].indice;

				client->nb_fichier_local += 1;//On met à jour le tracker avec le nouveau fichier téléchargé
		        client->compteur_dl += 1;
			}
		}
    }
	for(client->k = 0; client->k < client->compteur_dl; client->k++)
	{
		pthread_join(client->tab_fichier_dl[client->compteur_dl], NULL);
		close(tableau_dl[client->k].fd_fichier);
	}
	fclose(client->fd_local);
	maj_tracker(client);
	join(client, client->var_cte, client->tracker.taille_fichier/11);
    pthread_exit(NULL);
}

void* dl_fichier(void* donnes)
{/*Gère le téléchargement d'un block spécifié dans donnes*/
    Client *client = (Client*)donnes;

    sprintf(client->var, "%s%s%d.part\n", client->tracker.chemin_serveur, client->tracker.nom_fichier, client->indice);
    do{
		ecrireLigne(client->tracker.socket_client, client->var); //Envoie le fichier a dl
    	lireLigne(client->tracker.socket_client, client->var); //Le fichier est-il présent dans le serveur ? 0 non 1 oui
    }while(strcmp(client->var, "0") == 0);

	sprintf(client->var, "%s%s%d.part", client->dossier_dl, client->tracker.nom_fichier, client->indice);
	client->fd_fichier = open(client->var, O_TRUNC|O_WRONLY|O_CREAT, 0777);
    if(client->fd_fichier < 0)
    {
        perror("fd_local error");
        exit(EXIT_FAILURE);
    }
	strcpy(client->tracker.buff, "");
	lireLigne(client->tracker.socket_client, client->tracker.buff);
    while(strcmp(client->tracker.buff, "@@@@!!@!//785") != 0)
    {
        ecrireLigne(client->fd_fichier, client->tracker.buff);
		strcpy(client->tracker.buff, "");
		lireLigne(client->tracker.socket_client, client->tracker.buff);
    }
	ecrireLigne(client->tracker.socket_client, "1\n");
    //maj(client);
	strcpy(client->var, "");
    sprintf(client->var, "%d\n", client->indice); //maj des fichiers locals
	fprintf(client->fd_local, "%d\n", client->indice);
	tableau_local[client->numero_thread][taille_local[client->numero_thread]] = client->indice;
	taille_local[client->numero_thread] += 1;
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
{	/*Récupère les parties de fichiers qui ont déja "t" téléchargé*/
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

    while(strcmp(tracker->buff,"...") != 0)
    {
        int numero_fichier = 0;
        int compteur_fichier = 0;
		strcpy(tracker->buff, "");
		lireLigne(fd_tracker, tracker->buff);
        strcpy(tracker->ip[compteur], tracker->buff);
		printf("ip : %s", tracker->ip[compteur]);
        strcpy(tracker->buff, "");
		lireLigne(fd_tracker, tracker->buff);
		strcpy(tracker->chemin_serveur, tracker->buff);
		strcpy(tracker->buff, "");
		lireLigne(fd_tracker, tracker->buff);
		tracker->nb_fichier[compteur] = atoi(tracker->buff);
		strcpy(tracker->buff,"");
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
        compteur += 1;
        lireLigne(fd_tracker, tracker->buff);
    }
    tracker->nb_ip = compteur;
}
