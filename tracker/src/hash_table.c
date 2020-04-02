/**
 * @file hash_table.c
 * */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hash_table.h"

/**
 * Hash function in order to find index where to store elements
 * @param key unique key of the file
 * 
 * @return hashed index value : where to store the file 
 * */
int hash_value(char* key){
    int hash = 0;
    for (int i = 0; i < strlen(key); i++)
        hash = 31 * hash + key[i];
    return abs(hash%HASH_TABLE_LENGTH);
}

/**
 * Compare 1 file with differents criterions : if a file is the same as all criterions
 * then we add a new owner in it
 * @param f given f in the hash table which will be compared to all criterions
 * @param IP IP of the owner
 * @param port port where the owner listen to other peers
 * @param name filename
 * @param length total length of the file
 * @param piecesize size of each segment which will be downloaded between peers
 * 
 * @return hashed index value : where to store the file 
 * */
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

void hash__table_init(){
    for(int i = 0; i<HASH_TABLE_LENGTH;i++)
        SLIST_INIT(&hash_table[i]);
}

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