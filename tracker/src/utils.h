#ifndef UTILS_H
#define UTILS_H

/**
 * exit printing error prefixed by `prefix` if `condition` is true (non zero)
 */
void exit_if(int condition, const char *prefix);

void usage_commands(int socket);

void print_ip(unsigned int ip);
void show_local_ip(int portno);
void get_command(char *buffer, char *command);

int isNumeric (const char * s);

#endif