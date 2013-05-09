// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/core/util.h>
#include <gearbox/core/Errors.h>

#include <fstream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

// for zlib compress/decompress
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace bios=boost::iostreams;

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

namespace bfs=boost::filesystem;

#include <openssl/bio.h> // BIO_*
#include <openssl/evp.h> // BIO_f_base64
#include <openssl/buffer.h> // BUF_MEM

#include <uuid/uuid.h>

namespace Gearbox {

    static
    std::string
    slurp(std::istream & in) {
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
    
    std::string
    slurp(int fd, bool rewind) {
        fd = dup(fd); // leave original fd alone
        if ( fd < 0 ) gbTHROW( ERR_LIBC("Could not dup fd") );
        
        if (rewind) {
            // ignore error since not all fds are seekable (ex. pipes, sockets, etc)
            lseek(fd, 0, SEEK_SET);
        }
        
        bios::stream<bios::file_descriptor_source> f(fd, boost::iostreams::close_handle); // will close fd
        if ( !f.is_open() ) {
            gbTHROW( ERR_LIBC("Could not open fd") );
        }
        
        return slurp(f);
    }

    std::string
    slurp(const std::string & file) {
        std::ifstream f(file.c_str());
        if( f.fail() ) {
            gbTHROW( ERR_LIBC("Could not open " + file) );
        }
        return slurp(f);
    }
    
    void
    write_file(const bfs::path & path, const std::string & contents) {
        bfs::ofstream out(path, std::ios_base::out | std::ios_base::trunc);
        if ( !out.is_open() ) {
            gbTHROW( ERR_LIBC("Could not open " + path.string()) );
        }
        out << contents;
        out.close();
        
        if ( out.fail() ) {
            gbTHROW( ERR_LIBC("Failed to close " + path.string()) );
        }
    }

    void urandb64(std::string & data, int bytes) {
        char * rand_data = new char[bytes];
        FILE *f = fopen("/dev/urandom", "r");
        fread(rand_data, sizeof(char), bytes, f);
        fclose(f);
        BIO * b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO * mem = BIO_new(BIO_s_mem());
        mem = BIO_push(b64, mem);
        BIO_write(mem, rand_data, bytes);
        delete [] rand_data;
        (void)BIO_flush(mem);
        BUF_MEM *bptr;
        BIO_get_mem_ptr(mem, &bptr);
        data.assign(bptr->data, bptr->length);
        BIO_free_all(mem);
        return;
    }

    uint32_t urand() {
        uint32_t rand_data = 0;
        FILE *f = fopen("/dev/urandom", "r");
        fread((char*)&rand_data, sizeof(uint32_t), 1, f);
        fclose(f);

        return rand_data;
    }

    namespace {
        // a conservative list of characters that stand for themselves in a naked word
        const std::string NEUTRALS = "+,-./0123456789=ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
        
        // single-quote replacement sequence (see the comment below)
        const std::string SQUOTE_REPLACEMENT = "'\"'\"'";
        
        const char SQUOTE = '\'';
        const std::string SQUOTE_STR = "'";
        const std::string SPACE_STR = " ";
    } /* end anonymous namespace */
    
    /*
     * - non-empty words that only contain [a-zA-Z0-9_=.+,/-] pass through unchanged
     * - other words are enclosed in single-quotes, and each single-quote in them
     *   is replaced with the following five characters: '"'"'
     *   (i.e. leave squote mode, enter a dquoted squote, re-enter squote mode)
     */
    std::string shellquote_list(const std::vector<std::string> & words)
    {
        std::string res;
        
        for (std::vector<std::string>::const_iterator it = words.begin(); it != words.end(); ++it) {
            const std::string& w = *it;
            
            if (!res.empty()) {
                res.append(SPACE_STR);
            }
            
            if (w.empty () || w.find_first_not_of(NEUTRALS) != std::string::npos) {
                // this word need single-quoting
                size_t pos = w.find(SQUOTE);
                if (pos != std::string::npos) {
                    // contains a squote, replace it
                    std::string w2(w, 0, pos);
                    w2.append(SQUOTE_REPLACEMENT);
                    // as well as all the other squotes that it might contain
                    size_t pos2 = w.find(SQUOTE, pos+1);
                    while (pos2 != std::string::npos) {
                        w2.append(w, pos+1, pos2-(pos+1));
                        w2.append(SQUOTE_REPLACEMENT);
                        pos = pos2;
                        pos2 = w.find(SQUOTE, pos+1);
                    }
                    w2.append(w, pos+1, std::string::npos);
                    
                    // and put squotes around it
                    res.append(SQUOTE_STR);
                    res.append(w2);
                    res.append(SQUOTE_STR);
                }
                else {
                    // squote the word
                    res.append(SQUOTE_STR);
                    res.append(w);
                    res.append(SQUOTE_STR);
                }
            }
            else {
                // no squoting needed
                res.append(w);
            }
        }
        
        return res;
    }

    // Doug Crockford's Base32:
    // http://www.crockford.com/wrmg/base32.html
    int
    encode_b32c(uint64_t n, std::string & s)
    {
        // b32c alphabet excludes alphabetic characters that
        // are easily confused with one another:
        // i, l, o, u
        char basechars[] ="0123456789abcdefghjkmnpqrstvwxyz";
        while ( n ) {
            uint64_t q = ( n >> 5 );      // divide by 32
            if ( q != 0 ) {
                s += basechars[ n % 32 ]; // index is the remainder
                n = q;                    // set n to quotient
            }
            else {                        // n < 32 -- last digit
                s += basechars[ n ];      // index is n
                break;                    // done
            }
        }
        std::reverse(s.begin(), s.end()); // rfc4648 -- most significant
        // bit comes first
        return 0;
    }

    int
    decode_b32c( const std::string & s, uint64_t & t )
    {
        t = 0;
        uint64_t o = 1;
        std::string::const_reverse_iterator itr = s.rbegin();
        // iterate through the encoded string starting with
        // the least signficant base32 digit.
        for ( ; itr != s.rend(); itr++ ) {
            int b = *itr;
            // return 1 if s contains illegal characters
            if ( b < '0'  || (b > '9' && b < 'a' ) ||
                 b > 'z'  || b == 'i' || b == 'l'  ||
                 b == 'o' || b == 'u' ) {
                return 1;
            }
            // otherwise compute the base value foreach
            // base32 digit
            else if ( b <= '9' ) { b -= '0'; } // 0-9
            else if ( b <= 'h' ) { b -= 87;  } // a-h
            else if ( b <= 'k' ) { b -= 88;  } // j,k
            else if ( b <= 'n' ) { b -= 89;  } // m,n
            else if ( b <= 't' ) { b -= 90;  } // p-t
            else if ( b <= 'z' ) { b -= 91;  } // v-z
            // multiply base value * positional offset and
            // add to the total
            t += (b * o);
            // raise offset for next digit position * 32
            o *= 32;
        }
        return 0;
    }

    void
    uuid_b32c( std::string & b32u, bool force_v1 )
    {
        // the result of uuid_generate{_time} (3) as a URI and DNS safe string
        union {
            uint64_t data[2];
            uuid_t  uuid;
        } u;
        if ( force_v1 ) {
            // force algorithm to use 'v1' style uuid with mac, timestamp
            uuid_generate_time(u.uuid);
        }
        else {
            // use random data unless insufficient entropy, fallback to
            // mac, timestamp style uuid
            uuid_generate(u.uuid);
        }
        encode_b32c(u.data[0], b32u);
        encode_b32c(u.data[1], b32u);
    }

    void zlib_compress(const std::string & input, std::string & output) {
        boost::iostreams::filtering_streambuf<boost::iostreams::output> fsb;
        fsb.push(boost::iostreams::zlib_compressor());
        fsb.push(boost::iostreams::back_inserter(output));
        boost::iostreams::copy(boost::make_iterator_range(input), fsb);
    }

    void zlib_decompress(const std::string & input, std::string & output) {
        boost::iostreams::filtering_streambuf<boost::iostreams::input> fsb;
        fsb.push(boost::iostreams::zlib_decompressor());
        fsb.push(boost::make_iterator_range(input));
        boost::iostreams::copy(fsb, boost::iostreams::back_inserter(output));
    }
}
