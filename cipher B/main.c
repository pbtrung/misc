#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sodium.h>
#include <gcrypt.h>

int main(int argc, char *argv[]) {
    if (sodium_init() == -1) {
        printf("Unable to initialize libsodium\n");
        exit(-1);
    }

    /* Allocate a pool of 16k secure memory.  This make the secure memory
    available and also drops privileges where needed.  */
    gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);
    /* It is now okay to let libgcrypt complain when there was/is
    a problem with the secure memory. */
    gcry_control(GCRYCTL_RESUME_SECMEM_WARN);
    /* Tell libgcrypt that initialization has completed. */
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

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

        size_t kiv_len = crypto_stream_xchacha20_KEYBYTES + crypto_stream_xchacha20_NONCEBYTES;
        unsigned char *kiv = malloc(kiv_len);
        if (kiv == NULL) {
            printf("Memory couldn't be allocated\n");
            exit(-1);
        }
        gcry_error_t err = gcry_kdf_derive(argv[2], pwd_len, GCRY_KDF_PBKDF2, GCRY_MD_SHA3_512, argv[4], salt_len, 42, kiv_len, kiv);
        if (err) {
            printf("Error: gcry_kdf_derive\n");
            exit(-1);
        }
        int rc = crypto_stream_xchacha20_xor(buf, buf, fsize, &kiv[crypto_stream_xchacha20_KEYBYTES], kiv);
        if (rc != 0) {
            printf("Error: crypto_stream_xchacha20_xor\n");
            exit(-1);
        }

        if (fwrite(buf, fsize, 1, stdout) < 1) {
            printf("Unable to write to stdout\n");
            exit(-1);
        }
        free(buf);
        free(kiv);

    } else if (argc == 5 && strcmp(argv[1], "-h") == 0 && strcmp(argv[3], "-s") == 0) {
        size_t hkey_len = strlen(argv[2]);
        size_t salt_len = strlen(argv[4]);
        assert(hkey_len >= 50);
        assert(salt_len >= 50);

        size_t hk_len = 64;
        unsigned char *hk = malloc(hk_len);
        if (hk == NULL) {
            printf("Memory couldn't be allocated\n");
            exit(-1);
        }
        gcry_error_t err = gcry_kdf_derive(argv[2], hkey_len, GCRY_KDF_PBKDF2, GCRY_MD_SHA3_512, argv[4], salt_len, 42, hk_len, hk);
        if (err) {
            printf("Error: gcry_kdf_derive\n");
            exit(-1);
        }
        size_t hex_len = hk_len * 2 + 1;
        char *hex = malloc(hex_len);
        if (hex == NULL) {
            printf("Memory couldn't be allocated\n");
            exit(-1);
        }
        hex = sodium_bin2hex(hex, hex_len, hk, hk_len);
        if (fwrite(hex, hex_len - 1, 1, stdout) < 1) {
            printf("Unable to write to stdout\n");
            exit(-1);
        }
        free(hk);
        free(hex);

    } else {
        printf("Wrong argv\n");
        exit(-1);
    }

    return EXIT_SUCCESS;
}
