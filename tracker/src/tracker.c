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

#include "queue.h"
#include "utils.h"
#include "thpool.h"
#include "tracker.h"
#include "hash_table.h"

#define SIZE 1024

#define ANNOUNCE "announce"
#define LOOK "look"
#define UPDATE "update"
#define GET "getfile"

#define LISTEN "listen"

#define SEED "seed"
#define LEECH "leech"

//extern SLIST_HEAD(,file) hash_table[HASH_TABLE_LENGTH];

void error(char *msg)
{
    perror(msg);
    exit(1);
}

/**
 * Parse the buffer given by the peer in order to figoure out what the port is but also
 * which files will be added to the hash_table and finally all leechs incoming.
 * 
 * @param socket socket of the current connection
 * @param buffer message given by the peer
 * @param IP IP of the peer
 * 
 * @return will display "> ok" if everything happened right with all files added in hash_table,
 * will display the usage otherwise on stderr. Be careful, if the command has not the right number of arguments for seed (for example) corrects files before the error will be still
 * added to the hash table
 * */
void announce(int socket, char *buffer, char *IP)
{
    char *p, space = ' ';

    /* Returns the pointer of the first occurence of the separator */
    p = strchr(buffer, space);

    /* If there is no space in the command */
    if (p == NULL)
    {
        fprintf(stderr, "No \"%c\" found.\n", space);
        return;
    }

    /* Pointer of next word */
    buffer = p + 1;
    int i = 0, tmp = 0;

    /* Count number of files */
    int n = 1;

    /* LISTEN PORT TREATMENT */
    char port_arg[8];

    while (buffer[i] != ' ' && buffer[i] != 0 && buffer[i] != '\n')
    {
        port_arg[tmp] = buffer[i];
        i++;
        tmp++;
    }
    port_arg[tmp] = '\0';
    tmp = 0;
    i++;

    /* If the word wasn't "listen" : print the commands */
    if (strcmp(port_arg, "listen") != 0)
    {
        usage_commands();
        return;
    }

    /* Get local port */
    while (buffer[i] != ' ' && buffer[i] != 0 && buffer[i] != '\n')
    {
        port_arg[tmp] = buffer[i];
        i++;
        tmp++;
    }
    port_arg[tmp] = '\0';
    tmp = 0;
    i++;

    if (!isNumeric(port_arg))
    {
        usage_commands();
        return;
    }

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
            if (tmp == 0)
            {
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
                if (seed_arg == 3)
                {
                    /* Verify integrity of fields */
                    if (!isNumeric(seeds[1]) || !isNumeric(seeds[2]))
                    {
                        fprintf(stderr, "Size of piecesize must be integers\n");
                        usage_commands();
                        return;
                    }
                    printf("add:%s|key:%s\n",seeds[0],seeds[3]);
                    int add = hash__add(seeds[3]
                                        ,IP
                                        ,port
                                        ,seeds[0]
                                        ,atoi(seeds[1])
                                        ,atoi(seeds[2]));

                    if (!add)
                    {
                        fprintf(stderr, "Problem adding in has_table");
                        return;
                    }
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
                if (!isNumeric(key_leech))
                {
                    fprintf(stderr, "Key must be an integer\n");
                    usage_commands();
                    return;
                }

                /* Finish the word */
                key_leech[tmp] = '\0';
                //TODO : renvoyer une liste de peers avec les clés ???
                printf("key:%s\n", key_leech);
                tmp = 0;
                break;
            }

            i++;
            break;

        case '[':

            /* if we start by giving the seeds */
            if (!seed && !leech)
            {
                seed = 1;
                seed_arg = 0;
            }
            /* else if it is for leechs */
            else if (seed && !leech)
            {
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

                if (seed_arg == 3)
                {
                    /* Verify integrity of fields */
                    if (!isNumeric(seeds[1]) || !isNumeric(seeds[2]))
                    {
                        fprintf(stderr, "Size of piecesize must be integers\n");
                        usage_commands();
                        return;
                    }
                    printf("add:%s|key:%s\n",seeds[0],seeds[3]);
                    int add = hash__add(seeds[3]
                                        ,IP
                                        ,port
                                        ,seeds[0]
                                        ,atoi(seeds[1])
                                        ,atoi(seeds[2]));

                    if (!add)
                    {
                        fprintf(stderr, "Problem adding in hash_table");
                        return;
                    }
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
                //TODO : renvoyer une liste de peers avec les clés ???
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

    /* Everything happened good */
    n = write(socket, "> ok", 148);
    if (n < 0)
        error("ERROR writing to socket");
}

void look(int socket, char *buffer, char *IP)
{
    char *p, space = ' ';

    /* Returns the pointer of the first occurence of the separator */
    p = strchr(buffer, space);

    /* If there is no space in the command */
    if (p == NULL)
    {
        fprintf(stderr, "No \"%c\" found.\n", space);
        return;
    }

    /* Pointer of next word */
    buffer = p + 1;
    int i = 0, tmp = 0;

    char arg[1024], name[1024], size[64];
    char comparator = '=';
    int given_size = 0, given_name = 0;

    /* Read all characters */
    while (buffer[i] != 0 && buffer[i] != '\n' && buffer[i] != ']')
    {
        switch (buffer[i])
        {
        case ' ':
            if (tmp <= 0) {
                i++;
                break;
            }

            given_size = 0;
            given_name = 0;

            tmp = 0;
            i++;

            break;
        case '=':
            if (strcmp(arg,"filename") == 0)
                given_name = 1;       
            else if (strcmp(arg,"filesize") == 0)
                given_size = 1;
            else {
                usage_commands();
                return;
            }

            arg[tmp] = '\0';
            tmp = 0;
            i++;
            break;
        case '>':
            /* If the comparator is > with other than filesize, this is not valid */
            if (strcmp(arg,"filesize") != 0) {
                usage_commands();
                return;
            }
            given_size = 1;
            comparator = buffer[i];

            arg[tmp] = '\0';
            tmp = 0;
            i++;
            break;
        case '<':
            /* If the comparator is < with other than filesize, this is not valid */
            if (strcmp(arg,"filesize") != 0) {
                usage_commands();
                return;
            }
            given_size = 1;
            comparator = buffer[i];

            arg[tmp] = '\0';
            tmp = 0;     
            i++;
            break;
        case '[':
            i++;
            break;
        case '"':
            arg[tmp] = '\0';

            if (given_size)
                strcpy(size, arg);
            else if (given_name)
                strcpy(name, arg);

            tmp = 0;
            i++;
            break;
        default:
            arg[tmp] = buffer[i];
            tmp++;
            i++;
            break;
        }
    }

    printf("filename:%s\n",name);
    printf("size:%s\n",size);
    printf("comparator:%c\n",comparator);

    struct file *f = NULL;

    //TODO
    /* Search for the file with all criterions */
    //int search = hash__search()
    return;
}

void update(int socket, char *buffer, char *IP)
{
    printf("%s\n", buffer);
}

void getfile(int socket, char *buffer, char *IP)
{
    char *p, space = ' ';

    /* Returns the pointer of the first occurence of the separator */
    p = strchr(buffer, space);

    /* If there is no space in the command */
    if (p == NULL)
    {
        fprintf(stderr, "No \"%c\" found.\n", space);
        return;
    }

    /* Pointer of next word */
    buffer = p + 1;
    int i = 0, tmp = 0;

    char key[1024];

    /* Read all characters */
    while (buffer[i] != 0 && buffer[i] != '\n' && buffer[i] != ' ') {
        key[tmp] = buffer[i];
        i++;
    }

    struct file* f = NULL;

    int search = hash__search(key, f);

    if (!search) {
        fprintf(stderr, "File with key %s not found in the hash table\n", key);
        return;
    }

    /* Everything happened good */

    int n = write(socket, "> peers ",8);
    if (n < 0)
        error("ERROR writing to socket");

    n = write(socket, key, sizeof(key));
 
    //TODO itererer sur les owners et les afficher
    //affiche owners in file

    n = write(socket, "]", 1);

    return;
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

    printf("Here is the message: %s", buffer);

    get_command(buffer, command);
    printf("command:%s\n", command);

    if (!strcmp(command, ANNOUNCE))
        announce(socket, buffer, ip);
    else if (!strcmp(command, LOOK))
        look(socket, buffer, ip);
    else if (!strcmp(command, UPDATE))
        update(socket, buffer, ip);
    else if (!strcmp(command, GET))
        getfile(socket, buffer, ip);
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
