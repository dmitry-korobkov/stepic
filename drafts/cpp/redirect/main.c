#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char** argv) {
    
    while(1) {

        /* stdout */
        printf("stdout\n");
        fflush(stdout);

        /* stderr */
        fprintf(stderr, "stderr\n");
        fflush(stderr);

        sleep(1);
    }

    return 0;
}

