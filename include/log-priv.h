#ifndef LOG_XZY_PRIV_H_
#define LOG_XZY_PRIV_H_

#include <stdio.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>


struct log_control {
    int pid;
    int debuglevel;
    char prog_name[NAME_MAX];
    char shm_name[NAME_MAX];
    char ident[NAME_MAX];
    char target[NAME_MAX];
    char redirect[NAME_MAX];

    /***********************************************
     * do not touch these things in shared memory
     * *********************************************/
    FILE * target_fp;
    pthread_mutex_t mutex;
};

#ifdef __cplusplus
extern "C" {
#endif

struct log_control * log_control_get(const char * name);

void log_control_put(struct log_control * lc);

void log_control_redirect(struct log_control * lc, const char * target);

void log_control_set_debuglevel(struct log_control * lc, int debuglevel);

void log_control_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
