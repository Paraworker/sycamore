#include "sycamore/utils/process.h"
#include "sycamore/Core.h"

#include <fmt/core.h>
#include <getopt.h>

static constexpr auto usage
{
    "Usage: sycamore [options] [command]\n"
    "\n"
    "  -h, --help             Show this help message.\n"
    "  -s, --startup_cmd      Startup with command.\n"
    "\n"
};

static constexpr option longOptions[]
{
    {"help", no_argument, nullptr, 'h'},
    {"startup_cmd", required_argument, nullptr, 's'},
    {nullptr, 0, nullptr, 0},
};

int main(int argc, char **argv)
{
    const char* command{};

    int c, i;
    while ((c = getopt_long(argc, argv, "s:h", longOptions, &i)) != -1)
    {
        switch (c)
        {
            case 'h':
                fmt::print(usage);
                return EXIT_SUCCESS;
            case 's':
                command = optarg;
                break;
            default:
                fmt::print(usage);
                return EXIT_SUCCESS;
        }
    }

    if (optind < argc)
    {
        fmt::print(usage);
        return EXIT_SUCCESS;
    }

    setenv("XDG_BACKEND", "wayland", true);
    setenv("XDG_CURRENT_DESKTOP", "Sycamore", true);

    sycamore::core.start();

    if (command)
    {
        sycamore::spawn(command);
    }

    sycamore::core.run();

    return EXIT_SUCCESS;
}