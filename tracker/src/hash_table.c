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
 * update 1 file with differents criterions : if a file is the same as all criterions
 * then we add a new seeder in it
 * @param f given f in the hash table which will be updated to all criterions
 * @param IP IP of the seeder
 * @param port port where the seeder listen to other peers
 * @param name filename
 * @param length total length of the file
 * @param piecesize size of each segment which will be dseedloaded between peers
 *
 * @return hashed index value : where to store the file
 * */
int hash__update_seeder(struct file* f,char* IP, int port,char* name, int length, int piecesize){
    /* Error cases */
    if(!strcmp(name,"\0") || strcmp(name,f->name))
        return 0;
    if(length == -1 || length != f->length)
        return 0;
    if(piecesize == -1 || piecesize != f->piecesize)
        return 0;

    struct seeder* seed;
    SLIST_FOREACH(seed,&f->seeders,next_seeder)
        if(!strcmp(IP,seed->IP) && port == seed->port)
            return 1;

    /* Add a new seeder in the list */
    seed = malloc(sizeof(struct seeder));
    seed->IP = malloc(sizeof(char) * 16);
    strcpy(seed->IP,IP);

    seed->port = port;
    SLIST_INSERT_HEAD(&f->seeders,seed,next_seeder);
    f->nb_seeders += 1;
    return 1;
}

int hash__add_seeder(char* key,char* IP, int port,char* name, int length, int piecesize){

    int bool = 0;
    int index = hash_value(key);
    struct file *p;
    pthread_mutex_lock(&mutex_table[index]);
    SLIST_FOREACH(p,&hash_table[index],next_file){
        /* If we find the key : verify all criterions then add a new seeder */
        if(!strcmp(p->key,key)){
            bool = hash__update_seeder(p,IP,port,name,length,piecesize);
            pthread_mutex_unlock(&mutex_table[index]);
            return bool;
        }
    }

    /* Create a new file */
    struct file* f = malloc(sizeof(struct file));
    f->key = malloc(sizeof(char) * 1024);
    f->name = malloc(sizeof(char) * 1024);

    strcpy(f->key,key);
    f->nb_seeders = 1;
    strcpy(f->name,name);
    f->length = length;
    f->piecesize = piecesize;

    SLIST_INIT(&f->seeders);
    LIST_INIT(&f->leechers);
    struct seeder* seed = malloc(sizeof(struct seeder));
    seed->IP = malloc(sizeof(char) * 16);
    strcpy(seed->IP,IP);

    seed->port = port;
    SLIST_INSERT_HEAD(&f->seeders,seed,next_seeder);

    SLIST_INSERT_HEAD(&hash_table[index],f,next_file);
    pthread_mutex_unlock(&mutex_table[index]);

    bool = 1;
    return bool;
}

int hash__add_leecher(char* key, char* IP, int port){
    int nb = 0;
    int found = 1;
    int index = hash_value(key);
    struct file *p;
    pthread_mutex_lock(&mutex_table[index]);
    SLIST_FOREACH(p,&hash_table[index],next_file){
        if(!strcmp(p->key,key_table[i])){
            struct leecher *l;
            LIST_FOREACH(l,&p->leechers,next_leecher)
                if(!strcmp(IP,l->IP) || port == l->port){
                    found = 0;
                    nb ++;
                }
            if(found){
                struct leecher *leech = malloc(sizeof(struct leecher));
                leech->IP = malloc(sizeof(char) * 16);
                strcpy(leech->IP,IP);
                leech->port = port;
                LIST_INSERT_HEAD(&p->leechers,leech,next_leecher);
                p->nb_leechers ++;
                nb ++;
            }
        }
    }
    pthread_mutex_unlock(&mutex_table[index]);
    return nb;
}


struct file* hash__search(char* key){
    int index = hash_value(key);
    struct file *p;
    pthread_mutex_lock(&mutex_table[index]);
    SLIST_FOREACH(p,&hash_table[index],next_file){
        if(!strcmp(p->key,key)){
            pthread_mutex_unlock(&mutex_table[index]);
            return p;
        }
    }
    pthread_mutex_unlock(&mutex_table[index]);
    return NULL;
}

void hash__table_init(){
    for(int i = 0; i<HASH_TABLE_LENGTH;i++){
        SLIST_INIT(&hash_table[i]);
        pthread_mutex_init(&mutex_table[i],NULL);
    }
}

char* itoa(int val, int base){
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}

/**
 * Compare 1 file with differents criterions : if a file has the good name or the good size
 * @param compn is the comparator
 * @param size is the size wanted
 * @param size2 is the size of the file
 *
 * @return the true if all criterions is respected, false otherwise
 * */
int has_size(char compn, int size, int size2){
    if(size == -1)
        return 1;
    if('<' == compn && size2 < size)
        return 1;
    if('>' == compn && size2 > size)
        return 1;
    if('=' == compn && size2 == size)
        return 1;
    return 0;
}

int hash__getfiles(char* name, char compn, int size,char* files_found){
    int nb = 0;
    if(size == -1 && !strcmp(name,"-1"))
        return nb;
    for(int i = 0;i<HASH_TABLE_LENGTH;i++){
        struct file *f;
        pthread_mutex_lock(&mutex_table[i]);
        SLIST_FOREACH(f,&hash_table[i],next_file){
            if(has_size(compn,size,f->length) && (!strcmp(f->name,name) || !strcmp(name,"-1"))){
                    if(nb != 0)
                        strcat(files_found," ");
                    strcat(files_found,f->name);
                    strcat(files_found," ");
                    strcat(files_found,itoa(f->length,10));
                    strcat(files_found," ");
                    strcat(files_found,itoa(f->piecesize,10));
                    strcat(files_found," ");
                    strcat(files_found,f->key);
                    nb++;
            }
        }
        pthread_mutex_unlock(&mutex_table[i]);
    }
    return nb;
}

void hash__print(){
    for(int i = 0; i<HASH_TABLE_LENGTH;i++){
        pthread_mutex_lock(&mutex_table[i]);
        struct file *f;
        SLIST_FOREACH(f,&hash_table[i],next_file){
            printf("A l'indice %d,il y a %s %s [",i,f->name,f->key);
            struct seeder *seed;
            SLIST_FOREACH(seed,&f->seeders,next_seeder){
                    printf("%s:%d",seed->IP,seed->port);
            }
            printf("]\n");
        }
        pthread_mutex_unlock(&mutex_table[i]);
    }
}

void hash__table_end(){
    for(int i = 0; i<HASH_TABLE_LENGTH; i++){
        struct file *p;
        SLIST_FOREACH(p,&hash_table[i],next_file){
            struct seeder* seed;
            SLIST_FOREACH(seed,&p->seeders,next_seeder) {
                free(seed->IP);
                free(seed);
            }
            struct leecher* leech;
            LIST_FOREACH(leech,&p->leechers,next_leecher){
                free(leech->IP);
                free(leech);
            }
            free(p->key);
            free(p->name);
            free(p);
        }
        pthread_mutex_destroy(&mutex_table[i]);
    }
}
