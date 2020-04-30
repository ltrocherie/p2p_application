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
//#include "dict.h"

#define SIZE 1024

#define ANNOUNCE "announce"
#define LOOK "look"
#define UPDATE "update"
#define GET "getfile"

#define LISTEN "listen"

#define SEED "seed"
#define LEECH "leech"

#define CONFIG "./config.ini"
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
        exit_if(send(socket, "nok", 3,0) == -1, "ERROR sending to socket");
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

    /* Verify the port is a number */
    if (!isNumeric(port_arg))
    {
        exit_if(send(socket, "nok", 3,0) == -1, "ERROR sending to socket");
        return;
    }

    int port = atoi(port_arg);
    // ajout du couple IP:port dans un tableau d'owners
    //hash__add_ip_port(ip,port);
    //si hash__add_ip_port == 0 -> erreur ajout dans la table

    /* SEEDS LEECHS TREATMENT */

    int end = 0;
    int seed = 0, leech = 0;
    int seed_arg = 0;

    (void)seed;
    (void)leech;

    char arg[1024];

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

                        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                        fprintf(stderr, "Size of piecesize must be integers\n");
                        _log(log_fd, "\nSize of piecesize must be integers", "ERROR write log"); //exit_if ( write(log_fd, "\nSize of piecesize must be integers", 35) == -1, "ERROR write log" );
                        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                        exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                        return;
                    }

                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stdout,"add seed by:%s | add:%s|key:%s|taille:%d|piece:%d\n", IP, seeds[0], seeds[3],atoi(seeds[1]), atoi(seeds[2]));
                    _log(log_fd, "\nadd by :", "ERROR write log");
                    _log(log_fd, IP, "ERROR write log");
                    _log(log_fd, "\nkey ", "ERROR write log");
                    _log(log_fd, seeds[3], "ERROR write log");
                    _log(log_fd, "\nadd ", "ERROR write log");
                    _log(log_fd, seeds[0], "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                    int add = hash__add_seeder(seeds[3], IP, port, seeds[0], atoi(seeds[1]), atoi(seeds[2]));

                    if (!add)
                    {
                        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                        fprintf(stderr, "Problem adding in hash_table");
                        _log(log_fd, "\nProblem adding in hash_table", "ERROR write log");
                        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                        exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
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

                if (strcmp(key_leech,"\0")) {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stdout,"add leech by :%s | key:%s\n", IP, key_leech);
                    _log(log_fd, "\nadd leech by :", "ERROR write log");
                    _log(log_fd, IP, "ERROR write log");
                    _log(log_fd, "\nleech key ", "ERROR write log");
                    _log(log_fd, key_leech, "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                    //hash__add_leecher(IP, port,key_leech );
                }

                tmp = 0;
                break;
            }

            /* Not key words recognized yet */
            else {
                arg[tmp] = '\0';
                tmp = 0;

                /* if seed/leech key word is given */
                if (!strcmp(arg,SEED)) {
                    seed = 1;
                    seed_arg = 0;
                    leech = 0;
                } else if (!strcmp(arg,LEECH)) {
                    leech = 1;
                    seed = 0;
                }
                /* error case */
                else {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stderr, "Wrong key word");
                    _log(log_fd, "\nNo key word", "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                    exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                    return;
                }
            }

            i++;
            break;
        case '[':

            /* if we start by giving the seeds */
            if (!seed && !leech) {
                exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                fprintf(stderr, "Bracket without key word\n");
                _log(log_fd, "\nBracket without key word", "ERROR write log");
                exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
                
                exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
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
                        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                        fprintf(stderr, "Size of piecesize must be integers\n");
                        _log(log_fd, "\nSize of piecesize must be integers", "ERROR write log");
                        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
                        
                        exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                        return;
                    }

                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stdout,"add seed by:%s | add:%s|key:%s|taille:%d|piece:%d\n", IP, seeds[0], seeds[3],atoi(seeds[1]), atoi(seeds[2]));
                    _log(log_fd, "\nadd by :", "ERROR write log");
                    _log(log_fd, IP, "ERROR write log");
                    _log(log_fd, "\nkey ", "ERROR write log");
                    _log(log_fd, seeds[3], "ERROR write log");
                    _log(log_fd, "\nadd ", "ERROR write log");
                    _log(log_fd, seeds[0], "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                    int add = hash__add_seeder(seeds[3], IP, port, seeds[0], atoi(seeds[1]), atoi(seeds[2]));

                    if (!add) {
                        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                        fprintf(stderr, "Problem adding in hash_table");
                        _log(log_fd, "\nProblem adding in hash_table", "ERROR write log");
                        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                        exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                        return;
                    }
                    seed_arg = 0;
                }
                /* Else if not enough args in seeds */
                else if (seed_arg != 0) {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stderr, "Not enough parameters for a file\n");
                    _log(log_fd, "\nNot enough parameters for a file", "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
                    
                    exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                    return;
                }
                seed = 0;
            }
            /* LEECH CASE : at the end of every word we show the key */
            else if (!seed && leech) {
                /* Finish the word */
                key_leech[tmp] = '\0';

                if (strcmp(key_leech,"\0")) {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stdout,"add leech by :%s | key:%s\n", IP, key_leech);
                    _log(log_fd, "\nadd leech by :", "ERROR write log");
                    _log(log_fd, IP, "ERROR write log");
                    _log(log_fd, "leech key ", "ERROR write log");
                    _log(log_fd, key_leech, "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                    //hash__add_leecher(IP, port,key_leech );
                }
                leech = 0;

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
            else
                arg[tmp] = buffer[i];

            tmp++;
            i++;
            break;
        }
    }

    hash__print();
    /* Everything happened good */
    exit_if ( send(socket, "ok", 2, 0) == -1, "ERROR sending to socket" );
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
    char name[1024] = "-1";
    char size[64] = "-1";
    char comparator = '=';
    int given_size = 0, given_name = 0;
    int end = 0;

    /* Read all characters */
    while (buffer[i] != 0 && buffer[i] != '\n' && !end)
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

            if (!strcmp(arg, "filename")) {
                given_name = 1;
                given_size = 0;
            }
            else if (!strcmp(arg, "filesize")) {
                given_name = 0;
                given_size = 1;
            }
            else
            {
                send(socket, "nok", 3, 0);
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
                send(socket, "nok", 3, 0);
                return;
            }

            given_size = 1;
            given_name = 0;

            comparator = buffer[i];

            i++;
            break;
        case '<':
            arg[tmp] = '\0';
            tmp = 0;

            /* If the comparator is < with other than filesize, this is t valid */
            if (strcmp(arg, "filesize") != 0)
            {
                send(socket, "nok", 3, 0);
                return;
            }

            given_size = 1;
            given_name = 0;

            comparator = buffer[i];

            i++;
            break;
        case '[':
            i++;
            break;
        case ']':
            if (buffer[i-1] != '"') {
                send(socket, "nok", 3, 0);
                return;
            }

            end = 1;
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

    if (!end) {
        send(socket, "nok", 3, 0);
        return;
    }

    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
    printf("look by :%s\n", IP);
    printf("filename:%s\n", name);
    printf("size:%s\n", size);
    printf("comparator:%c\n", comparator);
    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

    if (!isNumeric(size))
    {
        send(socket, "nok", 3, 0);
        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
        _log(log_fd, "\nNaN size", "ERROR write log");
        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
        return;
    }

    if (given_size && atoi(size) < 0) {
        send(socket, "nok", 3, 0);
        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
        _log(log_fd, "\nNegative size", "ERROR write log");
        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

        return;
    }

    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
    _log(log_fd, "\nlook by :", "ERROR write log");
    _log(log_fd, IP, "ERROR write log");
    _log(log_fd, "\nfilename:", "ERROR write log");  
    _log(log_fd, name, "ERROR write log");           
    _log(log_fd, "\nsize:", "ERROR write log");      
    _log(log_fd, size, "ERROR write log");           
    _log(log_fd, "\ncomparator:", "ERROR write log");
    _log(log_fd, &comparator, "ERROR write log");
    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

    char find[1024] = {'\0'};
    strcat(find,"> list [");
    hash__getfiles(name, comparator, atoi(size), find);

    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
    printf("%s\n", find);
    printf("%d\n", atoi(size));
    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

    strcat(find,"]");

    send(socket,find,strlen(find)*sizeof(char),0);

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
        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
        fprintf(stderr, "No \"%c\" found.\n", space);
        _log(log_fd, "\nNo \"%c\" found", "ERROR write log");
        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

        exit_if (send(socket,"nok", 3,0) == -1, "ERROR sending to socket");
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
                exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                printf("update by :%s | seed:%s\n",IP, key);
                _log(log_fd, "\nupdate by :", "ERROR write log");
                _log(log_fd, IP, "ERROR write log");
                _log(log_fd, "\nupdate ", "ERROR write log");
                _log(log_fd, "\nkey ", "ERROR write log");   
                _log(log_fd, key, "ERROR write log");        
                exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                struct file* tmp = hash__search(key);
                if (tmp == NULL) {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stderr, "\nFile does not exist");
                    _log(log_fd, "\nFile does not exist", "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
                    
                    exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                    return;
                }

                int add = hash__add_seeder(key
                                            ,IP
                                            ,1000
                                            ,tmp->name
                                            ,tmp->length
                                            ,tmp->piecesize);
                if (!add) {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stderr, "Problem adding in hash_table");
                    _log(log_fd, "\nProblem adding in hash_table", "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                    exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                    return;
                }
            }
            /* Leeching case */
            else if (leech)
            {
                /* Add leeching ??? */
                exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                printf("update by :%s | leech:%s\n",IP, key);
                _log(log_fd, "\nupdate by :", "ERROR write log");
                _log(log_fd, IP, "ERROR write log");
                _log(log_fd, "key ", "ERROR write log"); //exit_if ( write(log_fd, "key ", 4) == -1, "ERROR writing log" );
                _log(log_fd, key, "ERROR write log"); //exit_if ( write(log_fd, key, strlen(key)*sizeof(key)) == -1, "ERROR writing log" );
                exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                //hash__add_leecher(IP, port,key_leech );

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
                break;

            /* Seed case */
            if (seed)
            {
                seed = 0;

                /* We add a new file OR a new owner in the existing file */
                exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                printf("update by :%s | seed:%s\n",IP, key);
                _log(log_fd, "\nupdate by :", "ERROR write log");
                _log(log_fd, IP, "ERROR write log");
                _log(log_fd, "\nupdate ", "ERROR write log"); //exit_if ( write(log_fd, "\nupdate ", 8) == -1, "ERROR write log" );
                _log(log_fd, "\nkey ", "ERROR write log"); //exit_if ( write(log_fd, "\nkey ", 5) == -1, "ERROR write log" );
                _log(log_fd, key, "ERROR write log"); //exit_if ( write(log_fd, key, strlen(key)*sizeof(char)) == -1, "ERROR write log" );
                exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                struct file* tmp = hash__search(key);

                if (tmp == NULL) {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stderr, "\nFile does not exist");
                    _log(log_fd, "\nFile does not exist", "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
                    
                    exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                    return;
                }

                int add = hash__add_seeder(key
                                            ,IP
                                            ,1000
                                            ,tmp->name
                                            ,tmp->length
                                            ,tmp->piecesize);
                if (!add) {
                    exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                    fprintf(stderr, "Problem adding in hash_table");
                    _log(log_fd, "\nProblem adding in hash_table", "ERROR write log");
                    exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

                    exit_if ( send(socket, "nok", 3, 0) == -1, "ERROR sending to socket" );
                    return;
                }

                hash__print();
            }
            /* Leeching case */
            else if (leech)
            {
                leech = 0;
                end = 1;

                /* Add leeching ??? */
                exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
                printf("update by :%s | leech:%s\n",IP, key);
                _log(log_fd, "\nupdate by :", "ERROR write log");
                _log(log_fd, IP, "ERROR write log");
                _log(log_fd, "key ", "ERROR write log");
                _log(log_fd, key, "ERROR write log");   
                exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
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
    exit_if ( send(socket, "ok", 3,0) == -1, "ERROR sending to socket");
}

void getfile(int socket, char *buffer, char *IP)
{
    char *p, space = ' ';

    /* Returns the pointer of the first occurence of the separator */
    p = strchr(buffer, space);

    /* If there is no space in the command */
    if (p == NULL)
    {
        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
        fprintf(stderr, "No \"%c\" found.\n", space);
        _log(log_fd, "\nNo \"%c\" found", "ERROR write log");
        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");

        exit_if (send(socket,"nok", 3,0) == -1, "ERROR sending to socket");
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
        exit_if (pthread_mutex_lock(&log_lock), "Error mutex lock log");
        fprintf(stderr, "get file by:%s | File with key %s not found in the hash table\n", IP, key);
        _log(log_fd, "\ngetfile by :", "ERROR write log");
        _log(log_fd, IP, "ERROR write log");
        _log(log_fd, "\nFile with key ", "ERROR write log"); //exit_if( write(log_fd,"\nFile with key ",15) == -1, "ERROR writing log" );
        _log(log_fd, key, "ERROR write log"); //exit_if( write(log_fd,key,strlen(key)*sizeof(char)) == -1, "ERROR writing log" );
        _log(log_fd, " not found in the hash table", "ERROR write log"); //exit_if( write(log_fd," not found in the hash table",28) == -1, "ERROR writing log" );
        exit_if (pthread_mutex_unlock(&log_lock), "Error mutex unlock log");
        
        exit_if (send(socket,"nok", 3,0) == -1, "ERROR sending to socket");
        return;
    }

    hash__print();
    char msg[1024] = {'\0'};
    strcat(msg,"> peers ");
    strcat(msg,key);
    strcat(msg," [");

    /* Writing peers */
    int nb_seeder = 0;
    struct seeder *seed;
    SLIST_FOREACH(seed, &f->seeders, next_seeder)
    {
        strcat(msg,seed->IP);
        strcat(msg,":");
        char* port = itoa(seed->port,10);
        strcat(msg,port);
        if (nb_seeder != f->nb_seeders - 1)
            strcat(msg," ");

        nb_seeder++;
    }
    strcat(msg,"]");
    send(socket,msg,strlen(msg)*sizeof(char),0);

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
    printf("Socket:%d\n",socket);

    _log(log_fd, "\nIP Received:", "ERROR write log"); //exit_if( write(log_fd,"\nIP Received:",13) == -1, "ERROR writing log" );
    _log(log_fd, ip, "ERROR write log"); //exit_if( write(log_fd,ip,strlen(ip)*sizeof(char)) == -1, "ERROR writing log" );
    _log(log_fd, "\nSocket:", "ERROR write log"); //exit_if( write(log_fd,"\nSocket:",7) == -1, "ERROR writing log" );
    _log(log_fd, itoa(socket,10), "ERROR write log"); //exit_if( write(log_fd,itoa(socket,10),strlen(itoa(socket,10))*sizeof(char)) == -1, "ERROR writing log" );

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
        exit_if(send(socket, "nok", 3,0) == -1, "ERROR sending to socket");
    close(socket);
    return;
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, *sock_thread, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* Pool of threads which will retrieve work to do in the queue of work */
    threadpool thpool = thpool_init(5);

    /* init a dictionary of users */
    //where all users have their own unique IP adresse for update


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
    
    exit_if ( pthread_mutex_destroy(&log_lock) != 0, "ERROR init mutex" );  
    close(log_fd);

    hash__table_end();

    thpool_destroy(thpool);

    return 0;
}
