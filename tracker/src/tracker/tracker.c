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

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
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

    /* socket internet */
    serv_addr.sin_family = AF_INET;
    /* lorsque l'on va lancer le serveur : en écoute sur toutes les interfaces */
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    /* port en ecoute de l'argument */
    serv_addr.sin_port = htons(portno);

    /* lier la socket a l'adresse */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* au maximum 5 connexions attentes */
    listen(sockfd, 5);

    sleep(10);

    clilen = sizeof(cli_addr);

    /* socket qui permet d'accepter la connexion */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    bzero(buffer, 256);

    /* je vais lire seulement les 255 premiers caractères */
    n = read(newsockfd, buffer, 255);
    if (n < 0)
        error("ERROR reading from socket");
    printf("Here is the message: %s\n", buffer);

    /* ecrire sur la socket */
    n = write(newsockfd, "I got your message", 18);
    if (n < 0)
        error("ERROR writing to socket");
    return 0;
}
