#include "worker.h"
#include "logger.h"

int main(void) {
    do_work();
    log_level = 3;
    do_work();
    return 0;
}