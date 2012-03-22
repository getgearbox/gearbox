#ifndef GEARBOX_CORE_PIPE_H
#define GEARBOX_CORE_PIPE_H

namespace Gearbox {
    class Pipe {
    public:
        Pipe();
        ~Pipe();
        int fd(int ix);
        void close(int ix);
    private:
        int filedes[2];
    };

    class ReadPipe: public Pipe {
    public:
        typedef Pipe super;
        ReadPipe();
        int fd();
    };
}

#endif
