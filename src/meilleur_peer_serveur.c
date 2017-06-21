void* meilleur_peer(void *donnes)
{s
    Server *server = (Server *)donnes;
    int k;
    connection_socket(server);

    lireLigne(server->canal, server->buff_peer); //De quelle fichier il s'agit ?
    for(k = 0; k < compteur_thread; k++)
    {
        if(strcmp(tab_server[k].nom_fichier, server->buff_peer))
            server->nb_fichier += 1;
    }
    sprintf(server->buff_peer, "%d\n", server->nb_fichier);
    ecrireLigne(server->canal, server->buff_peer);
    lireLigne(server->canal, server->buff_peer);
    close(server->canal);
    pthread_exit(NULL);

}
