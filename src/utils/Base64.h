#pragma once
#include "BaseClass.h"

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string>
#include <vector>
#include <openssl/buffer.h>

class Base64 final : public api::v1::BaseClass {
public:
    static std::string Encode(const std::string &input) {
        BUF_MEM *bufferPtr;

        BIO *b64 = BIO_new(BIO_f_base64());
        BIO *bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);

        // Disable newline
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

        BIO_write(bio, input.c_str(), static_cast<int>(input.length()));
        BIO_flush(bio);

        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string encoded(bufferPtr->data, bufferPtr->length);

        BIO_free_all(bio);

        return encoded;
    }

    static std::string Decode(const std::string &input) {
        std::vector<char> buffer(input.size());

        BIO *b64 = BIO_new(BIO_f_base64());
        BIO *bio = BIO_new_mem_buf(input.c_str(), static_cast<int>(input.length()));
        bio = BIO_push(b64, bio);

        // Disable newline
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

        const int decoded_length = BIO_read(bio, buffer.data(), static_cast<int>(input.length()));

        BIO_free_all(bio);

        if(decoded_length <= 0) {
            return "";
        }

        return std::string(buffer.data(), decoded_length);
    }
};
