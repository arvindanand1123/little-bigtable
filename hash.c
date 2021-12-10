#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    GHashTable *hash_table = g_hash_table_new(NULL, NULL);
    time_t t;
    int n = 4096;

    srand((unsigned) time(&t));
    for(gint i = 0; i < n; i++){
        gint tmp_r = rand() % n;
        g_hash_table_insert(hash_table, GINT_TO_POINTER(i), GINT_TO_POINTER(tmp_r));
    }
    printf("There are %d keys in the hash table\n",
        g_hash_table_size(hash_table));
    for(gint i = 0; i < n; i++){
        printf("key: %d, val: %d \n", i, g_hash_table_lookup(hash_table, GINT_TO_POINTER(i)));
    }

    g_hash_table_destroy(hash_table);
    return 0;
}