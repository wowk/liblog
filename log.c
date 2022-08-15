#define SYSLOG_NAMES
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <pthread.h>
#include <log-priv.h>


static struct log_control * logctrl = NULL;

int log_init_with_ident(const char * ident, int debuglevel) {
    int fd;
    char name[64];
    const char * prog;

    if(logctrl) {
        fprintf(stderr, "log library had been initialized\n");
        return -EEXIST;
    }
    
    log_control_cleanup();

    prog = program_invocation_short_name;

    snprintf(name, sizeof(name), "logctrl-%s-%d", prog, getpid());
    logctrl = log_control_get(name);
    if(!logctrl) {
        fprintf(stderr, "failed to open/create/map shared memory\n");
        return -errno;
    }

    pthread_mutex_init(&logctrl->mutex, NULL);

    logctrl->pid = getpid();

    log_control_set_debuglevel(logctrl, debuglevel);
    log_control_redirect(logctrl, "/dev/console");
    
    strncpy(logctrl->shm_name, name, sizeof(logctrl->shm_name));
    strncpy(logctrl->ident, ident, sizeof(logctrl->ident));
    
    snprintf(logctrl->prog_name, sizeof(logctrl->prog_name), "%s", prog);

    return 0; 
}

int log_init(int debuglevel) {
    return log_init_with_ident(program_invocation_short_name, debuglevel);
}
 
void log_set_debuglevel(int debuglevel) {
    pthread_mutex_lock(&logctrl->mutex);
    log_control_set_debuglevel(logctrl, debuglevel);
    pthread_mutex_unlock(&logctrl->mutex);
}

void log_redirect(const char * target) {
    pthread_mutex_lock(&logctrl->mutex);
    log_control_redirect(logctrl, target);
    pthread_mutex_unlock(&logctrl->mutex);
}

static const char * pri_name[] = {
    [LOG_EMERG] = "emerg",
    [LOG_ALERT] = "alert",
    [LOG_CRIT]  = "crit",
    [LOG_ERR]   = "error",
    [LOG_WARNING] = "warn",
    [LOG_NOTICE]  = "notice",
    [LOG_INFO]  = "info",
    [LOG_DEBUG] = "debug",
};

void do_log_print(int debuglevel, const char * file, int line, const char * fmt, ...) {
    va_list vl;
    

    if(debuglevel > logctrl->debuglevel || debuglevel < LOG_EMERG) {
        return;
    }
    
    if(!logctrl) {
        return;
    }

    if(logctrl->redirect[0]) {
        FILE * fp;
        
        pthread_mutex_lock(&logctrl->mutex);

        fp = fopen(logctrl->redirect, "a+");
        if(fp) {
            if(logctrl->target_fp) {
                fclose(logctrl->target_fp);
            }
            logctrl->target_fp = fp;
            strcpy(logctrl->target, logctrl->redirect);
        }
        memset(logctrl->redirect, 0, sizeof(logctrl->redirect));
        
        pthread_mutex_unlock(&logctrl->mutex);
    }

    if(!logctrl->target_fp) {
        return;
    }

    flockfile(logctrl->target_fp);
    
    fprintf(logctrl->target_fp, "[%d] [%s] [%s:%d] [%s] ", 
            logctrl->pid, logctrl->ident, file, line, pri_name[debuglevel]);
    va_start(vl, fmt);
    vfprintf(logctrl->target_fp, fmt, vl);
    va_end(vl);

    fputc('\n', logctrl->target_fp);
    
    fflush(logctrl->target_fp);

    funlockfile(logctrl->target_fp);
}

void __attribute__((destructor)) log_deinit(void) {
    char name[NAME_MAX];

    if(!logctrl) {
        return;
    }
    
    if(logctrl->target_fp) {
        fclose(logctrl->target_fp);
    }

    strncpy(name, logctrl->shm_name, sizeof(name));
    log_control_put(logctrl);

    shm_unlink(name);

    logctrl = NULL;
}

