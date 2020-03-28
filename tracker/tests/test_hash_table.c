#include "hash_table.h"

//Exemple d'utilisation de la hash_table
int main(int argc, char const *argv[]) {
    hash_table_init();
    add("LROTPbdlesGAY4EVER","255.255.255.255",255,"Le delegue",2054);
    add("C3PO","0.0.0.0",254,"Le petit robot",1027);
    add("LROTPbdlesGAY4EVER","255.255.255.255",33,"Le delegue",2052);
    struct file* f = malloc(sizeof(struct file));
    search("LROTPbdlesGAY4EVER",f);
    if(f->key == NULL){
        hash_table_end();
        return 0;
    }
    printf("%s %s %d\n",f->key,f->name,f->length);
    struct owner* own;
    SLIST_FOREACH(own,&f->owners,next_owner)
        printf("%s %d\n",own->IP,own->port);
    free(f);
    hash_table_end();
    return 0;
}
