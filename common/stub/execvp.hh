#include <gearbox/core/logger.h>
#include <unistd.h>
#include <string.h>

// this is just an example of how you could create a stub for testing 
// do_execvp
int do_execvp(const char* file, char* const argv[]) {
    if (strcmp(argv[0], "cat") == 0 &&
        strcmp(argv[1], "/etc/shadow") == 0 && 
        argv[2] == NULL) return 0;

    return execvp(file, argv);
}
