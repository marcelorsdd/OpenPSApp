#include "util/thread.hpp"

#include <pthread.h>
#include <signal.h>

#include <cstdlib>
#include <memory>

namespace openpsapp {
namespace {
constexpr size_t kThreadStackBytes = 256 * 1024;
constexpr size_t kSignalStackBytes = 64 * 1024;

void ReleaseSignalStack() {
    stack_t current {};
    if (sigaltstack(nullptr, &current) != 0 || (current.ss_flags & SS_DISABLE)) return;
    stack_t off {};
    off.ss_flags = SS_DISABLE;
    if (sigaltstack(&off, nullptr) == 0) std::free(current.ss_sp);
}

void *Run(void *arg) {
    std::unique_ptr<std::function<void()>> fn(static_cast<std::function<void()> *>(arg));
    InstallSignalStack();
    (*fn)();
    ReleaseSignalStack();
    return nullptr;
}
}

void InstallSignalStack() {
    stack_t ss {};
    ss.ss_sp = std::malloc(kSignalStackBytes);
    ss.ss_size = kSignalStackBytes;
    if (ss.ss_sp && sigaltstack(&ss, nullptr) != 0) std::free(ss.ss_sp);
}

bool Spawn(std::function<void()> fn) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, kThreadStackBytes);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    auto *heap_fn = new std::function<void()>(std::move(fn));
    pthread_t thread;
    bool ok = pthread_create(&thread, &attr, Run, heap_fn) == 0;
    pthread_attr_destroy(&attr);
    if (!ok) delete heap_fn;
    return ok;
}

}
