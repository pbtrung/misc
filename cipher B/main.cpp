#include <assert.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

#include <cryptopp/modes.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha3.h>
#include <cryptopp/threefish.h>
#include <cryptopp/hex.h>

#define KEY_LENGTH 128U
#define CTR_LENGTH 128U
#define TWEAK_LENGTH 16U

static std::vector<char> read_file(char const *filename) {
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    std::vector<char> result(pos);

    ifs.seekg(0, std::ios::beg);
    ifs.read(result.data(), pos);

    return result;
}

int main(int argc, char *argv[]) {

    if (argc == 7 && strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-s") == 0 && strcmp(argv[5], "-i") == 0) {

        size_t pwd_len = strlen(argv[2]);
        size_t salt_len = strlen(argv[4]);
        assert(pwd_len >= 50);
        assert(salt_len >= 50);

        try {
            size_t buf_len = KEY_LENGTH + CTR_LENGTH + TWEAK_LENGTH;
            CryptoPP::byte buf[buf_len];
            CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA3_512> pbkdf2;
            pbkdf2.DeriveKey(buf, buf_len, 0, (CryptoPP::byte *)argv[2], pwd_len, (CryptoPP::byte *)argv[4], salt_len, 42);

            std::vector<char> file_content = read_file(argv[6]);

            CryptoPP::ConstByteArrayParameter tweak(&buf[KEY_LENGTH + CTR_LENGTH], TWEAK_LENGTH, false);
            CryptoPP::AlgorithmParameters params = CryptoPP::MakeParameters(CryptoPP::Name::Tweak(), tweak);
            CryptoPP::Threefish1024::Encryption t3f(buf, KEY_LENGTH);
            t3f.SetTweak(params);
            CryptoPP::CTR_Mode_ExternalCipher::Encryption encryptor(t3f, &buf[KEY_LENGTH]);
            encryptor.ProcessData((CryptoPP::byte *)file_content.data(), (CryptoPP::byte *)file_content.data(), file_content.size());
            std::copy(file_content.begin(), file_content.end(), std::ostream_iterator<char>(std::cout, ""));
        } catch (CryptoPP::Exception const& ex) {
            std::cerr << "CryptoPP::Exception caught: " << ex.what() << std::endl;
            exit(-1);
        } catch (std::exception const& ex) {
            std::cerr << "std::exception caught: " << ex.what() << std::endl;
            exit(-1);
        }

    } else if (argc == 5 && strcmp(argv[1], "-h") == 0 && strcmp(argv[3], "-s") == 0) {

        size_t pwd_len = strlen(argv[2]);
        size_t salt_len = strlen(argv[4]);
        assert(pwd_len >= 50);
        assert(salt_len >= 50);

        try {
            size_t buf_len = 64;
            CryptoPP::byte buf[buf_len];
            CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA3_512> pbkdf2;
            pbkdf2.DeriveKey(buf, buf_len, 0, (CryptoPP::byte *)argv[2], pwd_len, (CryptoPP::byte *)argv[4], salt_len, 42);

            std::string encoded;
            CryptoPP::StringSource(buf, buf_len, true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded), false));
            std::cout << encoded;
        } catch (CryptoPP::Exception const& ex) {
            std::cerr << "CryptoPP::Exception caught: " << ex.what() << std::endl;
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
