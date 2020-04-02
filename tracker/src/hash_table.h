/** 
 * @file hash_table.h
 * Hash table structures and methods in order to store FILES owned by peers
 */
#ifndef HASH_TABLE
#define HASH_TABLE
#include "queue.h"

/**
* \def HASH_TABLE_LENGTH hash_table.h
*/
#define HASH_TABLE_LENGTH 10000

/**
* @struct owner hash_table.h
*/
struct owner {
    char* IP; /** IP adress of the owner of the file */
    int port; /** port of the owner of the file */
    SLIST_ENTRY(owner) next_owner; /** next_owner for queue bsd lib*/
};

/**
 * @struct file hash_table.h
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
 * Initialize hash_table
 * @return hash_table variable initialized
 * */
void hash__table_init();

/**
 * Add a file in the hash_table with its key, its criterions and its owner
 * @param key unique key linked to the file
 * @param IP IP of the owner
 * @param port port where the owner listen to other peers
 * @param name filename
 * @param length total length of the file
 * @param piecesize size of each segment which will be downloaded between peers
 * 
 * @return 1 if the file has been added correctly, 0 otherwise
 * */
int hash__add(char* key,char* IP, int port,char* name, int length, int piecesize);


/**
 * Search a file in the hash_table depending of the key but also on different possible
 * criterions such as the total size with (with different comparators)
 * @param key key in order to search for the file
 * @param f file which will be returned depeping on the search
 * 
 * @return 1 and the file in f if the file has been found, 0 and f NULL otherwise 
 * */
int hash__search(char* key,struct file *f);

/**
 * Free all table memory by freeing each elements inside
 * @return hash_table freeed
 * */
void hash__table_end();

#endif
