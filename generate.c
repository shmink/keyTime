#include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
// #include <time.h>

#include <time.h>
// #include <stdio.h>
#include <stdlib.h>
 
int main(void)
{
    int length;
    char output[2];
    char str[] = "0123456789ABCDEF";
    /* Seed number for rand() */
    srand((unsigned int) time(0) + getpid());
    length = rand() % 15 + 8;
 
    for (int i = 0; i < 2; ++i) {
        output[i] = str[rand() % 16];
        srand(rand());
    }
    // output[9] = '\0';

    int number = (int)strtol(output, NULL, 16);

    printf("%X\n", number);

    // printf("%s\n", output);

    // printf("%d\n", (int)time(0));
 
    //return EXIT_SUCCESS;
}