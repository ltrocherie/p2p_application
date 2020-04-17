#ifndef UTILS_H
#define UTILS_H

void usage();
void usage_commands(int socket);

void print_ip(unsigned int ip);
void show_local_ip(int portno);
void get_command(char *buffer, char *command);

int isNumeric (const char * s);

#endif