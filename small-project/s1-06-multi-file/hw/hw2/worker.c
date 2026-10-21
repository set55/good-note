#include "worker.h"
#include "logger.h"

void do_work(void)
{
    log_msg(1, "Work started");
    // Perform some work here
    log_msg(3, "Work detail");
}