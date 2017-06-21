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

        if(connect(tracker->socket_client, (struct sockaddr *)tracker->socket_serveur, sizeof(struct sockaddr_in)) < 0)
        {
            perror("connect");
			tracker->tableau_peer[k] = -1;
        }
//############################################Deportage de port#######################################################################
//on ne peut pas utiliser la fonction connection_socket ici car celle-ci utilise un résultat de meilleur_peer pour fonctionner...
		else
		{
			lireLigne(tracker->socket_client, tracker->port);
			ecrireLigne(tracker->socket_client, "1\n");
			tracker->socket_serveur = resolv(tracker->ip[k], tracker->port);
			if(tracker->socket_client == -1 || tracker->socket_serveur == NULL)
	        {
	            perror("socket");
	            exit(EXIT_FAILURE);
	        }

	        if(connect(tracker->socket_client, (struct sockaddr *)tracker->socket_serveur, sizeof(struct sockaddr_in)) < 0)
	        {
	            perror("connect");
	            tracker->tableau_peer[k] = -1;
	        }

	//############################################Fin déportage de port#####################################################################
			else
			{
		        ecrireLigne(tracker->socket_client, tracker->nom_fichier);
		        lireLigne(tracker->socket_client, tracker->buff);
		        tracker->tableau_peer[k] = tracker->nb_fichier[k]/atol(tracker->buff);
			}
		}
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
