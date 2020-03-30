#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hash_table.h"

/**
* Fonction de hachage
*/
int hash_value(char* key){
    int hash = 0;
    for (int i = 0; i < strlen(key); i++)
        hash = 31 * hash + key[i];
    return abs(hash%HASH_TABLE_LENGTH);
}

/**
* Fonction qui update la structure
*/
int compare(struct file* f,char* IP, int port,char* name, int length, int piecesize){
    if(strcmp(name,"-1") && !strcmp(name,f->name))
        f->name = name;
    if(length != -1 && length != f->length)
        f->length = length;
    if(piecesize != -1 && piecesize != f->piecesize)
        f->piecesize = piecesize;
    if(strcmp(IP,"-1") && port != -1){
        struct owner* own;
        SLIST_FOREACH(own,&f->owners,next_owner)
            if(!strcmp(IP,own->IP) && port == own->port)
                return 1;
        own = malloc(sizeof(struct owner));
        own->IP = IP;
        own->port = port;
        SLIST_INSERT_HEAD(&f->owners,own,next_owner);
        f->nb_owners += 1;
    }
    return 1;
}

/**
* Fonction qui ajoute un fichier ou met à jour un fichier
*/
int hash__add(char* key,char* IP, int port,char* name, int length, int piecesize){
    int bool = 0;
    int index = hash_value(key);
    struct file *p;
    SLIST_FOREACH(p,&hash_table[index],next_file){
        if(!strcmp(p->key,key)){
            bool = compare(p,IP,port,name,length,piecesize);
            return bool;
        }
    }
    struct file* f = malloc(sizeof(struct file));
    f->key = key;
    SLIST_INIT(&f->owners);
    struct owner* own = malloc(sizeof(struct owner));
    own->IP = IP;
    own->port = port;
    SLIST_INSERT_HEAD(&f->owners,own,next_owner);
    f->nb_owners = 1;
    f->name = name;
    f->length = length;
    f->piecesize = piecesize;
    SLIST_INSERT_HEAD(&hash_table[index],f,next_file);
    bool = 1;
    return bool;
}

/**
* Recherche le fichier key et le renvoie dans f
*/
int hash__search(char* key,struct file *f){
    int index = hash_value(key);
    struct file *p;
    SLIST_FOREACH(p,&hash_table[index],next_file){
        if(!strcmp(p->key,key)){
            *f = *p;
            return 1;
        }
    }
    f = NULL;
    return 0;
}

/**
* Initialise la hash_table
*/
void hash__table_init(){
    for(int i = 0; i<HASH_TABLE_LENGTH;i++)
        SLIST_INIT(&hash_table[i]);
}

/**
* Libère la mémoire
*/
void hash__table_end(){
    for(int i = 0; i<HASH_TABLE_LENGTH; i++){
        struct file *p;
        SLIST_FOREACH(p,&hash_table[i],next_file){
            struct owner* own;
            SLIST_FOREACH(own,&p->owners,next_owner)
                free(own);
            free(p);
        }
    }
}
