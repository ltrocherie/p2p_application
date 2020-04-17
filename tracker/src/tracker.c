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
#include <fcntl.h>
#include <string.h>

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

#define CONFIG "config.ini"
#define LOG "log"
int config_fd, log_fd;

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
        usage_commands(socket);
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
        usage_commands(socket);
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
                        usage_commands(socket);
                        return;
                    }
                    fprintf(stdout,"add:%s|key:%s\n", seeds[0], seeds[3]);
                    n = write(log_fd, "key ", 4);
                    n = write(log_fd, seeds[3], sizeof(seeds[3]));
                    n = write(log_fd, "add ", 4);
                    n = write(log_fd, seeds[0], sizeof(seeds[0]));

                    int add = hash__add(seeds[3], IP, port, seeds[0], atoi(seeds[1]), atoi(seeds[2]));

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
                /* Finish the word */
                key_leech[tmp] = '\0';
                //TODO : renvoyer une liste de peers avec les clés ???
                fprintf(stdout,"key:%s\n", key_leech);
                n = write(log_fd, "key ", 4);
                n = write(log_fd, key_leech, sizeof(key_leech));

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
                        usage_commands(socket);
                        return;
                    }

                    fprintf(stdout,"add:%s|key:%s\n", seeds[0], seeds[3]);
                    n = write(log_fd, "key ", 4);
                    n = write(log_fd, seeds[3], sizeof(seeds[3]));
                    n = write(log_fd, "add ", 4);
                    n = write(log_fd, seeds[0], sizeof(seeds[0]));

                    int add = hash__add(seeds[3], IP, port, seeds[0], atoi(seeds[1]), atoi(seeds[2]));

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
                    usage_commands(socket);
                    return;
                }
            }
            /* LEECH CASE : at the end of every word we show the key */
            else if (!seed && leech)
            {
                /* Finish the word */
                key_leech[tmp] = '\0';
                //TODO : renvoyer une liste de peers avec les clés ???
                fprintf(stdout,"key:%s\n", key_leech);
                n = write(log_fd, "key ", 4);
                n = write(log_fd, key_leech, sizeof(key_leech));

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

    char arg[1024];
    char name[1024] = "\0";
    char size[64] = "-1";
    char comparator = '=';
    int given_size = 0, given_name = 0;

    /* Read all characters */
    while (buffer[i] != 0 && buffer[i] != '\n' && buffer[i] != ']')
    {
        switch (buffer[i])
        {
        case ' ':
            if (tmp <= 0)
            {
                i++;
                break;
            }

            given_size = 0;
            given_name = 0;

            tmp = 0;
            i++;

            break;
        case '=':
            arg[tmp] = '\0';
            tmp = 0;
            if (strcmp(arg, "filename") == 0)
                given_name = 1;
            else if (strcmp(arg, "filesize") == 0)
                given_size = 1;
            else
            {
                usage_commands(socket);
                return;
            }
            i++;
            break;
        case '>':
            arg[tmp] = '\0';
            tmp = 0;
            /* If the comparator is > with other than filesize, this is not valid */
            if (strcmp(arg, "filesize") != 0)
            {
                usage_commands(socket);
                return;
            }
            given_size = 1;
            comparator = buffer[i];

            i++;
            break;
        case '<':
            arg[tmp] = '\0';
            tmp = 0;

            /* If the comparator is < with other than filesize, this is not valid */
            if (strcmp(arg, "filesize") != 0)
            {
                usage_commands(socket);
                return;
            }
            given_size = 1;
            comparator = buffer[i];

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
    if (!isNumeric(size))
    {
        return;
    }

    printf("filename:%s\n", name);
    printf("size:%s\n", size);
    printf("comparator:%c\n", comparator);

    char *find = malloc(1024 * sizeof(char));
    int s = atoi(size);
    hash__getfiles(comparator, name, s, find);
    printf("%s\n", find);
    printf("%d\n", s);
    int n = write(socket, "> list [", 8);
    n = write(socket, find, strlen(find)*sizeof(char));
    if (n < 0)
        error("ERROR writing to socket");
    n = write(socket, "]", 1);
    free(find);
    hash__print();
    return;
}

void update(int socket, char *buffer, char *IP)
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

    /* SEEDS LEECHS TREATMENT */
    int end = 0;
    int seed = 0, leech = 0;
    (void)seed;
    (void)leech;

    char key[1024];

    /* Read all characters */
    while (buffer[i] != 0 && buffer[i] != '\n' && buffer[i] != '\r' && end == 0)
    {
        switch (buffer[i])
        {
        case ' ':
            i++;

            /* If it is only spaces */
            if (tmp == 0)
                break;

            key[tmp] = '\0';
            tmp = 0;

            /* If we found the seed or leech keyword */
            if (!strcmp(key, SEED)) {
                seed = 1;
                break;
            } else if (!strcmp(key, LEECH)) {
                leech = 1;
                break;
            }

            /* Seed case */
            if (seed)
            {
                /* We add a new file OR a new owner in the existing file */
                printf("add seed:%s\n",key);
                /*int add = hash__add(seeds[3]
                                    ,IP
                                    ,port
                                    ,seeds[0]
                                    ,atoi(seeds[1])
                                    ,atoi(seeds[2]));*/
            }
            /* Leeching case */
            else if (leech)
            {
                /* Add leeching ??? */
                printf("add leech:%s\n",key);

            }
            break;
        case '[':
            i++;
            break;
        case ']':
            key[tmp] = '\0';
            tmp = 0;
            i++;

            /* If we didnt have neither seed or leech in [], we don't care */
            if (!strcmp(key, "\0"))
            {
                printf("no key\n");
                break;
            }

            /* Seed case */
            if (seed)
            {
                seed = 0;

                /* We add a new file OR a new owner in the existing file */
                printf("add seed:%s\n",key);
                /*int add = hash__add(seeds[3]
                                    ,IP
                                    ,port
                                    ,seeds[0]
                                    ,atoi(seeds[1])
                                    ,atoi(seeds[2]));*/
            }
            /* Leeching case */
            else if (leech)
            {
                leech = 0;
                end = 1;

                /* Add leeching ??? */
                printf("add leech:%s\n",key);

            }
            break;
        /* Case when we encounter a character */
        default:
            /* We fill the word in the args */
            key[tmp] = buffer[i];
            tmp++;
            i++;
            break;
        }
    }

    /* Everything happened good */
    int n = write(socket, "> ok", 148);
    if (n < 0)
        error("ERROR writing to socket");
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
    while (buffer[i] != 0 && buffer[i] != '\n' && buffer[i] != ' ')
    {
        key[tmp] = buffer[i];
        i++;
        tmp++;
    }
    key[tmp] = '\0';
    struct file *f = NULL;

    f = hash__search(key);

    if (f == NULL)
    {
        fprintf(stderr, "File with key %s not found in the hash table\n", key);
        return;
    }

    /* Everything happened good */

    int n = write(socket, "> peers ", 8);
    if (n < 0)
        error("ERROR writing to socket");
    int nb_owner = 0;
    n = write(socket, key, tmp*sizeof(char));
    n = write(socket, " [", 2);
    struct owner *own;
    SLIST_FOREACH(own, &f->owners, next_owner)
    {
        n = write(socket, own->IP, strlen(own->IP)*sizeof(char));
        n = write(socket, ":", 1);
        char* port = itoa(own->port,10);
        n = write(socket,port , strlen(port)*sizeof(char));
        if (nb_owner != f->nb_owners - 1)
            n = write(socket, " ", 1);
        nb_owner++;
    }

    n = write(socket, "]", 1);

    return;
}

void treat_socket(void *arg)
{
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
        usage_commands(socket);

    return;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* Pool of threads which will retrieve work to do in the queue of work */
    threadpool thpool = thpool_init(5);

    /* Open and write in config.ini */
    config_fd = open(CONFIG, O_RDONLY);
    log_fd = open(LOG, O_CREAT | O_WRONLY | O_APPEND,0755);

    /* Exiting program */
    if (config_fd < 0) {
        fprintf(stderr,"Error opening config.ini");
        return -1;
    }

    if (log_fd < 0) {
        fprintf(stderr,"Error opening log");
        return -1;
    }

    /* Choose ports */
    if (argc < 2 || argc > 2)
    {
        fprintf(stderr, "ERROR, you need to a port\n");
        usage();
        exit(1);
    }

    hash__table_init();

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

    hash__table_end();
    thpool_destroy(thpool);
    close(log_fd);
    close(config_fd);
    return 0;
}
