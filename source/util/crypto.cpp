#include "util/crypto.hpp"
#include <switch.h>
#include <string.h>
#include <mbedtls/bignum.h>

void Crypto::calculateMGF1andXOR(unsigned char* data, size_t data_size, const void* source, size_t source_size) {
    unsigned char h_buf[RSA_2048_BYTES] = {0};
    memcpy(h_buf, source, source_size);

    unsigned char mgf1_buf[0x20];
    size_t ofs = 0;
    unsigned int seed = 0;
    while (ofs < data_size) {
        for (unsigned int i = 0; i < sizeof(seed); i++) {
            h_buf[source_size + 3 - i] = (seed >> (8 * i)) & 0xFF;
        }
        sha256CalculateHash(mgf1_buf, h_buf, source_size + 4);
        for (unsigned int i = ofs; i < data_size && i < ofs + 0x20; i++) {
            data[i] ^= mgf1_buf[i - ofs];
        }
        seed++;
        ofs += 0x20;
    }
}

bool Crypto::rsa2048PssVerify(const void *data, size_t len, const unsigned char *signature, const unsigned char *modulus) {
    mbedtls_mpi signature_mpi;
    mbedtls_mpi modulus_mpi;
    mbedtls_mpi e_mpi;
    mbedtls_mpi message_mpi;

    mbedtls_mpi_init(&signature_mpi);
    mbedtls_mpi_init(&modulus_mpi);
    mbedtls_mpi_init(&e_mpi);
    mbedtls_mpi_init(&message_mpi);
    mbedtls_mpi_lset(&message_mpi, RSA_2048_BITS);

    unsigned char m_buf[RSA_2048_BYTES];
    unsigned char h_buf[0x24];
    const unsigned char E[3] = {1, 0, 1};

    mbedtls_mpi_read_binary(&e_mpi, E, 3);
    mbedtls_mpi_read_binary(&signature_mpi, signature, RSA_2048_BYTES);
    mbedtls_mpi_read_binary(&modulus_mpi, modulus, RSA_2048_BYTES);
    mbedtls_mpi_exp_mod(&message_mpi, &signature_mpi, &e_mpi, &modulus_mpi, NULL);

    if (mbedtls_mpi_write_binary(&message_mpi, m_buf, RSA_2048_BYTES) != 0) {
        throw "Failed to export exponentiated RSA message!";
    }

    mbedtls_mpi_free(&signature_mpi);
    mbedtls_mpi_free(&modulus_mpi);
    mbedtls_mpi_free(&e_mpi);
    mbedtls_mpi_free(&message_mpi);

    /* There's no automated PSS verification as far as I can tell. */
    if (m_buf[RSA_2048_BYTES-1] != 0xBC) {
        return false;
    }

    memset(h_buf, 0, 0x24);
    memcpy(h_buf, m_buf + RSA_2048_BYTES - 0x20 - 0x1, 0x20);

    /* Decrypt maskedDB. */
    calculateMGF1andXOR(m_buf, RSA_2048_BYTES - 0x20 - 1, h_buf, 0x20);

    m_buf[0] &= 0x7F; /* Constant lmask for rsa-2048-pss. */

    /* Validate DB. */
    for (unsigned int i = 0; i < RSA_2048_BYTES - 0x20 - 0x20 - 1 - 1; i++) {
        if (m_buf[i] != 0) {
            return false;
        }
    }   
    if (m_buf[RSA_2048_BYTES - 0x20 - 0x20 - 1 - 1] != 1) {
        return false;
    }

    /* Check hash correctness. */
    unsigned char validate_buf[8 + 0x20 + 0x20];
    unsigned char validate_hash[0x20];
    memset(validate_buf, 0, 0x48);

    sha256CalculateHash(&validate_buf[8], data, len);
    memcpy(&validate_buf[0x28], &m_buf[RSA_2048_BYTES - 0x20 - 0x20 - 1], 0x20);
    sha256CalculateHash(validate_hash, validate_buf, 0x48);

    return memcmp(h_buf, validate_hash, 0x20) == 0;
}