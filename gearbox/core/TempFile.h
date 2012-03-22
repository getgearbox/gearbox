#ifndef GEARBOX_CORE_TEMPFILE_H
#define GEARBOX_CORE_TEMPFILE_H

#include <gearbox/core/File.h>
#include <string>

namespace Gearbox {
    class TempFile : public File {
        typedef File super;
    public:
        TempFile(const std::string & prefix = "/tmp/gearbox");
        ~TempFile();
        
        void release();
        void unlink() const;
        const std::string & name() const;

    private:
        std::string name_;
    };
}

#endif
