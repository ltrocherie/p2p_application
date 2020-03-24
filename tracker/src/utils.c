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

void usage()
{
    printf("./tracker [port]\n");
    return;
}

//This function returns host name for local computer
void check_host_name(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

//find host info from host name
void check_host_entry(struct hostent *hostentry)
{
    if (hostentry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
}

//convert IP string to dotted decimal format
void IP_formatter(char *IPbuffer)
{
    if (NULL == IPbuffer)
    {
        perror("inet_ntoa");
        exit(1);
    }
}

void print_ip(unsigned int ip)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    printf("%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);
}

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

void get_command(char *buffer, char *command)
{
    char *p, space = ' ';
    int i;

    /* Returns the pointer of the first occurence of the separator */
    p = strchr(buffer, space);

    /* If there is no space in the command */
    if (p == NULL)
    {
        printf("No %c found.\n", space);
        return;
    }

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