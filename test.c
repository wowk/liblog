#include <log.h>
#include <unistd.h>


int main(int argc, char * argv[])
{
    log_init_with_ident("ident", LOG_WARNING);

    while(1) {
        log_warn("hello, warning message\n");        
        log_info("hello, info message\n");       
        sleep(1);
    }

    return 0;
}
