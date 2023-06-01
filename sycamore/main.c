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

static const struct option longOptions[] = {
        {"help", no_argument, nullptr, 'h'},
        {"startup_cmd", required_argument, nullptr, 's'},
        {0, 0, nullptr, 0}
};

int main(int argc, char **argv) {
    wlr_log_init(WLR_DEBUG, nullptr);

    char *startupCmd = nullptr;

    int c, i;
    while ((c = getopt_long(argc, argv, "s:h", longOptions, &i)) != -1) {
        switch (c) {
            case 'h':
                printf("%s", usage);
                exit(EXIT_SUCCESS);
            case 's':
                startupCmd = optarg;
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

    if (!serverInit()) {
        serverUninit();
        exit(EXIT_FAILURE);
    }

    setenv("WAYLAND_DISPLAY", server.socket, true);
    setenv("XDG_CURRENT_DESKTOP", "Sycamore", true);

    if (!serverStart()) {
        serverUninit();
        exit(EXIT_FAILURE);
    }

    if (startupCmd) {
        if (fork() == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startupCmd, nullptr);
        }
    }

    serverRun();

    serverUninit();
    return EXIT_SUCCESS;
}