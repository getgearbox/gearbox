#ifndef GEARBOX_CORE_SYSTEM_PAIR_H
#define GEARBOX_CORE_SYSTEM_PAIR_H

#include <vector>
#include <string>

namespace Gearbox {
    class SystemPair {
    public:
        typedef std::vector<std::string> cmd;
        SystemPair(
            const cmd & on_init,
            const cmd & on_destroy
        );
        ~SystemPair();
    private:
        cmd on_destroy_;
    };
}

#endif
