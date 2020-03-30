#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"


void verif(struct file *f,char* key, char* IP, int port, char* name, int length, int piecesize){
    assert(strcmp(f->key,key) == 0);
    assert(strcmp(f->name,name) == 0);
    assert(f->length == length);
    assert(f->piecesize == piecesize);
        int n = 0;
    struct owner* own;
    SLIST_FOREACH(own,&f->owners,next_owner){
        if(strcmp(own->IP,IP) == 0)
            if(own->port == port)
                n = 1;
    }
    assert(n);
}

void test_hash__search(){
    printf("Test de hash__search: ");
    struct file* f = malloc(sizeof(struct file));
    //On ajoute un élément
    hash__add("LROTPbestdlesGAY4EVER","255.255.255.255",255,"Le delegue",2054,3);
    hash__search("LROTPbestdlesGAY4EVER",f);
    assert(strcmp(f->key,"LROTPbestdlesGAY4EVER") == 0);
    free(f);
    printf("SUCCESS\n");
}

void test_hash__add(){
    printf("Test de hash__add: ");
    struct file* f = malloc(sizeof(struct file));
    //On ajoute un élément
    hash__add("LROTPbestdlesGAY4EVER","255.255.255.255",255,"Le delegue",2054,3);
    hash__search("LROTPbestdlesGAY4EVER",f);
    verif(f,"LROTPbestdlesGAY4EVER","255.255.255.255",255,"Le delegue",2054,3);
    //On modifie cet élément en lui ajoutant un port avec même IP
    hash__add("LROTPbestdlesGAY4EVER","255.255.255.255",380,"-1",-1,-1);
    hash__search("LROTPbestdlesGAY4EVER",f);
    verif(f,"LROTPbestdlesGAY4EVER","255.255.255.255",255,"Le delegue",2054,3);
    verif(f,"LROTPbestdlesGAY4EVER","255.255.255.255",380,"Le delegue",2054,3);
    //On ajoute un autre élément avec une autre clef
    hash__add("3OGenstil","225.250.225.230",210,"Un delegue",1028,1);
    hash__search("3OGenstil",f);
    verif(f,"3OGenstil","225.250.225.230",210,"Un delegue",1028,1);
    free(f);
    printf("SUCCESS\n");
}

//Exemple d'utilisation de la hash_table
int main(int argc, char const *argv[]) {
    hash__table_init();
    test_hash__search();
    test_hash__add();
    hash__table_end();
    return 0;
}
