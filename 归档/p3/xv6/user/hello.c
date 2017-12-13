#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
    char *p = (char*) ((640 * 1024) - 4096);
    printf(1, "%x\n", p);
    *p = 'a';
    int rv = fork();
    if (rv == 0) {
        printf(1, "child: %c\n", *p);
    } else if (rv > 0) {
        (void) wait();
        printf(1, "parent: %c\n", *p);
    } else {

    }
    exit();
}
