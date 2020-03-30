#ifndef HASH_TABLE
#define HASH_TABLE
#include "queue.h"

//Taille de la hash_table
#define HASH_TABLE_LENGTH 10000

//Structure de propritaire de fichier
struct owner {
    char* IP;
    int port;
    SLIST_ENTRY(owner) next_owner;
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

//Table de hachage
SLIST_HEAD(,file) hash_table[HASH_TABLE_LENGTH];

//Initialiser la hash_table
void hash_table_init();

//Ajoute ou met à jour un fichier de clé fey
void add(char* key,char* IP, int port,char* name, int length, int piecesize);

//Recherche un fichier à partir de sa clé key et renvoie le fichier f
void search(char* key,struct file *f);

//A utiliser pour libérer la hash_table
void hash_table_end();

#endif
