#include "util/crash.hpp"

#include <signal.h>
#include <unistd.h>

#include "util/log.hpp"
#include "util/thread.hpp"

namespace playtrack::crash {
namespace {
void OnFatalSignal(int sig) {
    char line[] = "fatal signal 00\n";
    line[13] = static_cast<char>('0' + (sig / 10) % 10);
    line[14] = static_cast<char>('0' + sig % 10);
    int fd = log::RawFd();
    if (fd >= 0) {
        (void)!write(fd, line, sizeof(line) - 1);
        fsync(fd);
    }
    (void)!write(STDOUT_FILENO, line, sizeof(line) - 1);
    signal(sig, SIG_DFL);
    kill(getpid(), sig);
}
}

void InstallHandlers() {
    InstallSignalStack();
    struct sigaction sa {};
    sa.sa_handler = OnFatalSignal;
    sa.sa_flags = SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    for (int sig : {SIGINT, SIGTERM, SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT, SIGSYS, SIGTRAP}) {
        sigaction(sig, &sa, nullptr);
    }
}

}
