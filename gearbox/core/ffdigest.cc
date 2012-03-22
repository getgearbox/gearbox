// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

/*
 *   ffdigest -- fast file digests
 *   
 *   Generating md5 checksums of large (4 GiB +) files introduces significant time to image
 *   deployment, the bottleneck being the rate at which the contents of the file can be read
 *   from the filesytem and into the digest function. The ffdigest digest_md5() function will 
 *   generate normal (full)  md5 signatures for small (< 64MiB) files and for larger files will
 *   sample 32KiB of data from the beginning, middle, and end of the file.
 *   
*/
#include <gearbox/core/util.h>

#include <openssl/evp.h>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
namespace bfs=boost::filesystem;

#define MIN_SAMPLING_DIGEST_SIZE 64 * (1024 * 1024)
#define DIGEST_SAMPLE_SIZE 32768

namespace Gearbox {

static void
full_digest(std::ifstream & in, uint64_t & /* size */, EVP_MD_CTX & mdctx, const EVP_MD * md)
{
    int block_size = EVP_MD_block_size(md);
    char *buf = new char[block_size + 1];
    while (in.good()) {
        in.read(buf, block_size);
        EVP_DigestUpdate(&mdctx, buf, in.gcount());
    }
    delete [] buf;
}

static void
sampling_digest(std::ifstream & in, uint64_t & size, EVP_MD_CTX & mdctx, const EVP_MD * /* md */)
{
    char *buf = new char[DIGEST_SAMPLE_SIZE + 1];
    
    // first sample
    in.read(buf, DIGEST_SAMPLE_SIZE);
    EVP_DigestUpdate(&mdctx, buf, in.gcount());

    // second sample
    in.seekg((size / 2 - DIGEST_SAMPLE_SIZE / 2));
    in.read(buf, DIGEST_SAMPLE_SIZE);
    EVP_DigestUpdate(&mdctx, buf, in.gcount());
    
    // third sample
    in.seekg(size - DIGEST_SAMPLE_SIZE);
    in.read(buf, DIGEST_SAMPLE_SIZE);
    EVP_DigestUpdate(&mdctx, buf, in.gcount());
    delete [] buf;
}

static std::string
digest_md5(const bfs::path & file, int fast)
{
    std::ostringstream digest;
    
    if (!exists(file)) return digest.str();
    if (is_directory(file)) return digest.str();

    uint64_t size = bfs::file_size(file);
    
    std::ifstream in(file.string().c_str(),
                     std::ios_base::in | std::ios_base::binary);
    if (in.bad()) return digest.str();

    EVP_MD_CTX mdctx;
    const EVP_MD *md = EVP_md5();
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    EVP_DigestInit(&mdctx, md);

    if (!fast || size < MIN_SAMPLING_DIGEST_SIZE) {
        full_digest(in, size, mdctx, md);
    }
    else {
        sampling_digest(in, size, mdctx, md);
    }
    EVP_DigestFinal(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);
    in.close();
    
    for (unsigned int i = 0; i < md_len; i++) {
        digest <<
            std::setw(2) <<
            std::setfill('0') <<
            std::hex <<
            (int) md_value[i];
    }
    return std::string(digest.str());
}

std::string digest_full(const bfs::path & file) {
    return digest_md5(file, 0);
}

std::string digest_fast(const bfs::path & file) {
    return digest_md5(file, 1);
}

} // namespace
