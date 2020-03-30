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
#include "hash_table.h"

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

void announce(char *buffer, char *IP)
{
    char *p, space = ' ';

    /* Returns the pointer of the first occurence of the separator */
    p = strchr(buffer, space);

    /* If there is no space in the command */
    if (p == NULL)
    {
        fprintf(stderr, "No %c found.\n", space);
        return;
    }

    /* Pointer of next word */
    buffer = p + 1;
    int i = 0, tmp = 0;

    /* Count number of files */
    int n = 1;

    /* LISTEN PORT TREATMENT */
    char port_arg[8];

    while (buffer[i] != ' ' && buffer[i] != 0 && buffer[i] != '\n') {
        port_arg[tmp] = buffer[i];
        i++;
        tmp++;
    }
    port_arg[tmp] = '\0';
    tmp = 0;
    i++;

    /* If the word wasn't "listen" : print the commands */
    if (strcmp(port_arg, "listen") != 0) {
        usage_commands();
        return;
    }

    /* Get local port */
    while (buffer[i] != ' ' && buffer[i] != 0 && buffer[i] != '\n') {
        port_arg[tmp] = buffer[i];
        i++;
        tmp++;
    }
    port_arg[tmp] = '\0';
    tmp = 0;
    i++;

    if (!isNumeric(port_arg)) {
        usage_commands();
        return;
    }

    printf("Port:%s\n",port_arg);
    int port = atoi(port_arg);

    /* SEEDS LEECHS TREATMENT */

    int end = 0;
    int seed = 0, leech = 0;
    int seed_arg = 0;

    (void)seed;
    (void)leech;

    /* Array of arguments for all file arguments : key, IP, port, filename, length, piecesize */
    char seeds[4][1024];

    /* Key leech asked */
    char key_leech[1024];

    buffer = buffer + i;
    i = 0;

    /* Read all characters */
    while (buffer[i] != 0 && buffer[i] != '\n' && buffer[i] != '\r' && end == 0)
    {
        switch (buffer[i])
        {
        case ' ':
            if (tmp == 0) {
                i++;
                break;
            }

            /* SEED CASE : every 4 words we add the file */
            if (seed && !leech)
            {
                /* Finish the word */
                seeds[seed_arg][tmp] = '\0';
                tmp = 0;

                /* If we have done a file */
                if (seed_arg == 3) {
                    /* Verify integrity of fields */
                    if (!isNumeric(seeds[1]) || !isNumeric(seeds[2])) {
                        fprintf(stderr,"Size of piecesize must be integers\n");
                        usage_commands();
                        return;
                    }
                    printf("---------------------------\n"
                            "File:%d\n",n);
                    printf("key:%s\n", seeds[3]);
                    printf("IP:%s\n", IP);
                    printf("port:%s\n", port_arg);
                    printf("filename:%s\n", seeds[0]);
                    printf("length:%s\n", seeds[1]);
                    printf("piecesize:%s\n", seeds[2]);
                    printf("---------------------------\n");

                    /* Add the file to the hash_table */
                    //add(seeds[3],IP,port,seeds[0],atoi(seeds[1]),atoi(seeds[2]));
                    seed_arg = 0;
                    n++;
                }
                /* If we didn't give enough args for a file */
                else
                    seed_arg++;
            }

            /* LEECH CASE : at the end of every word we show the key */
            else if (!seed && leech)
            {
                if (!isNumeric(key_leech)) {
                    fprintf(stderr,"Key must be an integer\n");
                    usage_commands();
                    return;
                }
                
                /* Finish the word */
                key_leech[tmp] = '\0';
                printf("key:%s\n", key_leech);
                tmp = 0;
                break;
            }

            i++;
            break;

        case '[':

            /* if we start by giving the seeds */
            if (!seed && !leech) {
                seed = 1;
                seed_arg = 0;
            }
            /* else if it is for leechs */
            else if (seed && !leech) {
                leech = 1;
                seed = 0;
            }

            tmp = 0;
            i++;
            break;

        case ']':

            /* SEED CASE : every 4 words we add the file */
            if (seed && !leech)
            {
                /* Finish the word */
                seeds[seed_arg][tmp] = '\0';
                tmp = 0;

                if (seed_arg == 3) {
                    /* Verify integrity of fields */
                    if (!isNumeric(seeds[1]) || !isNumeric(seeds[2])) {
                        fprintf(stderr,"Size of piecesize must be integers\n");
                        usage_commands();
                        return;
                    }

                    printf("---------------------------\n"
                            "File:%d\n",n);
                    printf("key:%s\n", seeds[3]);
                    printf("IP:%s\n", IP);
                    printf("port:%s\n", port_arg);
                    printf("filename:%s\n", seeds[0]);
                    printf("length:%s\n", seeds[1]);
                    printf("piecesize:%s\n", seeds[2]);
                    printf("---------------------------\n");
                    /* Add the file to the hash_table */
                    //add(seeds[3],IP,port,seeds[0],atoi(seeds[1]),seeds[2]);
                    seed_arg = 0;
                }
                else
                {
                    fprintf(stderr, "Not enough parameters for a file\n");
                    usage_commands();
                    return;
                } 
            }
            /* LEECH CASE : at the end of every word we show the key */
            else if (!seed && leech)
            {
                /* Finish the word */
                key_leech[tmp] = '\0';
                printf("key:%s\n", key_leech);
                tmp = 0;

                /* If both seeds and leechs args were given, it's finished */
                end = 1;
            }
            i++;
            break;

        /* Case when we encounter a character */
        default:
            /* We fill the word in the args */
            if (seed && !leech)
                seeds[seed_arg][tmp] = buffer[i];
            else if (!seed && leech)
                key_leech[tmp] = buffer[i];
            tmp++;
            i++;
            break;
        }
    }
}

void look(char *buffer)
{
    printf("%s\n", buffer);
}

void update(char *buffer)
{
    printf("%s\n", buffer);
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

    get_command(buffer, command);
    printf("%s\n", command);

    if (strcmp(command, ANNOUNCE) == 0)
        announce(buffer, ip);
    else if (strcmp(command, LOOK) == 0)
        look(buffer);
    else if (strcmp(command, UPDATE) == 0)
        update(buffer);
    else
        usage_commands();

    return;
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

        socket_ip arg = {newsockfd, inet_ntoa(cli_addr.sin_addr)};
        thpool_add_work(thpool, (void *)treat_socket, &arg);
    }

    thpool_destroy(thpool);
    return 0;
}
