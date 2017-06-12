#include "pse.h"

typedef struct
{
    struct sockaddr_in socket_server;
    struct sockaddr_in socket_client;
    socklen_t len;
    int socket;             //port 9091
    int socket_donne;       //port 9090
    char port[20];          //9091
    char port_donne[20];    //9090
    int canal;              //canal de dl
    int canal_donne;        //canal de conversation donne
    char buff[100];
    int fd_fichier;
    char var[100];
}Server;

void* upload(void *donnes);
void donnes(Server *server);
int est_dans(int valeur, int *tab_valeur, int taille_tab);

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
void donnes(Server *server)
{
    lireLigne(server->canal, server->buff);
    server->fd_fichier = open(server->buff, O_RDONLY);
    printf("fd_fichier = %d\n", server->fd_fichier);
    if(server->fd_fichier < 0)
        ecrireLigne(server->canal, "0\n");
    else
    {
        ecrireLigne(server->canal, "1\n");
        printf("a\n");
    }

    while(lireLigne(server->fd_fichier, server->buff))
    {
        ecrireLigne(server->canal, server->buff);
        printf("Il est ecris : %s\n", server->buff);
    }
    ecrireLigne(server->canal, "@@@@!!@!//785\n");
}

void* upload(void *donnees)
{
    Server *server = (Server*)donnees;
    donnes(server);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    Server server;
    Server *tab_server = malloc(sizeof(Server) * 100);
    int compteur_thread = 0;
    strcpy(server.port, "9091");
    server.socket = socket(AF_INET, SOCK_STREAM, 0);
    pthread_t *tab_thread = malloc(sizeof(pthread_t) * 100);
    int *tab_port = malloc(sizeof(int) * 10);
    int compteur_port = 0;
    srand(time(NULL));
    if(server.socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket ok\n");
    server.socket_server.sin_family = AF_INET;
    server.socket_server.sin_port = htons((short)atoi("9091"));
    server.socket_server.sin_addr.s_addr = INADDR_ANY;

    if(bind(server.socket, (struct sockaddr*)&server.socket_server, sizeof(server.socket_server)) != 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("bind ok\n");
    if(listen(server.socket, 20) != 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("listen ok\n");
    server.len = sizeof(server.socket_client);
    while(1)
    {
        tab_server[compteur_thread].canal = accept(server.socket, (struct sockaddr*)&server.socket_client, &server.len);
        printf("Connection\n");
        if(tab_server[compteur_thread].canal == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
//###############################Deportage de port################################################
        do {
            tab_port[compteur_port] = rand()%(9092-10100) + 9092;
        } while(est_dans(tab_port[compteur_port], tab_port, compteur_port) != 0);
        compteur_port += 1;
        sprintf(tab_server[compteur_thread].port,"%d\n", tab_port[compteur_port - 1]);
        ecrireLigne(tab_server[compteur_thread].canal, tab_server[compteur_thread].port);
        close(tab_server[compteur_thread].canal);
        tab_server[compteur_thread].socket = socket(AF_INET, SOCK_STREAM, 0);
        if(tab_server[compteur_thread].socket < 0)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        printf("**Socket ok\n");
        tab_server[compteur_thread].socket_server.sin_family = AF_INET;
        tab_server[compteur_thread].socket_server.sin_port = htons((short)atoi(tab_server[compteur_thread].port));
        tab_server[compteur_thread].socket_server.sin_addr.s_addr = INADDR_ANY;

        if(bind(tab_server[compteur_thread].socket, (struct sockaddr*)&tab_server[compteur_thread].socket_server, sizeof(tab_server[compteur_thread].socket_server)) != 0)
        {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        printf("**bind ok\n");
        if(listen(tab_server[compteur_thread].socket, 1) != 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        printf("**listen ok\n");
        tab_server[compteur_thread].len = sizeof(tab_server[compteur_thread].socket_client);
        tab_server[compteur_thread].canal = accept(tab_server[compteur_thread].socket, (struct sockaddr*)&tab_server[compteur_thread].socket_client, &tab_server[compteur_thread].len);
        if(tab_server[compteur_thread].canal == -1)
        {
            printf("b\n");
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("**connect ok\n");
//################################Fin deportage de port################################################
        pthread_create(&tab_thread[compteur_thread], NULL, upload, &tab_server[compteur_thread]);
        compteur_thread += 1;
        pthread_join(tab_thread[compteur_thread - 1], NULL);
    }
    return(EXIT_SUCCESS);
}
