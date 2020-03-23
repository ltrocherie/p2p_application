#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/queue.h>

#include "utils.h"
#include "thpool.h"

#define SIZE 1024

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void treat_socket(void* newsockfd)
{
    int socket = *((int *)newsockfd);

    char buffer[SIZE];

    /* Read all the message received*/
    int rr;
    bzero(buffer, SIZE);

    /* je vais lire seulement les 1023 premiers caractères */
    rr = read(socket, buffer, SIZE - 1);

    if (rr < 0)
        error("ERROR reading from socket");

    printf("Here is the message: %s\n", buffer);

    /*
    //parsing du message recu
    // recup les fichiers demandés

    // SI ON ANNOUNCE
    for (fichier)
    {
        //creer un seeder
        struct FICHIER = add(hash, nom, clé, seeder);

        
    }

    // SI ON LOOK
    for (fichier)
    {
        //
        struct FICHIER = search(hash, nom, taille, comparateur, clé);
        n = write(newsockfd, "peers ", 6);

        for (int i = 0; i < FICHIER->nb_seeders; i++)
        {
            n = write(newsockfd, "%d %d", FICHIER->seeders[i].ip, FICHIER->seeders[i].port, 18);
        }
    }

    // SI ON UPDATE
    
    */
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    /* Pool of threads which will retrieve work to do in the queue of work */
    threadpool thpool = thpool_init(1);

    /* Choose intervals where the tracker listen to ports */
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, you need to a port\n");
        usage();
        exit(1);
    }

    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    /* internet socket */
    serv_addr.sin_family = AF_INET;
    /* we listen on every interfaces */
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    /* listened port */
    serv_addr.sin_port = htons(portno);

    /* link socket to the address */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* maximum 5 entries */
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    /* Listen to every connection */
    for (;;)
    {
        /* socket qui permet d'accepter la connexion */
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (newsockfd < 0)
            error("ERROR on accept");

        thpool_add_work(thpool, (void *)treat_socket, &newsockfd);
    }

    return 0;
}
