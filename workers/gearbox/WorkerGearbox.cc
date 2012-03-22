// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <workers/gearbox/WorkerGearbox.h>

#define LOGCAT "gearbox.worker.gearbox"
#include <gearbox/core/logger.h>

#include <gearbox/store/dbconn.h>
#include <gearbox/core/Errors.h>
#include <gearbox/core/ConfigFile.h>
#include <gearbox/core/strlcpy.h>
#include <gearbox/core/util.h>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>

namespace bfs=boost::filesystem;

// for flock
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>

// for socket
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include <math.h> // for floor

using namespace Gearbox::Database;
using namespace Gearbox;
using std::string;

WorkerGearbox::WorkerGearbox(const string &config, bool withSync, bool withAsync) : Worker(config) {

    if( withSync ) {
        WORKER_REGISTER(WorkerGearbox, do_put_delay_job_v1);
        WORKER_REGISTER(WorkerGearbox, do_get_global_status_v1);
        WORKER_REGISTER(WorkerGearbox, do_create_global_status_v1);
        WORKER_REGISTER(WorkerGearbox, do_update_global_status_v1);
        WORKER_REGISTER(WorkerGearbox, do_decrement_global_counter_v1);
    }
    if( withAsync ) {
        WORKER_REGISTER(WorkerGearbox, do_post_global_status_v1);
        WORKER_REGISTER(WorkerGearbox, do_stop_global_status_v1);
        WORKER_REGISTER(WorkerGearbox, do_cancelwatch_global_status_v1);
        WORKER_REGISTER(WorkerGearbox, do_pollstate_global_status_v1);

        WORKER_REGISTER(WorkerGearbox, do_run_global_agents_v1);
        WORKER_REGISTER(WorkerGearbox, do_runlevel_global_agents_v1);
    }

    db_init( this->cfg().get_json("delay"), "delay");
    db_init( this->cfg().get_json("status"), "status");
}


void WorkerGearbox::poke_delay_daemon() {
    string fifo(this->cfg().get_string_default("delay", "fifo", ""));
    if ( fifo.empty() ) {
        fifo = this->cfg().get_string_default("gearbox", "root", LOCALSTATEDIR "/gearbox");
        fifo += "/delay.fifo";
    }

    if( bfs::exists(fifo) ) {
        try {
            signal(SIGPIPE,SIG_IGN);
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            struct sockaddr_un server;
            if (sock < 0) {
                _FATAL("failed to create socket " << sock << ": " << strerror(errno));
                gbTHROW( ERR_LIBC("failed to create socket") );
            }
            server.sun_family = AF_UNIX;
            strlcpy(server.sun_path, fifo.c_str(), sizeof(server.sun_path));

            if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
                _FATAL("Failed to connect to stream socket: " << strerror(errno));
                close(sock);
                gbTHROW( ERR_LIBC("failed to connect to stream socket") );
            }
            if (write(sock, "1", 1) < 0) {
                if( errno != EPIPE ) {
                    _FATAL("failed to write to stream socket: " << strerror(errno));
                    close(sock);
                    gbTHROW( ERR_LIBC("failed to write to stream socket") );
                }
            }
            close(sock);
        }
        catch (const std::exception & e) {
            // just warn to logs here, if we cant poke the daemon
            // the process will still get picked up eventually
            _WARN(e.what());
        }
    }
}

Worker::response_t
WorkerGearbox::do_put_delay_job_v1( const Job & job, JobResponse & resp )
{
    const Json & content = job.json_content();

    // we need to make sure this gets in the queue since we have accepted
    // the work
    while( true ) {
        try {
            soci::session & sess = getconn("delay");
            LoggedStatement lst(sess);
            time_t now = time(NULL);
            std::string status_name;
            if( content.hasKey("status_name") ) {
                status_name = content["status_name"].as<string>();
            }
            soci::indicator is_null = status_name.empty() ? soci::i_null : soci::i_ok;
            string encoded;
            zlib_compress(content["envelope"].serialize(),encoded);
            dblock("delay");
            lst << "INSERT INTO jobs (name,status_name,envelope,time,ctime) VALUES(:name,:status_name,:envelope,:time,:ctime)";
            lst.use(content["name"].as<string>()).use(status_name,is_null);
            // mysql does not support blobs, but std::strings work fine with blob data
            sess.get_backend_name() == "mysql" ? lst.use(encoded) : lst.use_blob(encoded);
            lst.use(this->cfg().get_int_default("delay","run_immediately",0) ? 0 : content["time"].as<long>()).use(now).execute(true);
            dbcommit("delay");
            break;
        }
        catch (const std::exception & err) {
            _WARN("Failed to insert delay job: " << err.what());
            dbrollback("delay");
            sleep(1);
        }
    }

    this->poke_delay_daemon();
    return WORKER_SUCCESS;
}

Worker::response_t
WorkerGearbox::do_get_global_status_v1( const Job & job, JobResponse & resp )
{
    this->status_manager().base_uri( job.base_uri() );

    if ( job.arguments().empty() ) {

        int expand=0;
        if( job.matrix_arguments().has_key("_expand") ) {
            expand = boost::lexical_cast<int>(job.matrix_arguments()["_expand"]);
        }
        this->status_manager().base_uri( job.base_uri() );
        StatusCollectionPtr sc = this->status_manager().collection();

        if( job.matrix_arguments().has_key("_count") ) {
            unsigned int count = boost::lexical_cast<unsigned int>(job.matrix_arguments()["_count"]);
            sc->limit(count);
        }
        if( job.matrix_arguments().has_key("progress") ) {
            int progress = boost::lexical_cast<int>(job.matrix_arguments()["progress"]);
            sc->filter_progress(progress,progress);
        }
        if( job.matrix_arguments().has_key("code") ) {
            int code = boost::lexical_cast<int>(job.matrix_arguments()["code"]);
            sc->filter_code(code,code);
        }
        if( job.matrix_arguments().has_key("operation") ) {
            sc->filter_operation(job.matrix_arguments()["operation"]);
        }
        if( job.matrix_arguments().has_key("component") ) {
            sc->filter_component(job.matrix_arguments()["component"]);
        }
        if( job.matrix_arguments().has_key("state") ) {
            sc->filter_state(job.matrix_arguments()["state"]);
        }
        if( job.matrix_arguments().has_key("uri") ) {
            sc->filter_uri(job.matrix_arguments()["uri"]);
        }

        Json out;
        for (unsigned int i=0; !sc->empty(); i++) {
            StatusPtr s = sc->pop();
            if ( expand ) {
                out["statuses"][i].parse(s->serialize());
            }
            else {
                out["statuses"][i] = s->name();
            }
        }
        if ( out.empty() ) {
            out["statuses"] = Json::Array();
        }
        resp.content( out.serialize() );
        return WORKER_SUCCESS;
    }
    StatusPtr s = this->status_manager().fetch(job.arguments()[0]);
    resp.content( s->serialize() );
    return WORKER_SUCCESS;
}


Worker::response_t
WorkerGearbox::do_cancelwatch_global_status_v1( const Job & job, JobResponse & resp ) {
    const Json & in = job.json_content();

    const StatusManager & sm = this->status_manager();

    int i=0;
    for( ; i < in["watch"].length(); ++i ) {
        StatusPtr s = sm.fetch( Uri(in["watch"][i].as<string>()) );
        if( ! s->has_completed() ) {
            break;
        }
        else if( ! s->is_success() ) {
            StatusPtr status = this->status_manager().fetch( job.arguments()[0] );
            status->fail(s->code());
            throw_from_code(s->code(), s->messages().back());
        }
    }
    
    Json cancelwatch;
    if( i < in["watch"].length() ) {
        for( int j=0; i+j < in["watch"].length(); ++j ) {
            cancelwatch["watch"][j] = in["watch"][i+j];
        }

        cancelwatch["state"] = in["state"];
        Job cancelWatchJob(job);
        cancelWatchJob.name("do_cancelwatch_global_status_v1");
        cancelWatchJob.content(cancelwatch.serialize());
        this->afterwards(cancelWatchJob,10);
        return WORKER_CONTINUE;
    }

    StatusPtr s = this->status_manager().fetch( job.arguments()[0] );
    const string & state = in["state"].as<string>();
    if( state == "CANCELLING" ) {
        // now try to cancel the job for the current status
        JobPtr on_cancel = s->on(Status::EVENT_CANCEL, this->job_manager());
        // was there a cancel job registered?
        if( on_cancel.get() ) {
            resp.status()->progress(70);
            JobResponse r = on_cancel->run();
            cancelwatch["watch"][0] = r.status()->uri();
            cancelwatch["state"] = "CANCELLED";

            Job cancelWatchJob(job);
            cancelWatchJob.name("do_cancelwatch_global_status_v1");
            cancelWatchJob.content(cancelwatch.serialize());
            this->afterwards(cancelWatchJob);
            return WORKER_CONTINUE;
        }
        // if no cancel job then just go straight to CANCELLED
        s->cancel();
    }
    else if ( state == "CANCELLED" ) {
        s->cancel();
    }
    return WORKER_SUCCESS;
}

Worker::response_t
WorkerGearbox::do_pollstate_global_status_v1( const Job & job, JobResponse & resp ) {
    StatusPtr s = this->status_manager().fetch( job.arguments()[0] );
    const Json & in = job.json_content();

    if( s->state() == Status::STATE_CANCELLED ) {
        // no where to go from CANCELLED state so just return OK
        return WORKER_SUCCESS;
    }

    const string & state = in["state"].as<string>();

    if( state == "STOPPED" &&
        ( s->state() == Status::STATE_STOPPED
          || s->state() == Status::STATE_COMPLETED ) ) {
        if( in.hasKey("next_state" ) ) {
            resp.status()->progress(40);
            const string & next_state = in["next_state"].as<string>();
            if( next_state == "CANCELLED" ) {
                if( ! s->state( Status::STATE_CANCELLING ) ) {
                    return WORKER_RETRY;
                }
                try {
                    // cancel all children first
                    const Array & children = s->children();
                    Json cancelwatch;
                    cancelwatch["watch"] = Json::Array();
                    const JobManager & jm = this->job_manager();
                    for( unsigned int i=0; i < children.size(); ++i ) {
                        JobResponse r = jm.job(HttpClient::METHOD_POST, children[i])->content("{\"state\":\"CANCELLED\"}").run();
                        if( r.status()->has_completed() && ! r.status()->is_success() ) {
                            throw_from_code( r.code(), r.status()->messages().back() );
                        }
                        cancelwatch["watch"][i] = r.status()->uri();
                    }
                    // what state do we end with ..
                    cancelwatch["state"] = "CANCELLING";

                    Job cancelWatchJob(job);
                    cancelWatchJob.name("do_cancelwatch_global_status_v1");
                    cancelWatchJob.content(cancelwatch.serialize());
                    return this->do_cancelwatch_global_status_v1(cancelWatchJob, resp);
                }
                catch( const std::exception & err ) {
                    s->state(Status::STATE_COMPLETED);
                    throw;
                }
            }
            else {
                gbTHROW( ERR_INTERNAL_SERVER_ERROR("Unexpected state transition requested: " + next_state) );
            }
        }
        else {
            return WORKER_SUCCESS;
        }
    }

    this->afterwards(job,10);
    return WORKER_CONTINUE;
}

Worker::response_t
WorkerGearbox::do_stop_global_status_v1( const Job & job, JobResponse & resp )
{
    // was this called after a pre-cancel?
    // if so we want to abort if the pre-cancel check failed
    StatusPtr evt = job.event_status();
    if( evt.get() ) {
        if( ! evt->is_success() ) {
            throw_from_code(evt->code(), evt->messages().back());
        }
    }

    resp.status()->progress(10);
    StatusPtr s = this->status_manager().fetch( job.arguments()[0] );
    Status::State old_state = s->state();

    if( old_state == Status::STATE_PENDING || old_state == Status::STATE_RUNNING ) {
        // it is probably in the queue so update the time to run asap
        // (we need it to run so that on$EVENT handlers will be run
        // after the Worker base does a checkpoint)
        if( ! s->state( Status::STATE_STOPPING ) ) {
            return WORKER_RETRY;
        }
        if( s->ytime() >= 0 ) {
            soci::session & sess = getconn( "delay", true );
            LoggedStatement lst(sess);
            dblock("delay");
            lst << "UPDATE jobs SET time = 0 WHERE status_name=:name";
            lst.use(s->name()).execute(true);
            dbcommit("delay");
            s->ytime(0);
            this->poke_delay_daemon();
        }
        else if( !s->concurrency() ) {
            // not in delay queue, and no process are working on it
            // so mark it as stopped
            try {
                // this will mark as STOPPED
                // set the error code and progress
                s->checkpoint();
            }
            catch( const WorkerStop & err) {
                // dont care, we expect this from checkpoint
            }
        }
    }

    Job pollJob(job);
    pollJob.name("do_pollstate_global_status_v1");
    pollJob.content("{\"state\": \"STOPPED\",\"next_state\":\"CANCELLED\"}");
    return this->do_pollstate_global_status_v1(pollJob, resp);
}

Worker::response_t
WorkerGearbox::do_post_global_status_v1( const Job & job, JobResponse & resp )
{
    if ( job.arguments().empty() ) {
        gbTHROW( ERR_BAD_REQUEST("missing required resource name") );
    }
    StatusPtr s = this->status_manager().fetch( job.arguments()[0] );
    const Json & in = job.json_content();
    if ( in.hasKey("state") ) {
        resp.status()->progress(1);
        const string & state = in["state"].as<string>();
        if ( state == "CANCELLED" ) {
            Status::State old_state = s->state();
            if( old_state == Status::STATE_CANCELLED )
                return WORKER_SUCCESS;
            if( old_state == Status::STATE_STOPPING
                || old_state == Status::STATE_CANCELLING )
                gbTHROW( ERR_CONFLICT("state is currently " + Status::state2str(old_state)) );

            JobPtr on_precancel = s->on(Status::EVENT_PRECANCEL, this->job_manager());
            // was there a precancel job registered?
            if( on_precancel.get() ) {
                Job stopJob(job);
                stopJob.name("do_stop_global_status_v1");

                on_precancel->on(Job::EVENT_COMPLETED, stopJob);
                this->afterwards(*on_precancel);
                return WORKER_CONTINUE;
            }

            // no precancel job so go straight to do_stop....
            return this->do_stop_global_status_v1(job,resp);
        }
        else {
            gbTHROW( ERR_BAD_REQUEST("transition to state '" + state + "' unsupported") );
        }
    }
    else if( in.hasKey("ytime") ) {
        int64_t ytime = in["ytime"].as<int64_t>();
        soci::session & sess = getconn( "delay", true);
        LoggedStatement lst(sess);
        dblock("delay");
        lst << "UPDATE jobs SET time=:time WHERE status_name=:name";
        lst.use(ytime).use(s->name()).execute(true);
        dbcommit("delay");
        s->ytime(ytime);
        this->poke_delay_daemon();
    }
    return WORKER_SUCCESS;
}

Worker::response_t
WorkerGearbox::do_create_global_status_v1( const Job & job, JobResponse & resp )
{
    const Json & content = job.json_content();
    this->status_manager().base_uri( job.base_uri() );

    try {
        // create status object, it writes to the db
        StatusPtr s = this->status_manager().create(
            content["name"].as<string>(),
            content["operation"].as<string>(),
            content["uri"].as<string>(),
            content["component"].as<string>()
        );
        if( content.hasKey("parent_uri") ) {
            s->parent_uri( content["parent_uri"].as<string>() );
        }
    }
    catch ( const std::exception & err ) {
        _FATAL("Worker: Unable to create job status: " << err.what() );
        gbTHROW( ERR_INTERNAL_SERVER_ERROR(err.what()) );
    }
    return WORKER_SUCCESS;
}

Worker::response_t
WorkerGearbox::do_update_global_status_v1( const Job & job, JobResponse & resp )
{
    const Json & content = job.json_content();
    StatusPtr status = this->status_manager().fetch(job.arguments()[0]);
    if( content.hasKey("children") ) {
        for( int i=0; i < content["children"].length(); i++ ) {
            status->add_child( content["children"][i].as<string>() );
        }
    }
    else if( content.hasKey("progress") ) {
        status->progress( content["progress"].as<int>() );
    }
    else {
        throw ERR_BAD_REQUEST("unexpected content for do_update_global_status_v1");
    }
    return WORKER_SUCCESS;
}

Worker::response_t
WorkerGearbox::do_run_global_agents_v1( const Job & job, JobResponse & resp ) {
    const Json & in = job.json_content();
    this->job_manager().parent_uri( resp.status()->uri() );
    JobQueue q = this->job_manager().job_queue( in["agents"] );

    JobPtr updateProgress = this->job_manager().job("do_update_global_status_v1");
    updateProgress->type(Job::JOB_SYNC);
    updateProgress->add_argument( resp.status()->name() );

    Job nextLevel(job);
    nextLevel.name("do_runlevel_global_agents_v1");
    nextLevel.content("{\"jobs\":[]}");

    JobManager jm(this->cfg());

    Json run_level;
    run_level["content"] = in["content"];
    // start at the last level chaining them together backwards
    // skip the last level as we will run that now, without calling
    // the runlevel job
    for( int level = q.size(); level > 0; --level ) {
        run_level["jobs"].clear();
        const std::vector<JobPtr> & in_degree = q[level - 1];
        int progress = (int)floor((100 / q.size()) * (level-1));
        if( progress == 0 ) progress = 1;
        if( in_degree.size() == 1 ) {
            in_degree[0]->on(Job::EVENT_COMPLETED, nextLevel);
            run_level["jobs"][0]["name"] = in_degree[0]->name();
            run_level["jobs"][0]["envelope"].parse(in_degree[0]->serialize());
        }
        else {
            std::string counter = make_counter(in_degree.size());

            JobPtr dec_counter = jm.job("do_decrement_global_counter_v1");
            dec_counter->type(Job::JOB_SYNC);
            dec_counter->on(Job::EVENT_COMPLETED, nextLevel);
            dec_counter->add_argument(counter);

            for( unsigned int i=0; i < in_degree.size(); ++i ) {
                in_degree[i]->on(Job::EVENT_COMPLETED, *dec_counter);
                run_level["jobs"][i]["name"] = in_degree[i]->name();
                run_level["jobs"][i]["envelope"].parse(in_degree[i]->serialize());
            }
        }
        updateProgress->content(
            "{\"progress\": " + boost::lexical_cast<string>(progress) + "}"
        );
        nextLevel.on(Job::EVENT_STARTED, *updateProgress);
        nextLevel.content(run_level.serialize());
    }
    this->afterwards(nextLevel);
    return WORKER_CONTINUE;
}

Worker::response_t
WorkerGearbox::do_runlevel_global_agents_v1( const Job & job, JobResponse & resp ) {
    StatusPtr event = job.event_status();
    if( event.get() ) {
        if( ! event->is_success() ) {
            const Array & msgs = event->messages();
            std::string err;
            if( msgs.empty() ) {
                err = "Unknown error from agent";
            }
            else {
                err = msgs.back();
            }
            throw_from_code(event->code(), err);
        }
    }

    const Json & in = job.json_content();

    if( in["jobs"].length() == 0 ) {
        return WORKER_SUCCESS;
    }

    for( int i=0; i < in["jobs"].length(); ++i ) {
        JobPtr agent = this->job_manager().job(
            in["jobs"][i]["name"].as<std::string>(),
            in["jobs"][i]["envelope"]
        );
        agent->content( in["content"].as<std::string>() );
        this->afterwards(*agent);
    }
    return WORKER_CONTINUE;
}

static const std::string COUNTER_PATH(LOCALSTATEDIR "/gearbox/db/counters/");

std::string
WorkerGearbox::make_counter(unsigned int start) {
    std::string id = JobManager::gen_id("i");
    std::string file = COUNTER_PATH + id;
    write_file(file, boost::lexical_cast<std::string>(start));
    return id;
}

Worker::response_t  WorkerGearbox::do_decrement_global_counter_v1( const Job & job, JobResponse & resp ) {

    StatusPtr evt = job.event_status();
    if( evt.get() ) {
        if( ! evt->is_success() ) {
            resp.status(*evt);
            throw_from_code(evt->code(), evt->messages().back());
        }
    }

    if( job.arguments().empty() ) {
        gbTHROW( ERR_BAD_REQUEST("missing counter id") );
    }
    int fd = -1;
    std::string counter_file(COUNTER_PATH + job.arguments()[0]);
    if( !bfs::exists(counter_file) ) {
        _WARN( counter_file << " does not exist!");
        return WORKER_SUCCESS;
    }

    try {
        fd = open(counter_file.c_str(), O_APPEND);
        if( fd == -1 ) {
            gbTHROW(ERR_LIBC("failed to open " + counter_file + " for locking"));
        }
        if( flock(fd, LOCK_EX) ) {
            gbTHROW(ERR_LIBC("failed to get exclusive lock on " + counter_file));
        }
        int newval = boost::lexical_cast<int>(slurp(counter_file)) - 1;
        write_file(
            counter_file,
            boost::lexical_cast<std::string>(newval)
        );
        if( flock(fd, LOCK_UN) ) {
            gbTHROW(ERR_LIBC("failed to unlock " + counter_file));
        }
        if( close(fd) ) {
            gbTHROW(ERR_LIBC("failed to close " + counter_file + " after locking"));
        }
        fd = -1;
        if ( newval > 0 )
            return WORKER_CONTINUE;

        bfs::remove(counter_file);
    }
    catch( const std::exception & err ) {
        _ERROR(err.what());
        _ERROR("failed to decrement counter, will try again");
        if( fd >= 0 ) {
            flock(fd,LOCK_UN);
            close(fd);
        }
        return WORKER_RETRY;
    }
    return WORKER_SUCCESS;
}

