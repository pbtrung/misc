#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pbkdf2.h"
#include "hc256.h"

int main(int argc, char *argv[]) {
    if (argc == 7 && strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-s") == 0 &&
        strcmp(argv[5], "-i") == 0) {

        size_t pwd_len = strlen(argv[2]);
        size_t salt_len = strlen(argv[4]);
        assert(pwd_len >= 50);
        assert(salt_len >= 50);

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

        size_t kiv_len = 64;
        unsigned char *kiv = malloc(kiv_len);
        if (kiv == NULL) {
            printf("Memory couldn't be allocated\n");
            exit(-1);
        }
        pbkdf2(argv[2], pwd_len, argv[4], salt_len, 42, kiv, kiv_len);

        hc_ctx *hc256_ctx = malloc(sizeof(hc_ctx));
        if (hc256_ctx == NULL) {
            printf("Memory couldn't be allocated\n");
            exit(-1);
        }
        hc256_setkey(hc256_ctx, kiv);
        hc256_crypt(hc256_ctx, buf, fsize);

        if (fwrite(buf, fsize, 1, stdout) < 1) {  
            printf("Unable to write to stdout\n");
            exit(-1);
        }
        free(buf);
        free(kiv);
        free(hc256_ctx);
    } else {
        printf("Wrong argv\n");
        exit(-1);
    }

    return EXIT_SUCCESS;
}
