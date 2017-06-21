typedef struct
{
    struct sockaddr_in socket_server;
    struct sockaddr_in socket_client;
    socklen_t len;
    int socket;             //port 9091
    char port[20];          //9091
    int canal;              //canal de dl
    char buff[2000];
    char buff_donne[2000];
    FILE *fd_tracker;
    int fd_fichier;
    char var[2000];
}Server;

void* upload(void *donnes);
void up_donnes(Server *server);
int est_dans(int valeur, int *tab_valeur, int taille_tab);
void* maj_tracker (void* donnes);
