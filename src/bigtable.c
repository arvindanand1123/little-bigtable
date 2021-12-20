#include "bigtable.h"

pthread_mutex_t *locks;
GHashTable **hash_tables;
time_t t_val;

#ifdef USE_VTUNE
	__itt_domain * itt_domain = NULL;
	__itt_string_handle * sh_sort = NULL; // the overall task name
	__itt_string_handle ** sh_parts = NULL; // per part task name

	#define vtune_task_begin(X) __itt_task_begin(itt_domain, __itt_null, __itt_null, sh_parts[X])
	#define vtune_task_end() __itt_task_end(itt_domain)
#else
	#define vtune_task_begin(X)
	#define vtune_task_end()
#endif


void insert(int id, gint key, gint val){
    //Lock
    //while(pthread_mutex_trylock(&locks[id])){printf("waiting to insert %d\n", key);}
    //printf("inserting: %d with val: %d\n", key, val);
    pthread_mutex_lock(&locks[id]);
    g_hash_table_insert(hash_tables[id], GINT_TO_POINTER(key), GINT_TO_POINTER(val));
    pthread_mutex_unlock(&locks[id]);
    //Unlock
}

int lookup(int id, gint key){
    //Lock
    //while(pthread_mutex_trylock(&locks[id])){printf("waiting to lookup %d\n", key);}
    pthread_mutex_lock(&locks[id]);
    int ret_val = (int)(g_hash_table_lookup(hash_tables[id], GINT_TO_POINTER(key)));
    pthread_mutex_unlock(&locks[id]);
    //Unlock
    return ret_val;
}

//modified hash function to hash directly to an allocated hash table
int mod_hash(int val){
    return g_direct_hash(val) % N;
}

//non zero random number
int mod_rand(){
    int ret = rand() % n;
    if(ret){
        return ret;
    }
    return mod_rand();
}

void thread_insert(int *block) {
#ifdef USE_VTUNE
    vtune_task_begin(block);
    int block_size = (int)(n/t);
    for(gint i = 0; i < block_size; i++){
        int hash = mod_hash(block[i]);

        gint tmp_r = mod_rand();
        insert(hash, block[i], tmp_r);
    }
    vtune_task_end();
#else
    int block_size = (int)(n/t);
    for(gint i = 0; i < block_size; i++){
        int hash = mod_hash(block[i]);
        gint tmp_r = mod_rand();
        insert(hash, block[i], tmp_r);
    }
#endif
}

void print_csv_line(char* test, int threadNum, int iterations, int numList, int numOperation, long long runTime) {
		fprintf(stdout, "test=%s threadNum=%d iterations=%d numList=%d numOperation=%d runTime(ms)=%lld tput(Mops)=%.2f\n",
    			    			test, threadNum, iterations, numList, numOperation, runTime/(1000*1000), 1000 * (double)numOperation/runTime);
}

void validate_hash_table(){
    int tmp = 0;
    for(int j = 0; j < N; j++){
        unsigned long table_size = g_hash_table_size(hash_tables[j]);
        printf("There are %d keys in hash table %d\n", table_size, j);
        for(gint i = 0; i < n; i++){
            int key = i;
            int val = lookup(j, key);
            if(val && p){
                printf("key: %d, val: %d \n", i, val);
            }
            if(val){
                tmp++;
            }
        }
    }
    //verify keys
    int block_size = (int)(n/t);
    if(tmp == block_size*t){
        printf("BigTable validated: all keys have been inserted and retrieved accurately. Hashtables are properly distributed.");
    } else {
        printf("Invalid table: %d", tmp);
    }

}

void clean_up(){
    for(int i = 0; i < N; i++){
        g_hash_table_destroy(hash_tables[i]);
        pthread_mutex_destroy(&locks[i]);
    }
}

void usage() {
	fprintf(stderr, "incorrect usage, please check README for more details... \n");
}

void alloc_locks(){
    locks = malloc(sizeof(pthread_mutex_t) * N);
    for(int i = 0; i < N; i++) {
            if(pthread_mutex_init(&locks[i], NULL) < 0){
                    perror("mutex");
                    exit(1);
            }
    }
}

void alloc_table(){
    hash_tables = malloc(sizeof(GHashTable *) * N);
    for (int i = 0; i < N; i++) {
        hash_tables[i] = g_hash_table_new(NULL, NULL);

	}
}

int main(int argc, char **argv) {
    int opt = 0;
    char input;
    struct timespec start_time, end_time;

    static struct option options [] = {
        {"N", 1, 0, 'N'},
        {"n", 1, 0, 'n'},
        {"t", 1, 0, 't'},
        {"p",0,0,'p'},
        {0, 0, 0, 0}
    };

    /* parse cmd args */
    while((opt=getopt_long(argc, argv, "its", options, NULL)) != -1){
        switch(opt){
            case 'N':
                N = (int)atoi(optarg);
                break;
            case 'n':
                n = (int)atoi(optarg);
                break;
            case 't':
                t = (int)atoi(optarg);
                break;
            case 'p':
                p = 1;
                break;
            default:
                usage();
                exit(1);
                break;
        }
    }

#ifdef USE_VTUNE
    itt_domain = __itt_domain_create("my domain");
		__itt_thread_set_name("my main");

		// pre create here, instead of doing it inside tasks
		sh_parts = malloc(sizeof(__itt_string_handle *) * t);
		assert(sh_parts);
		char itt_task_name[32];
		for (int i = 0; i < t; i++) {
			snprintf(itt_task_name, 32, "part-%d", i);
			sh_parts[i] = __itt_string_handle_create(itt_task_name);
		}
#endif

    if(N > n || t > N){
        usage();
        exit(1);
    }

    srand((unsigned) time(&t_val));

     /* launch threads & exec test */
    alloc_locks();
    alloc_table();

#ifdef USE_VTUNE
    __itt_task_begin(itt_domain, __itt_null, __itt_null,
    					__itt_string_handle_create("list"));
#endif
    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) < 0){
        print_errors("clock_gettime");
    }
    /* separate into blocks */
    int block_size = (int)(n/t);
    pthread_t threads[t];
    int blocks[t][block_size];
    printf("\n---------------Telemetry---------------\n");
    printf("Block Size: %d\n", block_size);
    for(int i = 0; i < t; i++){
        printf("Thread: %d\n",i);
        for(int j = 0; j < block_size; j++){
            int offset = i*block_size;
            blocks[i][j] = j + offset;
            if(p){
                printf("Block val: %d\n", blocks[i][j]);
            }
        }
        pthread_create(&threads[i], NULL, thread_insert, &blocks[i]);
    }

#ifdef USE_VTUNE
    __itt_task_end(itt_domain);
    free(sh_parts);
#endif

    printf("---------------------------------------\n");

	for (int i = 0; i < t; i++) {
		if (pthread_join(threads[i], NULL) < 0) {
				perror("thread_join");
				exit(1);
		}
	}

    if(clock_gettime(CLOCK_MONOTONIC, &end) < 0){
        print_errors("clock_gettime");
    }
    
    long long diff = (end.tv_sec - start.tv_sec) * ONE_BILLION;
    diff += end.tv_nsec;
    diff -= start.tv_nsec;

    int numOpts = n+block_size; //Big-oh estimation of ops

    print_csv_line("BigTable", t, n, N, numOpts, diff);

	atexit(clean_up);
	exit(0);
}