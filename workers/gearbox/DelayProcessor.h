#ifndef DELAY_PROCESSOR_H
#define DELAY_PROCESSOR_H

#include <gearbox/job/JobManager.h>

namespace Gearbox {
class DelayProcessor {
public:
    DelayProcessor(const std::string & config);
    ~DelayProcessor();

    void run_delayed_jobs();
    void wait_next_job();

    void initfifo();
    
    ConfigFile cfg;
    Gearbox::JobManager jm;
    std::string fifo;
    int fifo_rfd;
    int fifo_wfd;
};
} // namespace
#endif
