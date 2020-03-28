#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include <pthread.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "thpool.h"
#include "tracker.h"

#define SIZE 1024

#define ANNOUNCE "announce"
#define LOOK "look"
#define UPDATE "update"

#define LISTEN "listen"

#define SEED "seed"
#define LEECH "leech"

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void treat_socket(void *arg)
{
    char args[1024];
    char buffer[SIZE];
    char command[SIZE];

    /* Cast into struct socket_ip */
    socket_ip socket_with_ip = *((socket_ip *)arg);

    /* Retrieve the socket */
    int socket = socket_with_ip.socketfd;
    char *ip = socket_with_ip.ip;

    printf("IP Received %s\n", ip);

    /* Read all the message received*/
    int rr;
    bzero(buffer, SIZE);
    rr = read(socket, buffer, SIZE - 1);

    /* If there is no byte read : error */
    if (rr < 0)
        error("ERROR reading from socket");

    printf("Here is the message: %s\n", buffer);

    get_command(buffer,command);
    printf("%s\n", command);

    printf("1 : %s\n",command);
    printf("2 : %s\n",ANNOUNCE);

    if (strcmp(command, ANNOUNCE) == 0)
        printf("ANNOUNCE\n");
    else if (strcmp(command, LOOK) == 0)
        printf("LOOK\n");
    else if (strcmp(command, UPDATE) == 0)
        printf("UPDATE\n");
  

    int tmp = 0;
    int end = 0;
    int i = 0;
    int seed = 0, leech = 0;

    (void)seed;
    (void)leech;

    while(buffer[i] != 0 || end == 0) {
        switch(buffer[i]) {
            case ' ':
                if (tmp > 0) {
                    args[tmp] = '\0';
                    printf("%s\n",args);
                    tmp = 0;
                }
                i++;
                break;
            case '[':
                if (!seed && !leech)
                    seed = 1;
                else if (seed && !leech)
                    leech = 1;
                
                i++;
                break;
            case ']':
                args[tmp] = '\0';
                printf("%s\n",args);
                tmp = 0;
                i++;

                /* If both seed and leechs args weere given, it's finished*/
                if(seed && leech)
                    end = 1;
                break;
            default:
                args[tmp] = buffer[i];
                tmp++;
                i++;
                break;
        }
    }

    printf("fin traitement\n");

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

    /* Pool of threads which will retrieve work to do in the queue of work */
    threadpool thpool = thpool_init(5);

    /* Choose ports */
    if (argc < 2 || argc > 2)
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

    show_local_ip(portno);

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

        socket_ip arg = {newsockfd,inet_ntoa(cli_addr.sin_addr)};
        thpool_add_work(thpool, (void *)treat_socket, &arg);
    }

    thpool_destroy(thpool);
    return 0;
}
