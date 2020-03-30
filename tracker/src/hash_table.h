#ifndef HASH_TABLE
#define HASH_TABLE
#include "queue.h"

/**
* \def HASH_TABLE_LENGTH hash_table.h
*/
#define HASH_TABLE_LENGTH 10000

/**
* \struct owner hash_table.h
*/
struct owner {
    char* IP; /** IP adress of the owner of the file */
    int port; /** port of the owner of the file */
    SLIST_ENTRY(owner) next_owner; /** next_owner for queue bsd lib*/
};

/**
 * \struct file hash_table.h
 */
struct file {
        char *key; /** pseudo-unique key of the file */
        char *name; /** filename */
        int length; /** total length of the file */
        int piecesize; /** size of the piece */

        int nb_owners; /** total number of owners */
        SLIST_HEAD(,owner) owners; /** list of owners of the file */

        SLIST_ENTRY(file) next_file; /** next_file for queue bsd lib */
};

/**
* \var hash_table hash_table.h
*/
SLIST_HEAD(,file) hash_table[HASH_TABLE_LENGTH];

/**
* \fn hash__table_init() hash_table.h
*/
void hash__table_init();

/**
* \fn hash__add() hash_table.h
*/
int hash__add(char* key,char* IP, int port,char* name, int length, int piecesize);


/**
* \fn hash__search() hash_table.h
*/
int hash__search(char* key,struct file *f);

/**
* \fn hash__table_end() hash_table.h
*/
void hash__table_end();

#endif
