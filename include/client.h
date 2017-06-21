int tableau_local[100][100];
int taille_local[100];

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
	int numero_thread;
	char adresse_ip[20];
	int fichier_split;
	FILE *fichier;
	char var_cte[2000];
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
void connection_socket(Client *client, int port);
void join(Client *client, char *nom, int nombre_fichier);
void maj_tracker(Client* client);
