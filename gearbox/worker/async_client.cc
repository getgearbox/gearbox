// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <libgearman/gearman.h>
#include <string.h>

int main (void) {
    gearman_client_st client;
    gearman_return_t ret;
    char * workload = "{'name': 'coryb'}";

    gearman_client_create(&client);
    gearman_client_add_server(&client, "localhost", 4730);

    for( int i=0; i<1000; i++ ) {
        gearman_task_st task;
        (void)gearman_client_add_task_background(
            &client, &task, NULL,
            "hello", NULL, workload, strlen(workload),
            &ret);
        gearman_client_run_tasks(&client);
        gearman_task_free(&task);
    }
    gearman_client_free(&client);

    return 0;
}
