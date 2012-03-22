// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <workers/gearbox/DelayProcessor.h>

#include <gearbox/core/Json.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/strlcpy.h>
#include <gearbox/core/util.h>

#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

#include <boost/scoped_ptr.hpp>

#include <gearbox/store/dbconn.h>
using namespace Gearbox;
using namespace Gearbox::Database;

// for socket
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <signal.h>

DelayProcessor::DelayProcessor(const std::string & config)
    : cfg( config ), 
      jm(cfg),
      fifo_rfd(-1),
      fifo_wfd(-1)
{
    db_init(cfg.get_json("delay"), "delay");

    fifo = cfg.get_string_default("delay", "fifo", "");
    if ( fifo.empty() ) {
        fifo = cfg.get_string_default("gearbox", "root", LOCALSTATEDIR "/gearbox");
        fifo += "/delay.fifo";
    }
}

DelayProcessor::~DelayProcessor() {
    if( fifo_rfd >= 0 ) 
        close(fifo_rfd);
    if( fifo_wfd >= 0 ) 
        close(fifo_wfd);
    if( bfs::exists(fifo) ) {
        bfs::remove(fifo);
    }
}

void DelayProcessor::initfifo() {

    // create socket so worker can notify us when there is new
    // work added
    
    if( bfs::exists(fifo) ) {
        bfs::remove(fifo);
    }
        
    struct sockaddr_un server; 
    
    signal(SIGPIPE,SIG_IGN);
    fifo_rfd = socket(AF_UNIX, SOCK_STREAM, 0); 
    if (fifo_rfd < 0) { 
        _FATAL("failed to create socket " << fifo_rfd << ": " << strerror(errno));
        gbTHROW( ERR_LIBC("failed to create socket") );
    } 
    _DEBUG("fifo: " << fifo);
    server.sun_family = AF_UNIX; 
    strlcpy(server.sun_path, fifo.c_str(), sizeof(server.sun_path)); 

    if (bind(fifo_rfd, (struct sockaddr *) &server, sizeof(struct sockaddr_un))) { 
        _FATAL("failed to bind stream socket: " << strerror(errno));
        gbTHROW( ERR_LIBC("failed to bind stream socket") );
    } 
    listen(fifo_rfd, 1); 
}

void DelayProcessor::run_delayed_jobs() {
    _DEBUG("looking for delayed jobs that are now runnable");
    soci::session & sess = getconn( "delay", true);
    LoggedStatement lst(sess);
    time_t now = time(NULL);
    long id, time;
    std::string name, encoded;

    boost::scoped_ptr<soci::blob> bp;
    lst << "SELECT id, name, envelope, time FROM jobs WHERE time <= :now ORDER BY time ASC";
    lst.use(now).into(id).into(name);
    // soci mysql does not support blobs but std::strings seem to work fine
    if( sess.get_backend_name() == "mysql" ) {
        lst.into(encoded);
    }
    else {
        bp.reset(new soci::blob(sess));
        lst.into(*bp);
    }
    lst.into(time).execute();
    while ( lst.fetch() ) {
        _DEBUG("creating job for " << name);

        if( bp.get() ) {
            encoded.resize(bp->get_len(),'\0');
            bp->read(0,(char*)encoded.data(),bp->get_len());
        }
        
        std::string envelope;
        zlib_decompress(encoded,envelope);
        JobPtr job = jm.job(name, envelope);
        // force it to be async so delayDaemon does
        // not deadlock waiting for sync job to finish
        job->type(Job::JOB_ASYNC);
        JobResponse resp = job->run();
        // check to see if job was queued before deleting
        if( resp.code() < 300 ) {
            _DEBUG("deleting id: " << id);
            dblock("delay");
            LoggedStatement llst(sess);
            llst << "DELETE from jobs where id=:id";
            llst.use(id).execute(true);
            dbcommit("delay");
        }
    }
}

void DelayProcessor::wait_next_job() {
    soci::session & sess = getconn( "delay", true);
    LoggedStatement lst(sess);
    // now select next one and select on socket with [job.time - now] seconds
    time_t when = 0;
    lst << "SELECT time from jobs ORDER BY time ASC LIMIT 1";
    lst.into(when).execute(true);
    
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(this->fifo_rfd, &rfds);

    int retval = 0;
    if( !lst.got_data() ) {
        // nothing in the queue so wait forever
        _DEBUG("delay queue empty, waiting for worker notification");
        retval = select(this->fifo_rfd+1, &rfds, NULL, NULL, NULL);
    }
    else {
        struct timeval timeout;
        time_t now = time(NULL);
        if( when - now <= 0 ) {
            // whoa the timeout is in the past
            // so just return
            return;
        }
        timeout.tv_sec = when - now;
        timeout.tv_usec = 0;
        retval = select(this->fifo_rfd+1, &rfds, NULL, NULL, &timeout);
    }
    // retval == -1 => error
    // retval > 0   => worker wrote to us
    // retval == 0  => timeout
    if( retval == -1 ) {
        _FATAL("select failure: " << strerror(errno));
        gbTHROW( ERR_LIBC("select failed") );
    }
    else if ( retval ) {
        _DEBUG("got notification, flushing fifo");
        char buf[128];
        int conn = accept(fifo_rfd, NULL, NULL);
        if (conn < 0) {
            _FATAL("accept failed: " << strerror(errno));
            gbTHROW( ERR_LIBC("accept failed") );
        }
        while( recv(this->fifo_rfd, buf, 128, 0) > 0 ) {
            continue;
        }
        close(conn);
    }
    else {
        _DEBUG("select timeout, there must be jobs to run");
    }
}
