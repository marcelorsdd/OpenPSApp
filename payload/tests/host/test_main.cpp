#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "fixture.hpp"
#include "test.hpp"

int main(int argc, char **argv) {
    std::string err;
    if (argc == 3 && std::strcmp(argv[1], "--make-fixture") == 0) {
        if (!fixture::Build(argv[2], &err)) {
            std::fprintf(stderr, "fixture: %s\n", err.c_str());
            return 1;
        }
        std::printf("fixture written to %s\n", argv[2]);
        return 0;
    }

    char tmpl[] = "/tmp/openpsapp-test-XXXXXX";
    const char *root = mkdtemp(tmpl);
    if (!root || !fixture::Build(root, &err)) {
        std::fprintf(stderr, "fixture: %s\n", err.c_str());
        return 1;
    }
    setenv("OPENPSAPP_ROOT", root, 1);

    int failed_cases = 0;
    for (const test::Case &c : test::Registry()) {
        int before = test::Failures();
        c.fn();
        bool ok = test::Failures() == before;
        if (!ok) ++failed_cases;
        std::printf("%s %s\n", ok ? "ok  " : "FAIL", c.name);
    }
    std::printf("\n%zu tests, %d failed\n", test::Registry().size(), failed_cases);

    std::string cleanup = std::string("rm -rf '") + root + "'";
    std::system(cleanup.c_str());
    return failed_cases == 0 ? 0 : 1;
}
