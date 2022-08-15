#define SYSLOG_NAMES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <ftw.h>
#include <libgen.h>
#include <log.h>
#include <log-priv.h>


//add by vane to fix issue of level-show-------
typedef struct _codex {
    char *c_name;
    int c_val;
}CODEX;

static CODEX priority_names[] =
{
    { "emerg", LOG_EMERG },
    { "alert", LOG_ALERT },
    { "crit", LOG_CRIT },
    { "error", LOG_ERR }, 
    { "warn", LOG_WARNING }, 
    { "notice", LOG_NOTICE },
    { "info", LOG_INFO },
    { "debug", LOG_DEBUG },
    { "none", 8 }, 
    { NULL, -1 }
};
//---------------------------------------------


static int pid = -1;
static int debuglevel = -1;
static const char * name;
static const char * target; 

void usage(void) {
    printf("usage:\n");
    printf("\t-l               : list all log controller\n");
    printf("\t-n <name>        : log controller name\n");
    printf("\t-p <pid>         : log controller pid\n");
    printf("\t-c <debuglevel>  : log controller debuglevel\n");
    printf("\t-t <target>      : log controller target\n");
    printf("\t-h               : show help message\n");
}

static int list_log_controller_cb(const char * fpath, 
        const struct stat * sb, int nfd) 
{
    char path[512];
    const char * shm_name;
    struct log_control * lc;

    strncpy(path, fpath, sizeof(path));
    shm_name = basename(path);
    
    if(strncmp(shm_name, "logctrl-", 8))
        return 0;
    
    lc = log_control_get(shm_name);
    if(!lc) {
        return 0;
    }

    fprintf(stdout, "%-10d %-10s %d(%s)\n", lc->pid, lc->prog_name, 
            lc->debuglevel, priority_names[lc->debuglevel].c_name);
    
    log_control_put(lc);

    return 0;
}

static int redirect_log_controller_target_cb(const char * fpath,
        const struct stat * sb, int nfd) 
{
    char path[512];
    const char * shm_name;
    struct log_control * lc;

    strncpy(path, fpath, sizeof(path));
    shm_name = basename(path);

    if(strncmp(shm_name, "logctrl-", 8))
        return 0;
    
    lc = log_control_get(shm_name);
    if(!lc) {
        return 0;
    }
    
    if(!name || !strcmp(name, lc->prog_name)) {
        if(pid <= 0 || pid == lc->pid) {
            log_control_redirect(lc, target);
        }    
    }
    
    log_control_put(lc);

    return 0;
}

static int set_log_controller_debuglevel_cb(const char * fpath,
        const struct stat * sb, int nfd)
{
    char path[512];
    const char * shm_name;
    struct log_control * lc;

    strncpy(path, fpath, sizeof(path));
    shm_name = basename(path);

    if(strncmp(shm_name, "logctrl-", 8))
        return 0;
    
    lc = log_control_get(shm_name);
    if(!lc) {
        return 0;
    }

    if(!name || !strcmp(name, lc->prog_name)) {
        if(pid <= 0 || lc->pid == pid) {
            log_control_set_debuglevel(lc, debuglevel);
        }
    }
        
    log_control_put(lc);

    return 0;
}

int main(int argc, char * argv[]) {
    int op;
    int help = 0;
    int list = 0;

    log_control_cleanup();

    while(-1 != (op = getopt(argc, argv, "ln:c:t:p:h"))) {
        switch(op) {
            case 'l':
                list = 1;
                break;

            case 'n':
                name = optarg;
                break;

            case 'p':
                pid = atoi(optarg);
                break;

            case 'c':
                debuglevel = atoi(optarg);
                break;

            case 't':
                target = optarg;
                break;

            case 'h':
            case '?':
            default:
                help = 1;
                break;
        }
    }
    
    if(help) {
        usage();
    } else if(list) {
        ftw("/dev/shm", list_log_controller_cb, 1);
    } else {
        if(debuglevel >= 0) {
            ftw("/dev/shm", set_log_controller_debuglevel_cb, 1);
        }
        if(target) {
            ftw("/dev/shm", redirect_log_controller_target_cb, 1);
        }
    }

    return 0;
}
