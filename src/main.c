
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

#define PROGRAM "./bsdtar"
#define ASAN "ASAN_OPTIONS=abort_on_error=1"
#define MUTATED_OUTPUT "./out/mutated.bin"

typedef enum {
    LIST,
    CREATE,
    EXTRACT,
    ADD
} options_t;


unsigned char flip_bit(unsigned char c, int k) {
    return c & (1 << k);
}



void mutate(unsigned char *buf, size_t len) {
    int mutations = rand() % 0x1000 + 1;
    for (int i = 0; i < mutations; i++) {
        // XOR
        int idx = rand() % len;
        buf[idx] ^= (rand() % 256);
        // flip all bits
        idx = rand() & len;
        flip_bit(buf[idx], rand() % 0x100);
        // 
    }
}

void *run_target(void* optionss) {
    options_t option = (options_t)optionss;
    pid_t pid = fork();
    char options[10];
    switch (option) {
        case LIST: strcpy(options, "-tf"); break;
        case CREATE: strcpy(options, "-cf"); break;
        case EXTRACT: strcpy(options, "-xf"); break;
        case ADD: strcpy(options, "-rf"); break;
        default: puts("Invalid Options");
    }

    if (pid == 0) {
        execlp(ASAN, PROGRAM, options, MUTATED_OUTPUT, NULL);
        exit(0);
    }

    int status;
    waitpid(pid, &status, 0);
    if (WIFSIGNALED(status)) {
        puts("CRASHED");
        int sig = WTERMSIG(status);
        if (sig == 11) printf("SIGSEGV\n");
        else if (sig == 6) puts("ABORTED");
                    
        char cmd[0x100];
        sprintf(cmd, "cp %s crashes/crash_%ld", MUTATED_OUTPUT, time(NULL));
        system(cmd);
    }

}

char *pick_random_seed(const char *path) {
        DIR *corpus_folder = opendir(path);
        struct dirent *entry;
    if (corpus_folder == NULL) {
        puts("Unable to read corpus folder");
        exit(1);
    }
    char *files[1000];
    int count = 0;
    while ((entry = readdir(corpus_folder)) != NULL) {
        if (entry->d_name[0] == '.') continue; //|| entry->d_name[1] == "..") continue;
        char *name = strdup(entry->d_name);
        files[count++] = name;
    }

    if (count == 0) return NULL;

    int idx = rand() % count;

    char *seed = malloc(512);
    snprintf(seed, 512, "%s/%s", path, files[idx]);

    for (int i = 0; i < count; i++) {
        free(files[i]);
    }
    closedir(corpus_folder);
    return seed;
}


int main(int argc, char** argv) {
    pthread_t extract_pt;
    pthread_t list_pt;
    pthread_t add_pt;
    pthread_t create_pt;

    
    puts("libarchive fuzzer");
    srand(time(NULL));
    system("mkdir -p out");
    char *seed;
    int i = 0;
    while (1) {
        if (i == 0) seed = pick_random_seed("corpus");
        else if (i == 0x200) i = 0;
        i++;
        FILE *fp = fopen(seed, "rb");
        if (!fp) {
            //
            printf("No seed found");
            break;
        }

        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        rewind(fp);
        unsigned char *buf = malloc(size);
        fread(buf, 1, size, fp);
        fclose(fp);

        mutate(buf, size);

        FILE *out = fopen(MUTATED_OUTPUT, "wb");
        fwrite(buf, 1, size, out);
        fclose(out);
        
        free(buf);

        // implement threads (run using all options)
        // list |  pthread_create(&thread1, NULL, foo, NULL);
        pthread_create(&list_pt, NULL, run_target, (void*)LIST);
        pthread_create(&create_pt, NULL, run_target, (void*)CREATE);
        pthread_create(&extract_pt, NULL, run_target, (void*)EXTRACT);
        pthread_create(&add_pt, NULL, run_target, (void*)ADD);

        pthread_join(list_pt, NULL);
        pthread_join(create_pt, NULL);
        pthread_join(extract_pt, NULL);
        pthread_join(add_pt, NULL);       
    }



    
    return 0;
}