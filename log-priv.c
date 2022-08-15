#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#include <syslog.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>

#include <log-priv.h>

struct log_control * log_control_get(const char * shm_name) {
    int fd;
    struct log_control * lc;

    fd = shm_open(shm_name, O_RDWR|O_CREAT, 0777);
    if(fd < 0) {
        fprintf(stderr, "failed to open/create shared memory %s: %m\n", shm_name);
        return NULL;
    }

    ftruncate(fd, sizeof(struct log_control));
    lc = (struct log_control*)mmap(NULL, sizeof(struct log_control), 
            PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    
    close(fd);

    if(!lc) {
        perror("failed to mmap shared memory\n");
        shm_unlink(shm_name);
        return NULL;
    }

    return lc;
}

void log_control_put(struct log_control * lc) {
    if(lc) {
        munmap(lc, sizeof(struct log_control));
    }
}

void log_control_redirect(struct log_control * lc, const char * target) {
    if(lc) {
        strncpy(lc->redirect, target, sizeof(lc->target));
    }
}

void log_control_set_debuglevel(struct log_control * lc, int debuglevel) {
    if(debuglevel < LOG_EMERG) {
        debuglevel = LOG_EMERG;
    } else if(debuglevel > LOG_DEBUG){
        debuglevel = LOG_DEBUG;
    }

    if(lc) {
        lc->debuglevel = debuglevel;
    } else {
        fprintf(stderr, "failed to set loglevel, library is not initialized\n");
    } 
}

static int check_process_running(int pid, const char * prog_name) {
    FILE * fp;
    char path[1024];

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    if(0 > access(path, F_OK)) {
        return 0;
    }

    fp = fopen(path, "r");
    if(!fp) {
        return -errno;
    }

    if(1 != fscanf(fp, "%s", path)) {
        fclose(fp);
        return -errno;
    }
    
    fclose(fp);

    if(!strcmp(prog_name, basename(path))) {
        return 1;
    }

    return 0;
}


static int log_control_cleanup_cb (const char *fpath, const struct stat *sb, int typeflag) {
    char path[512];
    const char * shm_name;
    struct log_control * lc;
    
    strncpy(path, fpath, sizeof(path));
    shm_name = basename(path);

    if(strncmp(shm_name, "logctrl-", 8)) {
        return 0;
    }
    
    lc = log_control_get(shm_name);
    if(!lc) {
        return 0;
    }
    
    if(0 == check_process_running(lc->pid, lc->prog_name)) {
        log_control_put(lc);
        shm_unlink(shm_name);
    } else {
        log_control_put(lc);
    } 
    
    return 0;
}

void log_control_cleanup(void) {
    ftw("/dev/shm/", log_control_cleanup_cb, 1);
}
