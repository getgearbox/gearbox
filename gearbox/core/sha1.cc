// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <openssl/evp.h>

#include <fstream>
#include <string>

#include <boost/filesystem/path.hpp>

#include "gearbox/core/Errors.h"

using std::ifstream;
using std::streamsize;
using std::string;

namespace Gearbox {

std::string
sha1sum(const boost::filesystem::path &filename)
{
    ifstream ifs(filename.string().c_str());
    if (ifs.fail())
        gbTHROW( ERR_LIBC("Failed to open " + filename.string()) );

    OpenSSL_add_all_digests();

    const char digest_type[] = "SHA1";
    const EVP_MD *md = EVP_get_digestbyname(digest_type);
    if (! md)
        gbTHROW( ERR_INTERNAL_SERVER_ERROR(string("Unknown message digest type: ") + digest_type) );

#define EVP_CALL(func, ...)                                     \
    if (func(__VA_ARGS__) != 1)                                 \
        throw ERR_INTERNAL_SERVER_ERROR(string(#func) + " failed")

    class EvpMdCtxPtr {
    public:
        EvpMdCtxPtr() { EVP_MD_CTX_init(&mdctx_); }  // returns void
        ~EvpMdCtxPtr() { EVP_MD_CTX_cleanup(&mdctx_); }
        operator EVP_MD_CTX *() { return &mdctx_; }
    private:
        EVP_MD_CTX mdctx_;
    } mdctx;

    const streamsize blocksize = 512 * 1024;
    char buf[blocksize];
    EVP_CALL(EVP_DigestInit_ex, mdctx, md, NULL);

    while (! ifs.eof()) {
        ifs.read(buf, blocksize);
        if (ifs.bad())
            gbTHROW( ERR_LIBC("Error reading from " + filename.string()) );

        EVP_CALL(EVP_DigestUpdate, mdctx, buf, ifs.gcount());
    }

    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    EVP_CALL(EVP_DigestFinal_ex, mdctx, md_value, &md_len);
#undef EVP_CALL

    string hexstring;
    static const char lut[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'a', 'b', 'c', 'd', 'e', 'f' };
    for (unsigned int i = 0; i < md_len; ++i) {
        hexstring += lut[md_value[i] >> 4];
        hexstring += lut[md_value[i] & 0x0F];
    }

    return hexstring;
}

}  // namespace Gearbox
