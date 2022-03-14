#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <wlr/util/log.h>

#include "sycamore/server.h"

int main(int argc, char **argv) {
    wlr_log_init(WLR_DEBUG, NULL);

    char *startup_cmd = NULL;
    int c;
    while ((c = getopt(argc, argv, "s:h")) != -1) {
        switch (c) {
            case 's':
                startup_cmd = optarg;
                break;
            default:
                printf("Usage: %s [-s startup command]\n", argv[0]);
                return EXIT_SUCCESS;
        }
    }
    if (optind < argc) {
        printf("Usage: %s [-s startup command]\n", argv[0]);
        return EXIT_SUCCESS;
    }

    struct sycamore_server *server = server_create();
    if (!server) {
        exit(EXIT_FAILURE);
    }

    setenv("WAYLAND_DISPLAY", server->socket, true);

    if (startup_cmd) {
        if (fork() == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
        }
    }

    if (!server_start(server)) {
        server_destroy(server);
        exit(EXIT_FAILURE);
    }

    server_run(server);
    server_destroy(server);
    return EXIT_SUCCESS;
}
