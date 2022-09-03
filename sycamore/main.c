#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <wlr/util/log.h>
#include "sycamore/server.h"

void load_background() {
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "swaybg -m fill -i /home/ha/Pictures/wallpaper.jpg", (void *)NULL);
    }
}

void load_bar() {
    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "waybar", (void *)NULL);
    }
}

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

    load_background();
    load_bar();

    server_run();

    server_fini();
    return EXIT_SUCCESS;
}