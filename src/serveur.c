#include "pse.h"
typedef struct
{
    struct sockaddr_in socket_server;
    struct sockaddr_in socket_client;
    socklen_t len;
    int socket;
    char port[20];
    int canal;
    char buff[100];
    char nom_fichier[200];
    int fd_fichier;
    char fichier_part[200];
}Server;

void* worker (void *donnes);

void* worker (void *donnes)
{
    Server *server = (Server *)donnes;
    sprintf(server->fichier_part, "%s%d.part", server->nom_fichier, 1);
    printf("nom du fichier : %s\n", server->fichier_part);
    server->fd_fichier = open(server->fichier_part, O_RDONLY);
    if(server->fd_fichier < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    while(lireLigne(server->fd_fichier, server->buff))
    {
        ecrireLigne(server->canal, server->buff);
        printf("il est ecris : %s\n", server->buff);
    }
    close(server->fd_fichier);
    close(server->canal);
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
    Server server;
    strcpy(server.port, "9091");
    strcpy(server.nom_fichier, "fichier_test");
    server.socket = socket(AF_INET, SOCK_STREAM, 0);
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
    while(1)
    {
        server.len = sizeof(server.socket_client);
        server.canal = accept(server.socket, (struct sockaddr*)&server.socket_client, &server.len);
        if(server.canal == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("connect ok\n");
        worker(&server);
    }
}
