// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <tap/trivial.h>
#include <gearbox/core/logger.h>
#include <gearbox/core/JsonSchema.h>
#include <gearbox/core/util.h>
#include <gearbox/store/dbconn.h>
using namespace Gearbox;

#include <signal.h>
#include <limits.h>
#include <sys/wait.h>
#include <libgen.h>
#include <unistd.h>

#include <workers/gearbox/WorkerGearbox.h>
#include <workers/gearbox/DelayProcessor.h>

#include <stub/gearman.hh>

using namespace Gearbox;
using std::string;
using namespace soci;

int main(int argc, char *argv[]) {
    TEST_START(12);

    string basedir = string(dirname(argv[0])) + "/../";
    chdir(basedir.c_str());

    log_init("./unit.conf");
    OK( run("./mkdb") == 0 );
//    db_init("./unit.conf");
    
    WorkerGearbox w("./unit.conf");
    DelayProcessor d("./unit.conf");
    ConfigFile cfg("./unit.conf");
    d.initfifo();
    

    JsonSchema createSchema;
    createSchema.parseFile("../../gearbox/schemas/create-delay-job-v1.js");

    time_t now = time(NULL);
    Json in, out;
    
    Json req;
    req["time"] = now + 2;
    req["name"] = "do_put_doer_thing_v1";
    req["envelope"].parse( slurp("./job.js") );

    w.status_manager().create("s-1234","create","http://host:4080/thing/v1/thing/t-1234");
    
    NOTHROW( req.validate(&createSchema) );

    JobManager jm(cfg);
    JobPtr job = jm.job("do_put_delay_job_v1");
    job->content(req.serialize());

    JobResponse resp;
    
    NOTHROW( w.do_put_delay_job_v1(*job,resp) );

    soci::session & sql = Gearbox::Database::getconn();

    int id, t, ctime;
    string name;
    soci::blob b(sql);
    sql << "SELECT id, name, envelope, time, ctime FROM jobs",
        into(id),
        into(name),
        into(b),
        into(t),
        into(ctime);
    std::string compressed(b.get_len(),'\0');
    b.read(0,(char*)compressed.data(),b.get_len());
    string envelope;
    Gearbox::zlib_decompress(compressed, envelope);
    
    IS( id, 1 );
    IS( name, "do_put_doer_thing_v1" );
    Json input;
    input.parse(slurp("./job.js"));
    IS( envelope, input.serialize() );
    IS( t, now + 2 );
    OK( ctime > 0 );
    
    // this will flush the fifo update
    NOTHROW( d.wait_next_job() );

    // this will wait the 2 seconds
    NOTHROW( d.wait_next_job() );
    OK( time(NULL) >= now + 2 );
    
    // now put the job in the stub gearman queue
    NOTHROW( d.run_delayed_jobs() );

    TEST_END;
}

        

    
