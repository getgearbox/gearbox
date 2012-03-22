#ifndef GEARBOX_CORE_TEMPDIR_H
#define GEARBOX_CORE_TEMPDIR_H

#include <string>

namespace Gearbox {
    class TempDir {
    public:
        TempDir(const std::string & prefix = "/tmp/gearbox");
        ~TempDir();
        const std::string & name() const;
    private:
        std::string name_;
    };
}

#endif
