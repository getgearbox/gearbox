// -*- c++ -*-
#ifndef GEARBOX_CORE_UTIL
#define GEARBOX_CORE_UTIL

#include <vector>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <gearbox/core/Pipe.h>

namespace Gearbox 
{
    std::string digest_full( const boost::filesystem::path & path );
    std::string digest_fast( const boost::filesystem::path & path );
    std::string sha1sum( const boost::filesystem::path & path );
    
    typedef std::vector<std::string> Cmd;
    
    template <typename T>
    Cmd &
    operator<<(Cmd & v, const T & value) {
        v.push_back(boost::lexical_cast<std::string>(value));
        return v;
    }

    pid_t run_bg (const Cmd & cmd, ReadPipe & p);
    pid_t run_bg (const Cmd & cmd, int fd_out, int fd_err);

    int run (const Cmd & cmd, int fd_out = 1 , int fd_err = 2);
    int run (const Cmd & cmd, std::string & output);
    int run (const Cmd & cmd, std::string & stdout, std::string & stderr);
    int run (const std::string & cmd);
    int run (const std::string & cmd, std::string & output);
    int run (const std::string & cmd, std::string & stdout, std::string & stderr);

    std::string shellquote_list(const Cmd & cmd);

    std::string slurp(const std::string & file);
    std::string slurp(int fd, bool rewind = true);

    void write_file(const boost::filesystem::path & path, const std::string & contents);

    void urandb64(std::string & output, int size=32);
    uint32_t urand();

    int encode_b32c(uint64_t n, std::string & s);
    int decode_b32c( const std::string & s, uint64_t & t );
    void uuid_b32c( std::string & b32u, bool force_v1 );

    void zlib_compress  (const std::string & input, std::string & output);
    void zlib_decompress(const std::string & input, std::string & output);

} // namespace Gearbox

#endif // GEARBOX_CORE_UTIL
