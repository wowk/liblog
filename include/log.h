#ifndef LOG_XZY_H_
#define LOG_XZY_H_

#include <stdio.h>
#include <errno.h>
#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

int log_init(int debuglevel);
int log_init_with_ident(const char * ident, int debuglevel);

void log_set_debuglevel(int debuglevel);

void log_redirect(const char * target);

void do_log_print(int debuglevel, const char * file, int line, const char * fmt, ...);

#define log_print(debuglevel, fmt, ...) \
    do_log_print(debuglevel, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define log_emerg(fmt, ...) log_print(LOG_EMERG, fmt, ##__VA_ARGS__)
#define log_alert(fmt, ...) log_print(LOG_ALERT, fmt, ##__VA_ARGS__)
#define log_crit(fmt, ...)  log_print(LOG_CRIT, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_print(LOG_ERR, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  log_print(LOG_WARNING, fmt, ##__VA_ARGS__)
#define log_notice(fmt, ...) log_print(LOG_NOTICE, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_print(LOG_INFO, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) log_print(LOG_DEBUG, fmt, ##__VA_ARGS__)

void do_log_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
