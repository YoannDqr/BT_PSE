#include "pse.h"
#include "serveur.h"

int main(int argc, char *argv[])
{
    Server server; //ecoute sur le port 9091 permet la transmission des fichiers
    Server server_donne; //ecoute sur le port 9092 permet la transmission du tracker
    int k;
    Server *tab_server = malloc(sizeof(Server) * 1000);
    Server *tab_server_donne = malloc(sizeof(Server) * 1000);
    int compteur_thread = 0;
    strcpy(server.port, "9091");


    server.socket = socket(AF_INET, SOCK_STREAM, 0);
    server_donne.socket = socket(AF_INET, SOCK_STREAM, 0);
    pthread_t *tab_thread = malloc(sizeof(pthread_t) * 1000);
    pthread_t *tab_thread_donne = malloc(sizeof(pthread_t) * 1000);
    int *tab_port = malloc(sizeof(int) * 1000);
    int compteur_port = 0;
    srand(time(NULL));
    if(server.socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server.socket_server.sin_family = AF_INET;
    server.socket_server.sin_port = htons((short)atoi("9091"));
    server.socket_server.sin_addr.s_addr = INADDR_ANY;

    if(bind(server.socket, (struct sockaddr*)&server.socket_server, sizeof(server.socket_server)) != 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if(listen(server.socket, 20000) != 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }





//######################### COnfiguration de la socket du 9092 #################################################################################

    server_donne.socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_donne.socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_donne.socket_server.sin_family = AF_INET;
    server_donne.socket_server.sin_port = htons((short)atoi("9092"));
    server_donne.socket_server.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_donne.socket, (struct sockaddr*)&server_donne.socket_server, sizeof(server_donne.socket_server)) != 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if(listen(server_donne.socket, 20000) != 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }



//###########################################Fin config 9092###########################################################################################






    server.len = sizeof(server.socket_client);
    while(1)
    {
        memcpy(&tab_server_donne[compteur_thread], &server_donne, sizeof(Server));
        tab_server[compteur_thread].canal = accept(server.socket, (struct sockaddr*)&server.socket_client, &server.len);

        if(tab_server[compteur_thread].canal == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
//###############################Deportage de port################################################
        tab_port[compteur_port] = 20001 + compteur_port;
        tab_port[compteur_port + 1] = 20001 + compteur_port + 1;
        sprintf(tab_server_donne[compteur_thread].port, "%d\n", 20001 + compteur_port + 1);
        compteur_port += 2;
        sprintf(tab_server[compteur_thread].port,"%d\n", tab_port[compteur_port - 2]);
        printf("port : %s    %s", tab_server[compteur_thread].port, tab_server_donne[compteur_thread].port);
        tab_server[compteur_thread].socket = socket(AF_INET, SOCK_STREAM, 0);
        if(tab_server[compteur_thread].socket < 0)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        tab_server[compteur_thread].socket_server.sin_family = AF_INET;
        tab_server[compteur_thread].socket_server.sin_port = htons((short)atoi(tab_server[compteur_thread].port));
        tab_server[compteur_thread].socket_server.sin_addr.s_addr = INADDR_ANY;

        if(bind(tab_server[compteur_thread].socket, (struct sockaddr*)&tab_server[compteur_thread].socket_server, sizeof(tab_server[compteur_thread].socket_server)) != 0)
        {
            perror("bind");
            exit(EXIT_FAILURE);
        }

        if(listen(tab_server[compteur_thread].socket, 1) != 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        tab_server[compteur_thread].len = sizeof(tab_server[compteur_thread].socket_client);
        ecrireLigne(tab_server[compteur_thread].canal, tab_server[compteur_thread].port);
        strcpy(tab_server[compteur_thread].buff, "");
        lireLigne(tab_server[compteur_thread].canal, tab_server[compteur_thread].buff);

        close(tab_server[compteur_thread].canal);
        tab_server[compteur_thread].canal = accept(tab_server[compteur_thread].socket, (struct sockaddr*)&tab_server[compteur_thread].socket_client, &tab_server[compteur_thread].len);

        if(tab_server[compteur_thread].canal == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

//################################Fin deportage de port################################################
        tab_server_donne[compteur_thread].len = sizeof(server_donne.socket_client);
        pthread_create(&tab_thread[compteur_thread], NULL, upload, &tab_server[compteur_thread]);
        pthread_create(&tab_thread_donne[compteur_thread], NULL, maj_tracker, &tab_server_donne[compteur_thread]);

        compteur_thread += 1;
    }

    for(k = 0; k < compteur_thread; k++)
    {
        pthread_join(tab_thread[compteur_thread], NULL);
        pthread_join(tab_thread_donne[compteur_thread], NULL);
    }
    return(EXIT_SUCCESS);
}
