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
    int fd_fichier;
}Server;

void* worker (void *donnes);

void* worker (void *donnes)
{
    Server *server = (Server *)donnes;
    lireLigne(server->canal, server->buff);
    printf("%s\n", server->buff);
    server->fd_fichier = open(server->buff, O_RDONLY);
    if(server->fd_fichier < 0)
    {
        perror("open");
        close(server->canal);
        exit(EXIT_FAILURE);
    }
    while(lireLigne(server->fd_fichier, server->buff))
    {
        ecrireLigne(server->canal, server->buff);
        printf("il est ecris : %s\n", server->buff);
    }
    close(server->fd_fichier);
    close(server->canal);
    return(NULL);
}
int main(int argc, char *argv[])
{
    Server server;
    strcpy(server.port, "9091");
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
    server.len = sizeof(server.socket_client);
    while(1)
    {
        server.canal = accept(server.socket, (struct sockaddr*)&server.socket_client, &server.len);
        if(server.canal == -1)
        {
            printf("b\n");
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("connect ok\n");
        worker(&server);
        printf("a\n");
    }
    exit(EXIT_SUCCESS);
}
