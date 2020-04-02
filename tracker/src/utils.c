#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

/**
 * Print how to run the tracker
 */
void usage()
{
    printf("./tracker [port]\n");
    return;
}

/**
 * Print all different commands that a peer can send to the tracker
 */
void usage_commands()
{
    printf("List of possible commands :\n"
            "- announce listen <local port> seed [file1 length1 piecesize1 key1 ...] leech [key2 key3 ...] : announce the peer is listen on his local port and seeding given files and leeching given keys\n"
            "- look [criterion1 criterion2] : returns a list of files with all criterions (=, >, < with size in Mo)\n"
            "- getfile key : returns a list of peers who have the file\n"
            "- update seed [key1 key2 ...] leech [key10 key11 ...] : warns the trackers of the new files the peer has\n");
    return;
}

/**
 * Return host name for local computer
 */
void check_host_name(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

/**
 * Find host info from host name 
 */
void check_host_entry(struct hostent *hostentry)
{
    if (hostentry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
}

/**
 * Convert IP string to dotted decimal format
 */
void IP_formatter(char *IPbuffer)
{
    if (NULL == IPbuffer)
    {
        perror("inet_ntoa");
        exit(1);
    }
}

//TODO : faire en sorte que cela ne soit pas l'ip locale d'affichée
void show_local_ip(int portno)
{
    /* 
    char host[256];
    char *IP;
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host)); //find the host name
    check_host_name(hostname);
    host_entry = gethostbyname(host); //find host information
    check_host_entry(host_entry);
    IP = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    //Convert into IP string
    printf("Current Host Name: %s\n", host);
    printf("Host IP: %s\n", IP);
    printf("Listening on port %d...\n", portno);

    */

    struct ifaddrs *id;
    int val;
    val = getifaddrs(&id);

    if (val != 0) {
        fprintf(stderr, "getifaddrs");
        return;
    }

    printf("Network Interface Name :- %s\n", id->ifa_name);
    //printf("Network Address of %s :- %d\n", id->ifa_name, id->ifa_addr);
    printf("Socket Data : -%s\n", id->ifa_addr->sa_data);
}

/**
 * Retrieve the command and fill the command buffer given in parameter
 * @param buffer : buffer filled with all the 1024 bytes buffer g=iven by the peer
 * @param command : empty buffer which will be filled
 * @return command buffer filled with the command if a command is recognized, not filled otherwise
 */
void get_command(char *buffer, char *command)
{
    char *p, space = ' ';
    int i;

    /* Returns the pointer of the first occurence of the separator */
    p = strchr(buffer, space);

    /* If there is no space in the command */
    if (p == NULL)
    {
        fprintf(stderr,"No %c found.\n", space);
        return;
    }

    /* Position of the first itération */
    i = p - buffer;

    int seed = 0, leech = 0;
    /* Get the command */
    strncpy(command, &buffer[0], i);
    command[i] = '\0';

    /* to lower */
    int j = 0;
    while(command[j]) {
        command[j] = tolower(command[j]);
        j++;
    }

    return;
}

int isNumeric (const char * s)
{
    if (s == NULL || *s == '\0' || isspace(*s))
      return 0;
    char * p;
    strtod (s, &p);
    return *p == '\0';
}
