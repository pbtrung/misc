#include <assert.h>
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (sodium_init() == -1) {
        printf("Unable to initialize libsodium\n");
        exit(-1);
    }

    if (argc == 7 && strcmp(argv[1], "-k") == 0 && strcmp(argv[3], "-n") == 0 &&
        strcmp(argv[5], "-i") == 0) {
        assert(strlen(argv[2]) == crypto_stream_xchacha20_KEYBYTES);
        assert(strlen(argv[4]) == crypto_stream_xchacha20_NONCEBYTES);

        FILE *f = fopen(argv[6], "rb");
        if (f == NULL) {
            printf("File couldn't be opened\n");
            exit(-1);
        }
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        unsigned char *buf = malloc(fsize);
        if (buf == NULL) {
            printf("Memory couldn't be allocated\n");
            exit(-1);
        }
        size_t nread = fread(buf, fsize, 1, f);
        if (nread < fsize && ferror(f)) {
            printf("Unable to read file\n");
            exit(-1);
        }
        fclose(f);
        int rc = crypto_stream_xchacha20_xor(buf, buf, fsize, argv[4], argv[2]);
        if (rc != 0) {
            printf("Error: crypto_stream_xchacha20_xor\n");
            exit(-1);
        }
        if (fwrite(buf, fsize, 1, stdout) < 1) {
            printf("Unable to write to stdout\n");
            exit(-1);
        }
        free(buf);
    } else {
        printf("Wrong argv\n");
        exit(-1);
    }

    return EXIT_SUCCESS;
}
