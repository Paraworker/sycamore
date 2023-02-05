#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include "sycamore/server.h"

static const char usage[] =
        "Usage: sycamore [options] [command]\n"
        "\n"
        "  -h, --help             Show this help message.\n"
        "  -s, --startup_cmd      Startup with command.\n"
        "\n";

static const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"startup_cmd", required_argument, NULL, 's'},
        {0, 0, NULL, 0}
};

int main(int argc, char **argv) {
    wlr_log_init(WLR_DEBUG, NULL);

    char *startup_cmd = NULL;

    int c, i;
    while ((c = getopt_long(argc, argv, "s:h", long_options, &i)) != -1) {
        switch (c) {
            case 'h':
                printf("%s", usage);
                exit(EXIT_SUCCESS);
            case 's':
                startup_cmd = optarg;
                break;
            default:
                printf("%s", usage);
                return EXIT_SUCCESS;
        }
    }

    if (optind < argc) {
        printf("%s", usage);
        return EXIT_SUCCESS;
    }

    if (!server_init()) {
        server_fini();
        exit(EXIT_FAILURE);
    }

    setenv("WAYLAND_DISPLAY", server.socket, true);

    if (!server_start()) {
        server_fini();
        exit(EXIT_FAILURE);
    }

    if (startup_cmd) {
        if (fork() == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
        }
    }

    server_run();

    server_fini();
    return EXIT_SUCCESS;
}