#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s key_length\n", argv[0]);
        return 1;
    }

    int keylen = atoi(argv[1]);
    char *fullKey = malloc((keylen + 2) * sizeof(char)); // Include space for newline and null terminator

    srand(time(0));

    for (int i = 0; i < keylen; i++) {
        int randomInt = ((rand() % 27) + 1);
        if (randomInt == 27) {
            fullKey[i] = 32; // space character
        } else {
            fullKey[i] = randomInt + 64; // capital letters
        }
    }

    fullKey[keylen] = '\n'; // Add newline character
    fullKey[keylen + 1] = '\0'; // Null terminate the string

    fprintf(stdout, "%s", fullKey);

    free(fullKey); // Free allocated memory

    return 0;
}