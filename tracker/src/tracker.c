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
#include <pthread.h>

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

int log_fd;
pthread_mutex_t log_lock;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

/**
 * Parse the buffer given by the peer in order to figure out what the port is but also
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
        exit_if(send(socket, "> nok", 5,0) == -1, "ERROR sending to socket");
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
        exit_if(send(socket, "> nok", 5,0) == -1, "ERROR sending to socket");
        return;
    }

    int port = atoi(port_arg);
    // ajout du couple IP:port dans un tableau d'owners
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
            if (seed && !leech) {
                /* Finish the word */
                seeds[seed_arg][tmp] = '\0';
                tmp = 0;

                /* If we have done a file */
                if (seed_arg == 3) {
                    /* Verify integrity of fields */
                    if (!isNumeric(seeds[1]) || !isNumeric(seeds[2])) {
                        fprintf(stderr, "Size of piecesize must be integers\n");
                        exit_if ( write(log_fd, "\nSize of piecesize must be integers", 35) == -1, "ERROR write log" );
                        exit_if ( send(socket, "> nok", 5, 0) == -1, "ERROR sending to socket" );
                        return;
                    }

                    fprintf(stdout,"add:%s|key:%s|taille:%d|piece:%d\n", seeds[0], seeds[3],atoi(seeds[1]), atoi(seeds[2]));
                    exit_if ( write(log_fd, "\nkey ", 5) == -1, "ERROR write socket" );
                    exit_if ( write(log_fd, seeds[3], strlen(seeds[3])*sizeof(char)) == -1, "ERROR write socket" );
                    exit_if ( write(log_fd, "\nadd ", 5) == -1, "ERROR write socket" );
                    exit_if ( write(log_fd, seeds[0], strlen(seeds[0])*sizeof(char)) == -1, "ERROR write socket" );

                    int add = hash__add(seeds[3], IP, port, seeds[0], atoi(seeds[1]), atoi(seeds[2]));

                    if (!add)
                    {
                        fprintf(stderr, "Problem adding in hash_table");
                        exit_if ( write(log_fd, "\nProblem adding in hash_table", 33) == -1, "ERROR write log" );
                        exit_if ( send(socket, "> nok", 5, 0) == -1, "ERROR sending to socket" );
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
            else if (!seed && leech) {
                /* Finish the word */
                key_leech[tmp] = '\0';
                //TODO : renvoyer une liste de peers avec les clés ???
                fprintf(stdout,"key:%s\n", key_leech);
                exit_if ( write(log_fd, "\nkey ", 5) == -1, "ERROR write log" );
                exit_if ( write(log_fd, key_leech, strlen(key_leech)*sizeof(char)) == -1, "ERROR write log" );
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
                    if (!isNumeric(seeds[1]) || !isNumeric(seeds[2])) {
                        fprintf(stderr, "Size of piecesize must be integers\n");
                        exit_if ( write(log_fd, "\nSize of piecesize must be integers", 35) == -1, "ERROR write log" );
                        exit_if ( send(socket, "> nok", 5, 0) == -1, "ERROR sending to socket" );
                        return;
                    }

                    fprintf(stdout,"add:%s|key:%s|taille:%d|piece:%d\n", seeds[0], seeds[3],atoi(seeds[1]), atoi(seeds[2]));
                    exit_if ( write(log_fd, "\nkey ", 5) == -1, "ERROR write log" );
                    exit_if ( write(log_fd, seeds[3], strlen(seeds[3])*sizeof(char)) == -1, "ERROR write log" );
                    exit_if ( write(log_fd, "\nadd ", 5) == -1, "ERROR write log" );
                    exit_if ( write(log_fd, seeds[0], strlen(seeds[0])*sizeof(char)) == -1, "ERROR write log" );

                    int add = hash__add(seeds[3], IP, port, seeds[0], atoi(seeds[1]), atoi(seeds[2]));

                    if (!add) {
                        fprintf(stderr, "Problem adding in hash_table");
                        exit_if ( write(log_fd, "\nProblem adding in hash_table", 33) == -1, "ERROR write log" );
                        exit_if ( send(socket, "> nok", 5, 0) == -1, "ERROR sending to socket" );
                        return;
                    }
                    seed_arg = 0;
                } else {
                    fprintf(stderr, "Not enough parameters for a file\n");
                    exit_if ( write(log_fd, "\nNot enough parameters for a file", 33) == -1, "ERROR write log" );
                    exit_if ( send(socket, "> nok", 5, 0) == -1, "ERROR sending to socket" );
                    return;
                }
            }
            /* LEECH CASE : at the end of every word we show the key */
            else if (!seed && leech) {
                /* Finish the word */
                key_leech[tmp] = '\0';
                //TODO : renvoyer une liste de peers avec les clés ???
                fprintf(stdout,"key:%s\n", key_leech);
                exit_if ( write(log_fd, "key ", 4) == -1, "ERROR writing log" );
                exit_if ( write(log_fd, key_leech, strlen(key_leech)*sizeof(char)) == -1, "ERROR writing log" );

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

    hash__print();
    /* Everything happened good */
    exit_if ( send(socket, "> ok", 4, 0) == -1, "ERROR sending to socket" );
    close(socket);
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
                send(socket, "> nok", 5, 0);
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
                send(socket, "> nok", 5, 0);
                return;
            }
            given_size = 1;
            comparator = buffer[i];

            i++;
            break;
        case '<':
            arg[tmp] = '\0';
            tmp = 0;

            /* If the comparator is < with other than filesize, this is t valid */
            if (strcmp(arg, "filesize") != 0)
            {
                send(socket, "> nok", 5, 0);
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
        send(socket, "> nok", 5, 0);
        exit_if( write(log_fd,"\nNaN size",9) == -1, "ERROR NaN size" );
        return;
    }

    if (given_size && atoi(size) < 0) {
        send(socket, "> nok", 5, 0);
        exit_if( write(log_fd,"\nNegative size",9) == -1, "ERROR negative size" );
        return;
    }

    printf("filename:%s\n", name);
    printf("size:%s\n", size);
    printf("comparator:%c\n", comparator);

    exit_if( write(log_fd,"\nfilename:",10) == -1, "ERROR writing to log" );
    exit_if( write(log_fd,name,strlen(name)*sizeof(char)) == -1, "ERROR writing to log" );
    exit_if( write(log_fd,"\nsize:",6) == -1, "ERROR writing to log" );
    exit_if( write(log_fd,size,strlen(size)*sizeof(char)) == -1, "ERROR writing to log" );
    exit_if( write(log_fd,"\ncomparator:",12) == -1, "ERROR writing to log" );
    exit_if( write(log_fd,&comparator,sizeof(char)) == -1, "ERROR writing to log" );

    char *find = calloc(1024,sizeof(char));
    hash__getfiles(comparator, name, atoi(size), find);

    printf("%s\n", find);
    printf("%d\n", atoi(size));

    exit_if( send(socket, "> list [", 8, 0) == -1, "ERROR writing to socket");
    exit_if( send(socket, find, strlen(find)*sizeof(char), 0) == -1, "ERROR writing to socket");
    exit_if( send(socket, "]", 1, 0) == -1, "ERROR writing to socket");
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
    if (p == NULL) {
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
                printf("update seed:%s\n",key);
                exit_if ( write(log_fd, "\nupdate ", 8) == -1, "ERROR write log" );
                exit_if ( write(log_fd, "\nkey ", 5) == -1, "ERROR write log" );
                exit_if ( write(log_fd, key, strlen(key)*sizeof(char)) == -1, "ERROR write log" );

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
                exit_if ( write(log_fd, "key ", 4) == -1, "ERROR writing log" );
                exit_if ( write(log_fd, key, strlen(key)*sizeof(key)) == -1, "ERROR writing log" );

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
            if (!strcmp(key, "\0")) {
                printf("no key\n");
                break;
            }

            /* Seed case */
            if (seed)
            {
                seed = 0;

                /* We add a new file OR a new owner in the existing file */
                printf("update seed:%s\n",key);
                exit_if ( write(log_fd, "\nupdate ", 8) == -1, "ERROR write log" );
                exit_if ( write(log_fd, "\nkey ", 5) == -1, "ERROR write log" );
                exit_if ( write(log_fd, key, strlen(key)*sizeof(char)) == -1, "ERROR write log" );

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
                exit_if ( write(log_fd, "key ", 4) == -1, "ERROR writing log" );
                exit_if ( write(log_fd, key, strlen(key)*sizeof(key)) == -1, "ERROR writing log" );
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
    exit_if ( send(socket, "> ok", 5,0) == -1, "ERROR sending to socket");
    close(socket);
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
        exit_if( write(log_fd,"\nNo \"%c\" found",13) == -1, "ERROR writing log" );
        exit_if (send(socket,"> nok", 5,0) == -1, "ERROR sending to socket");
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
        exit_if( write(log_fd,"\nFile with key ",15) == -1, "ERROR writing log" );
        exit_if( write(log_fd,key,strlen(key)*sizeof(char)) == -1, "ERROR writing log" );
        exit_if( write(log_fd," not found in the hash table",28) == -1, "ERROR writing log" );
        exit_if (send(socket,"> nok", 5,0) == -1, "ERROR sending to socket");
        return;
    }

    hash__print();

    /* Everything happened good */

    exit_if( send(socket,"> peers ", 8,0) == -1, "ERROR sending to socket");

    exit_if( send(socket,key, strlen(key)*sizeof(char),0) == -1, "ERROR sending to socket");
    exit_if( send(socket," [", 2,0) == -1, "ERROR sending to socket");

    /* Writing peers */
    int nb_owner = 0;
    struct owner *own;
    SLIST_FOREACH(own, &f->owners, next_owner)
    {
        exit_if( send(socket, own->IP, strlen(own->IP)*sizeof(char), 0) == -1, "ERROR sending to socket");
        exit_if( send(socket, ":", 1, 0) == -1, "ERROR sending to socket");
        char* port = itoa(own->port,10);
        exit_if( send(socket,port , strlen(port)*sizeof(char), 0) == -1, "ERROR sending to socket");

        if (nb_owner != f->nb_owners - 1)
            exit_if( send(socket, " ", 1, 0) == -1, "ERROR sending to socket");

        nb_owner++;
    }
    exit_if( send(socket, "]", 1, 0) == -1, "ERROR sending to socket");

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
        exit_if(send(socket, "> nok", 5,0) == -1, "ERROR sending to socket");

    return;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, *sock_thread, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* Pool of threads which will retrieve work to do in the queue of work */
    threadpool thpool = thpool_init(5);

    /* Open and write in log */
    log_fd = open(LOG, O_CREAT | O_WRONLY | O_APPEND,0755);
    if (log_fd < 0) {
        fprintf(stderr,"Error opening log");
        return -1;
    }

    exit_if ( pthread_mutex_init(&log_lock, NULL) != 0, "ERROR init mutex" );

    hash__table_init();

    /* no port given : default port */
    if (argc < 2) {
        FILE* config = NULL;
        config = fopen(CONFIG, "r");
        if (config == NULL) {
            perror("Error opening config.ini");
            return -1;
        }
        char *p;

        char *line = malloc( sizeof(char)* 1024 );
        while(!feof(config)) //loop to read the file
        {
            fgets( line, 1024, config );
            p = strchr(line, ':')+1;
            portno = atoi(p);
        }
        free( line );
        fclose(config);
    } else
        portno = atoi(argv[1]);

    printf("Run on port:%d\n",portno);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    exit_if ( sockfd < 0, "ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    /* internet socket */
    serv_addr.sin_family = AF_INET;
    /* we listen on every interfaces */
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    /* listened port */
    serv_addr.sin_port = htons(portno);

    /* link socket to the address */
    exit_if ( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0, "ERROR on binding");

    show_local_ip(portno);

    /* maximum 5 entries */
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);

    /* Listen to every connection */
    for (;;)
    {
        /* accept incoming connection */
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        exit_if ( newsockfd < 0, "ERROR on accept" );

        sock_thread = malloc(sizeof(int));
        exit_if ( !sock_thread, "Error : Malloc Failed" );

        *sock_thread = newsockfd;

        socket_ip arg = {*sock_thread, inet_ntoa(cli_addr.sin_addr)};
        thpool_add_work(thpool, (void *)treat_socket, &arg);
    }
    close(sockfd);
    close(log_fd);

    hash__table_end();

    thpool_destroy(thpool);

    return 0;
}
