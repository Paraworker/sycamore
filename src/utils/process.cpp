#include "sycamore/utils/process.h"

#include <spdlog/spdlog.h>
#include <sys/wait.h>

namespace sycamore
{

void spawn(const char* command)
{
    int socket[2];
    if (pipe(socket) != 0)
    {
        spdlog::error("Create pipe for fork failed");
        return;
    }

    pid_t child, grandchild;
    child = fork();
    if (child < 0)
    {
        close(socket[0]);
        close(socket[1]);
        spdlog::error("First fork failed");
        return;
    }

    if (child == 0)
    {
        // run in child
        sigset_t set;
        sigemptyset(&set);
        sigprocmask(SIG_SETMASK, &set, nullptr);

        grandchild = fork();
        if (grandchild < 0)
        {
            close(socket[0]);
            close(socket[1]);
            spdlog::error("Second fork failed");
            // exit child
            _exit(0);
        }

        if (grandchild == 0)
        {
            // run in grandchild
            close(socket[0]);
            close(socket[1]);
            execl("/bin/sh", "/bin/sh", "-c", command, nullptr);
            // exit grandchild
            _exit(0);
        }

        close(socket[0]);
        write(socket[1], &grandchild, sizeof(grandchild));
        close(socket[1]);
        // exit child
        _exit(0);
    }

    // run in parent
    close(socket[1]);
    read(socket[0], &grandchild, sizeof(grandchild));
    close(socket[0]);

    // clear child and leave child to init
    waitpid(child, nullptr, 0);
}

}