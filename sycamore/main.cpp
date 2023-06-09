#include <getopt.h>
#include "sycamore/utils/process.h"
#include "sycamore/Core.h"

static constexpr char usage[] =
        "Usage: sycamore [options] [command]\n"
        "\n"
        "  -h, --help             Show this help message.\n"
        "  -s, --startup_cmd      Startup with command.\n"
        "\n";

static constexpr option longOptions[] = {
        {"help", no_argument, nullptr, 'h'},
        {"startup_cmd", required_argument, nullptr, 's'},
        {nullptr, 0, nullptr, 0}
};

int main(int argc, char **argv) {
    const char* startupCmd = nullptr;

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

    if (!sycamore::Core::instance.init()) {
        exit(EXIT_FAILURE);
    }

    if (!sycamore::Core::instance.start()) {
        sycamore::Core::instance.uninit();
        exit(EXIT_FAILURE);
    }

    if (startupCmd) {
        sycamore::spawn(startupCmd);
    }

    sycamore::Core::instance.run();

    sycamore::Core::instance.uninit();
    return EXIT_SUCCESS;
}