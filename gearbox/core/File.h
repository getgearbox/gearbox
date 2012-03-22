#ifndef GEARBOX_CORE_FILE_H
#define GEARBOX_CORE_FILE_H

#include <boost/filesystem/path.hpp>
#include <sys/types.h>
#include <fcntl.h>

namespace Gearbox {
    class File {
    public:
        File();
        File(const boost::filesystem::path & pathname, int flags = O_RDWR|O_CREAT|O_TRUNC, mode_t mode = 0600);
        File(int fd);
        ~File();
        int fd();
        void open(const boost::filesystem::path & pathname, int flags = O_RDWR|O_CREAT|O_TRUNC, mode_t mode = 0600);
        void close();
    protected:
        int fd_;
    };
}

#endif
