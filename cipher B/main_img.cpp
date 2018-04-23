#include <assert.h>
#include <string.h>
#include <math.h>

#include <fstream>
#include <iostream>

#include <cryptopp/modes.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha3.h>
#include <cryptopp/threefish.h>
#include <cryptopp/hex.h>

#include <sodium.h>
#include <GraphicsMagick/Magick++.h>

static inline void u64_to_u8(unsigned char out[8U], uint64_t x) {
    out[0] = (unsigned char) (x & 0xff); x >>= 8;
    out[1] = (unsigned char) (x & 0xff); x >>= 8;
    out[2] = (unsigned char) (x & 0xff); x >>= 8;
    out[3] = (unsigned char) (x & 0xff); x >>= 8;
    out[4] = (unsigned char) (x & 0xff); x >>= 8;
    out[5] = (unsigned char) (x & 0xff); x >>= 8;
    out[6] = (unsigned char) (x & 0xff); x >>= 8;
    out[7] = (unsigned char) (x & 0xff);
}

static inline uint64_t u8_to_u64(const unsigned char in[8U]) {
    uint64_t x;

    x  = in[7]; x <<= 8;
    x |= in[6]; x <<= 8;
    x |= in[5]; x <<= 8;
    x |= in[4]; x <<= 8;
    x |= in[3]; x <<= 8;
    x |= in[2]; x <<= 8;
    x |= in[1]; x <<= 8;
    x |= in[0];

    return x;
}

int main(int argc, char *argv[]) {

    if (sodium_init() < 0) {
        std::cerr << "ERROR: sodium_init()" << std::endl;
        exit(-1);
    }

    const unsigned int KEY_LENGTH = 128;
    const unsigned int CTR_LENGTH = 128;
    const unsigned int TWEAK_LENGTH = 16;
    const unsigned int SALT_LENGTH = 64;
    const unsigned int HSALT_LENGTH = 64;
    const unsigned int HMAC_LENGTH = 64;
    const unsigned int FILE_LENGTH = 8;
    const unsigned int HEADER_LENGTH = HMAC_LENGTH + SALT_LENGTH + HSALT_LENGTH;

    if (argc == 10 && strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-h") == 0 && strcmp(argv[5], "-i") == 0 && strcmp(argv[7], "-o") == 0 && strcmp(argv[9], "-e") == 0) {

        size_t pwd_len = strlen(argv[2]);
        assert(pwd_len >= 5);
        size_t hpwd_len = strlen(argv[4]);
        assert(hpwd_len >= 5);  

        try {
            std::ifstream ifs(argv[6], std::ios::binary | std::ios::ate);
            uint64_t file_len = ifs.tellg();
            size_t img_side = ceil(sqrt(file_len + HEADER_LENGTH + FILE_LENGTH));
            std::vector<char> img_body(img_side * img_side);
            ifs.seekg(0, std::ios::beg);
            ifs.read(&img_body[HEADER_LENGTH + FILE_LENGTH], file_len);

            u64_to_u8((unsigned char *)&img_body[HEADER_LENGTH], file_len);

            unsigned char salt[SALT_LENGTH];
            randombytes_buf(salt, SALT_LENGTH);
            std::memcpy(&img_body[HMAC_LENGTH], salt, SALT_LENGTH);
            unsigned char hsalt[HSALT_LENGTH];
            randombytes_buf(hsalt, HSALT_LENGTH);
            std::memcpy(&img_body[HMAC_LENGTH + SALT_LENGTH], hsalt, HSALT_LENGTH);

            size_t buf_len = KEY_LENGTH + CTR_LENGTH + TWEAK_LENGTH;
            CryptoPP::byte buf[buf_len];
            CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA3_512> pbkdf2;
            pbkdf2.DeriveKey(buf, buf_len, 0, (CryptoPP::byte *)argv[2], pwd_len, salt, SALT_LENGTH, 42);

            CryptoPP::ConstByteArrayParameter tweak(&buf[KEY_LENGTH + CTR_LENGTH], TWEAK_LENGTH, false);
            CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(CryptoPP::Name::Tweak(), tweak);
            CryptoPP::Threefish1024::Encryption t3f(buf, KEY_LENGTH);
            t3f.SetTweak(params);
            CryptoPP::CTR_Mode_ExternalCipher::Encryption encryptor(t3f, &buf[KEY_LENGTH]);
            encryptor.ProcessData((CryptoPP::byte *)&img_body[HEADER_LENGTH], (CryptoPP::byte *)&img_body[HEADER_LENGTH], img_body.size() - HEADER_LENGTH);

            CryptoPP::byte hkey[HMAC_LENGTH * 3];
            pbkdf2.DeriveKey(hkey, HMAC_LENGTH * 3, 0, (CryptoPP::byte *)argv[4], hpwd_len, hsalt, HSALT_LENGTH, 42);
            std::memcpy(img_body.data(), &hkey[HMAC_LENGTH * 2], HMAC_LENGTH);
            CryptoPP::HMAC<CryptoPP::SHA3_512> hmac(hkey, HMAC_LENGTH * 2);
            hmac.Update((CryptoPP::byte *)img_body.data(), img_body.size());
            CryptoPP::byte hash[HMAC_LENGTH];
            hmac.Final(hash);
            std::memcpy(img_body.data(), hash, HMAC_LENGTH);

            Magick::InitializeMagick(*argv);
            Magick::Image image;
            image.quiet(false);
            image.read(img_side, img_side, "I", Magick::CharPixel, img_body.data());
            image.magick("GRAY");
            image.depth(8);
            image.write(argv[8]);

        } catch (CryptoPP::Exception const& ex) {
            std::cerr << "CryptoPP::Exception caught: " << ex.what() << std::endl;
            exit(-1);
        } catch (Magick::Exception const& ex) {
            std::cerr << "Magick::Exception caught: " << ex.what() << std::endl;
            exit(-1);
        } catch (std::exception const& ex) {
            std::cerr << "std::exception caught: " << ex.what() << std::endl;
            exit(-1);
        }

    } else if (argc == 10 && strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-h") == 0 && strcmp(argv[5], "-i") == 0 && strcmp(argv[7], "-o") == 0 && strcmp(argv[9], "-d") == 0) {

        size_t pwd_len = strlen(argv[2]);
        assert(pwd_len >= 5);
        size_t hpwd_len = strlen(argv[4]);
        assert(hpwd_len >= 5);

        try {
            Magick::InitializeMagick(*argv);
            Magick::Image image(argv[6]);
            image.quiet(false);
            unsigned int c = image.columns();
            unsigned int r = image.rows();
            std::vector<char> img_body(c * r);
            image.write(0, 0, c, r, "I", Magick::CharPixel, img_body.data());

            unsigned char hash_from_img[HMAC_LENGTH];
            std::memcpy(hash_from_img, img_body.data(), HMAC_LENGTH);
            unsigned char hsalt[HSALT_LENGTH];
            std::memcpy(hsalt, &img_body[HMAC_LENGTH + SALT_LENGTH], HSALT_LENGTH);

            CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA3_512> pbkdf2;
            CryptoPP::byte hkey[HMAC_LENGTH * 3];
            pbkdf2.DeriveKey(hkey, HMAC_LENGTH * 3, 0, (CryptoPP::byte *)argv[4], hpwd_len, hsalt, HSALT_LENGTH, 42);
            std::memcpy(img_body.data(), &hkey[HMAC_LENGTH * 2], HMAC_LENGTH);
            CryptoPP::HMAC<CryptoPP::SHA3_512> hmac(hkey, HMAC_LENGTH * 2);
            hmac.Update((CryptoPP::byte *)img_body.data(), img_body.size());
            CryptoPP::byte hash[HMAC_LENGTH];
            hmac.Final(hash);
            if (std::memcmp(hash, hash_from_img, HMAC_LENGTH) != 0) {
                std::cerr << "ERROR: HMAC" << std::endl;
                std::cerr << "FILE : " << argv[6] << std::endl;
                exit(-1);
            }

            unsigned char salt[SALT_LENGTH];
            std::memcpy(salt, &img_body[HMAC_LENGTH], SALT_LENGTH);

            size_t buf_len = KEY_LENGTH + CTR_LENGTH + TWEAK_LENGTH;
            CryptoPP::byte buf[buf_len];
            pbkdf2.DeriveKey(buf, buf_len, 0, (CryptoPP::byte *)argv[2], pwd_len, salt, SALT_LENGTH, 42);

            CryptoPP::ConstByteArrayParameter tweak(&buf[KEY_LENGTH + CTR_LENGTH], TWEAK_LENGTH, false);
            CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(CryptoPP::Name::Tweak(), tweak);
            CryptoPP::Threefish1024::Encryption t3f(buf, KEY_LENGTH);
            t3f.SetTweak(params);
            CryptoPP::CTR_Mode_ExternalCipher::Encryption encryptor(t3f, &buf[KEY_LENGTH]);
            encryptor.ProcessData((CryptoPP::byte *)&img_body[HEADER_LENGTH], (CryptoPP::byte *)&img_body[HEADER_LENGTH], img_body.size() - HEADER_LENGTH);

            uint64_t file_len = u8_to_u64((unsigned char *)&img_body[HEADER_LENGTH]);
            std::ofstream dec_image;
            dec_image.open(argv[8]);
            std::copy(&img_body[HEADER_LENGTH + FILE_LENGTH], &img_body[HEADER_LENGTH + FILE_LENGTH + file_len], std::ostream_iterator<char>(dec_image, ""));
            dec_image.close();

        } catch (CryptoPP::Exception const& ex) {
            std::cerr << "CryptoPP::Exception caught: " << ex.what() << std::endl;
            exit(-1);
        } catch (Magick::Exception const& ex) {
            std::cerr << "Magick::Exception caught: " << ex.what() << std::endl;
            exit(-1);
        } catch (std::exception const& ex) {
            std::cerr << "std::exception caught: " << ex.what() << std::endl;
            exit(-1);
        }

    } else {
        printf("Wrong argv\n");
        exit(-1);
    }

    return EXIT_SUCCESS;
}
