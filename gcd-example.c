#include <dispatch/dispatch.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

void
deferred_code(__unused void *arg)
{

        printf("block_dispatch\n");
        exit(0);
}

int
main(int argc, char *argv[])
{
        dispatch_queue_t q;
        dispatch_time_t t;

        q = dispatch_get_main_queue();
        t = dispatch_time(DISPATCH_TIME_NOW, 5LL * NSEC_PER_SEC);

        dispatch_async_f(q, NULL, deferred_code);

        dispatch_main();
        return (0);
}
