// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include "config.h"
#include <gearbox/job/JobManager.h>

#include <gearbox/core/logger.h>
#include <gearbox/core/Errors.h>

#include <gearbox/job/GearmanJobImpl.h>
#include <gearbox/job/RestJobImpl.h>

#include <gearbox/core/util.h> // uuid_b32c urand

#include <unistd.h> // usleep
#include <glob.h>
#include <stdexcept>
#include <queue>

#include <gearbox/core/JsonSchema.h>
#include <sys/stat.h> // stat

#include <limits>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs=boost::filesystem;

namespace Gearbox {
    typedef std::map< std::string, boost::shared_ptr<JsonSchema> > SchemaMap;
    static SchemaMap schemas;

    static JsonSchema * getSchema(const std::string & schemaName, const ConfigFile & cfg) {
        static std::string schemadir = cfg.get_string_default("schemadir", DATADIR "/gearbox/schemas");
        std::string schema = schemadir + "/" + schemaName + ".js";
        if( schemas.find(schema) == schemas.end() ) {
            struct stat buf;
            if( stat(schema.c_str(), &buf) == 0 ) {
                _TRACE("Using Schema " << schema);
                schemas.insert( SchemaMap::value_type(schema, boost::shared_ptr<JsonSchema>(new JsonSchema())) );
                schemas[schema]->parseFile( schema );
            }
            else {
                _DEBUG("Schema " << schema << " not found, using NULL schema");
                schemas.insert( SchemaMap::value_type(schema, boost::shared_ptr<JsonSchema>()) );
            }
        }
        return schemas[schema].get();
    }
    
    JsonSchema * JobManager::getSchema(const JobImpl * ji, const ConfigFile & cfg ) {
        std::string schema = ji->operation() 
            + "-" + ji->component()
            + "-" + ji->resource_type()
            + "-" + ji->api_version();
        JsonSchema * s = Gearbox::getSchema(schema, cfg);
        if( !s ) {
            schema = ji->operation()
                + "-global"
                + "-" + ji->resource_type()
                + "-" + ji->api_version();
            s = Gearbox::getSchema(schema, cfg);
        }
        if( !s ) {
            int needed = cfg.get_int_default("require_schemas", -1);
            bool fail = false;
            // if not explicitly enabled/disabled in the worker
            // config file then check the global static setting
            if( needed == -1 ) {
                if( JobManager::require_schemas ) {
                    fail = true;
                }
            }
            if( needed == 1 ) {
                fail = true;
            }
            if( fail ) {
                gbTHROW( std::runtime_error("Required schema for job " + ji->name() + " not found!") );
            }
        }
                
        return s;
    }

    bool JobManager::require_schemas(false);

    struct JobManager::Private {
        ConfigFile cfg;
        std::string parent_uri;
        std::string base_uri;
        static std::map<std::string, int> runnable_jobs;
        Private(const Private & copy)
            : cfg(copy.cfg),
              parent_uri(copy.parent_uri),
              base_uri(copy.base_uri) {}

        Private & operator=(const Private & copy) {
            this->cfg = copy.cfg;
            this->parent_uri = copy.parent_uri;
            this->base_uri = copy.base_uri;
            return *this;
        }

        Private(const ConfigFile & c) : cfg(c) {
            if( runnable_jobs.empty() ) {
                std::string job_file = cfg.get_string("handlers_file" );
                if ( ! job_file.empty() ) {
                    _DEBUG( "Enabling jobs from file '" << job_file << "'..." );
                    Json hcfg;
                    hcfg.parseFile( job_file );
                    for ( int i=0; i < hcfg["handlers"].length(); i++ ) {
                        std::string job = hcfg["handlers"][i].as<std::string>();
                        _TRACE( "-> " << job );
                        this->runnable_jobs[job] = 1;
                    }
                }
                
                std::string confdir = cfg.get_string_default("gearbox", "conf", SYSCONFDIR "/gearbox");
                glob_t globbuf;
                glob( std::string(confdir + "/*handlers[._]d").c_str(), 0, NULL, &globbuf);
        
                std::vector<std::string> job_dirs;
                for( unsigned int i=0; i < globbuf.gl_pathc; i++ ) {
                    if( bfs::is_directory(bfs::path(globbuf.gl_pathv[i])) ) {
                        job_dirs.push_back( globbuf.gl_pathv[i] );
                    }
                }
                
                globfree(&globbuf);
                
                for( unsigned int i=0; i < job_dirs.size(); ++i ) {
                    _DEBUG( "Enabling jobs from dir '" << job_dirs[i] << "'..." );
                    bfs::directory_iterator end;
                    for ( bfs::directory_iterator itr( job_dirs[i] );
                          itr != end;
                          ++itr ) {
                        std::string job = itr->path().filename().string();
                        size_t s = job.find( "do_", 0 );
                        if ( s < std::string::npos ) {
                            s = job.find("_", s);
                            if( s < std::string::npos ) {
                                _TRACE( "-> " << job );
                                this->runnable_jobs[job] = 1;             
                            }
                            else {
                                _DEBUG( "Ignoring '" << job << "' (does not start with do_)" );
                            }
                        }
                        else {
                            _DEBUG( "Ignoring '" << job << "' (bad prefix)" );
                        }
                    }
                }
            }
        }
    };

    std::map<std::string, int> JobManager::Private::runnable_jobs;

    JobManager::JobManager(const ConfigFile & cfg) : impl( new Private(cfg) ) {}
    
    JobManager::JobManager(const JobImpl * job) : impl( new Private( job->cfg() ) ) {
        this->base_uri( job->base_uri() );
        this->parent_uri( job->parent_uri() );
    }

    JobManager::JobManager(const JobManager & copy) : impl(new Private(*copy.impl)) {}
    
    JobManager & JobManager::operator=(const JobManager & copy ) {
        if( this == &copy ) return *this;
        *(this->impl) = *(copy.impl);
        return *this;
    }

    JobManager::~JobManager() {
        if( impl ) delete impl;
    }

    void 
    JobManager::parent_uri( const std::string & parent_uri_uri ) {
        impl->parent_uri = parent_uri_uri;
    }

    void
    JobManager::base_uri( const std::string & base_uri ) {
        impl->base_uri = base_uri;
    }

    void
    JobManager::cfg( const ConfigFile & c ) {
        impl->cfg = c;
    }

    bool JobManager::known_job_name( const std::string & name ) const {
        if ( impl->cfg.get_int_default( "allow_unknown_jobs", 0 ) ) {
            return true;
        }
        return impl->runnable_jobs[name];
    }

    JobPtr
    JobManager::job(const std::string & job_name) const {
        if ( ! this->known_job_name(job_name) ) {
            gbTHROW( std::invalid_argument("No such job '" + job_name + "' enabled in config.") );
        }
        
        std::auto_ptr<JobImpl> ji(new GearmanJobImpl(impl->cfg));
        ji->base_uri( impl->base_uri );
        ji->parent_uri( impl->parent_uri );
        ji->name(job_name);
        return JobPtr( new Job( ji.release() ) );
    }

    JobPtr
    JobManager::job(HttpClient::method_t method, const Uri & job_uri) const {
        std::auto_ptr<JobImpl> ji(new RestJobImpl(impl->cfg));
        ji->base_uri( impl->base_uri );
        ji->parent_uri( impl->parent_uri );
        ji->method(method);
        ji->name(job_uri.canonical());
        return JobPtr( new Job(ji.release()) );
    }

    JobPtr
    JobManager::job(const std::string & job_name, const std::string & envelope) const {
        Json job_envelope;
        job_envelope.setSchema( Gearbox::getSchema("job-envelope", impl->cfg) );
        job_envelope.parse(envelope);
        return this->job(job_name, job_envelope);
    }

    JobPtr
    JobManager::job(const std::string & job_name, const Json & job_envelope ) const {
        if ( ! this->known_job_name(job_name) ) {
            _WARN("No such job '" + job_name + "' enabled in config.");
        }
        std::auto_ptr<JobImpl> ji(new GearmanJobImpl(impl->cfg));
        ji->name(job_name);

        if ( job_envelope.hasKey("job_type") ) {
            std::string type = job_envelope["job_type"].as<std::string>();
            if( type == "sync" ) {
                ji->type(Job::JOB_SYNC);
            }
            else if ( type == "async" ) {
                ji->type(Job::JOB_ASYNC);
            }
            else {
                gbTHROW( std::invalid_argument(
                    "job envelope does not contain a valid job_type"
                ) );
            }
        }
        // this must be done before status and base_uri are parsed
        // so that they dont taint the currently running job manager
        // which should have the base_uri/parent_uri set by the "current"
        // jobs, not the on-event jobs
        if( job_envelope.hasKey("on") ) {
            const Json::Object & obj = job_envelope["on"].as<Json::Object>();
            Json::Object::const_iterator it  = obj.begin();
            Json::Object::const_iterator end = obj.end();
            for( ; it != end; ++it ) {
                JobPtr j = this->job(
                    it->second->get("name").as<std::string>(),
                    it->second->get("envelope")
                );
                ji->on( Job::str2event(it->first), *j );
            }
        }
        if ( job_envelope.hasKey("version") )
            ji->api_version( job_envelope["version"].as<std::string>() );
        if ( job_envelope.hasKey("operation") )
            ji->operation( job_envelope["operation"].as<std::string>() );
        if ( job_envelope.hasKey("component") )
            ji->component( job_envelope["component"].as<std::string>() );
        if ( job_envelope.hasKey("resource") ) {
            if ( job_envelope["resource"].hasKey("type") )
                ji->resource_type( job_envelope["resource"]["type"].as<std::string>() );
            if ( job_envelope["resource"].hasKey("name") )
                ji->resource_name( job_envelope["resource"]["name"].as<std::string>() );
        }
        if( job_envelope.hasKey("parent_uri") )
            ji->parent_uri( job_envelope["parent_uri"].as<std::string>() );
        if( job_envelope.hasKey("base_uri") ) {
            ji->base_uri( job_envelope["base_uri"].as<std::string>() );
            impl->base_uri = job_envelope["base_uri"].as<std::string>();
        }
        if( job_envelope.hasKey("status") ) {
            ji->status( job_envelope["status"].as<std::string>() );
        }

        // this throws when no schema available and requireSchema is set
        this->getSchema(ji.get(), impl->cfg);

        if( job_envelope.hasKey("content") ) {
            ji->content( job_envelope["content"].as<std::string>() );
        }
        
        if( job_envelope.hasKey("headers") ) {
            const Json::Object & obj = job_envelope["headers"].as<Json::Object>();
            Json::Object::const_iterator it = obj.begin();
            for( ; it != obj.end(); ++it ) {
                // if this was a remote http call then
                // y-staus-parent-uri header will be set
                if( it->first == "y-status-parent-uri" ) {
                    ji->parent_uri( it->second->as<std::string>() );
                }
                else {
                    ji->add_header(it->first, it->second->as<std::string>());
                }
            }
        }

        if( job_envelope.hasKey("matrix_arguments") ) {
            const Json::Object & obj = job_envelope["matrix_arguments"].as<Json::Object>();
            Json::Object::const_iterator it = obj.begin();
            for( ; it != obj.end(); ++it ) {
                ji->add_matrix_argument(it->first, it->second->as<std::string>());
            }
        }
            
        if( job_envelope.hasKey("environ") ) {
            const Json::Object & obj = job_envelope["environ"].as<Json::Object>();
            Json::Object::const_iterator it = obj.begin();
            for( ; it != obj.end(); ++it ) {
                ji->add_environ(it->first, it->second->as<std::string>());
            }
        }

        if( job_envelope.hasKey("query_params") ) {
            const Json::Object & obj = job_envelope["query_params"].as<Json::Object>();
            Json::Object::const_iterator it = obj.begin();
            for( ; it != obj.end(); ++it ) {
                ji->add_query_param(it->first, it->second->as<std::string>());
            }
        }
            
        if( job_envelope.hasKey("arguments") ) {
            const Json::Array & arr = job_envelope["arguments"].as<Json::Array>();
            Json::Array::const_iterator it = arr.begin();
            for( ; it != arr.end(); ++it ) {
                ji->add_argument((*it)->as<std::string>());
            }
        }

        if( job_envelope.hasKey("event_status") ) {
            StatusManager sm(impl->cfg);
            ji->event_status( *(sm.fetch(job_envelope["event_status"])) );
        }

        return JobPtr( new Job(ji.release()) );
    }

    void
    JobManager::delay(const Job & job, int seconds ) const {
        _DEBUG("delaying job " << job.name() << " for " << seconds << " seconds.");
        JobPtr dj = this->job("do_put_delay_job_v1");
        dj->type(Job::JOB_SYNC);
        Json delay;
        
        time_t when = time(NULL) + seconds;

        delay["name"] = job.name();
        delay["envelope"].parse(job.serialize());
        delay["time"] = when;

        if( ! job.status().empty() ) {
            StatusManager sm(impl->cfg);
            StatusPtr s = sm.fetch(job.status());
            s->ytime( when );
            delay["status_name"] = s->name();
        }

        dj->content(delay.serialize());
        while( true ) {
            try {
                JobResponse resp = dj->run();
                if( resp.status()->has_completed() && !resp.status()->is_success() ) {
                    _WARN("Failed to put delay job " << dj->name() << ": " << resp.status()->messages().back());
                    sleep(1);
                    continue;
                }
                break;
            }
            catch(const std::exception & err) {
                _WARN("Failed to run delay job: " << err.what());
                sleep(1);
            }
        }
    }

    void
    JobManager::retry( const Job & job, int max_delay, int max_jitter ) {

        // keep track of how many times this job has been retried
        int retry = 1;
        if( ! job.status().empty() ) {
            StatusManager sm(impl->cfg);
            StatusPtr s = sm.fetch(job.status());
            retry = s->failures() + 1;
            s->failures( retry );
        }

        _DEBUG("retrying job " << job.name() << " for the " << retry << " attempt");

        // fib(35) = 9220695 seconds =~ 106 days
        // that's excessive and we want to avoid an overflow
        if ( max_delay > 9220695 ) {
            _WARN("maximum max_delay is 9220695 seconds (which is fib(35))");
            max_delay = 9220695;
        }
        int delay = max_delay;

        // formula for the nth fibonicci number
        float phi = 1.618;
        if ( retry < 35 ) 
            delay = (int)(ceil( ( pow( phi, retry ) - pow( 1 - phi, retry ) ) / sqrt( 5 ) ) );

        if ( delay > max_delay )
            delay = max_delay;

        if ( delay > max_jitter )
            delay += (int) (max_jitter * (urand() / (std::numeric_limits<uint32_t>::max() + 1.0)));
        
        this->delay( job, delay );
    }

    std::string
    JobManager::gen_id(const std::string & prefix) {
        std::string id = prefix + "-";
        std::string uuid;
        uuid_b32c( uuid, false );
        id += uuid;
        return id;
    }

    JobQueue
    JobManager::job_queue( const Json & job_config ) const {
        _TRACE("Agent configuration:" << job_config.serialize() );
        // define graph types    
        typedef std::vector<JobPtr> Nodes;
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> Graph;
        typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
        typedef std::list<Vertex> Order;

        JobQueue jq;

        // parse the JSON agent configuration into a set of nodes
        if ( job_config.empty() ) return jq;

        Nodes nodes;
        Json::Object & sa = job_config.as<Json::Object>();
        Json::Object::iterator it = sa.begin();
        for (; it != sa.end(); ++it ) {
            Job::JobType type = Job::JOB_ASYNC;
            Json & value = *(it->second);
            if( value.hasKey("type") ) {
                if( value["type"].as<std::string>() == "SYNC" ) {
                    type = Job::JOB_SYNC;
                }
            }
            JobPtr j;
            if( value.hasKey("method") ) {
                std::string mval = value["method"].as<std::string>();
                HttpClient::method_t method = 
                    mval == "PUT"    ? HttpClient::METHOD_PUT :
                    mval == "POST"   ? HttpClient::METHOD_POST :
                    mval == "DELETE" ? HttpClient::METHOD_DELETE :
                                       HttpClient::METHOD_GET;
                j = this->job(method, it->first);
                j->type(Job::JOB_UNKNOWN);
            }
            else {
                j = this->job(it->first);
                j->type(type);
            }
            j->impl->base_uri( impl->base_uri );
            j->impl->parent_uri( impl->parent_uri );
            
            // 0 timeout means we shouldn't timeout
            int timeout = value.hasKey("timeout") ? value["timeout"].as<int>() : 0;
            j->timeout(timeout);
            nodes.push_back(j);
        }

        // allocate a graph of N nodes.
        Graph graph( nodes.size() );
        int i;
        for ( i=0; i<=(int)nodes.size()-1; i++ ) {
            std::string node = nodes.at(i)->name();
            if ( !sa[node]->hasKey("require") ) continue;
            for ( int j=0; j<=(*sa[node])["require"].length()-1; j++ ) {
                std::string req = (*sa[node])["require"][j].as<std::string>();
                int k = 0;
                bool found = false;
                Nodes::iterator it = nodes.begin();
                for( ; it != nodes.end(); it++ ) {
                    if ( req == (*it)->name() ) { found=true; break; }
                    k++;
                }
                if ( !found ) {
                    gbTHROW( ERR_INTERNAL_SERVER_ERROR(
                        "Agent: '" + node + "' has undefined dependency '" +
                        req + "'" ) );
                }
                else if ( i == k ) {
                    gbTHROW( ERR_INTERNAL_SERVER_ERROR(
                        "Agent: '" + node + "' defines a cyclic dependency!" ) ); 
                }
                boost::add_edge(k, i, graph);
            }
        }
    
        // topologically sort the graph, providing dependency ordering -- will throw "not a dag"
        // if cycles are discovered.
        Order order;
        try {
            boost::topological_sort(graph, std::front_inserter(order));
        }
        catch ( std::exception & e ) {
            gbTHROW( ERR_INTERNAL_SERVER_ERROR( "Cycle detected in configuration: " + std::string(e.what()) ) );
        }
    
        // for nodes that have dependencies (in_degree() > 0) iterate through the edges and count them,
        // this provides grouping of nodes which are orthogonal to one another and can run in parallel
        std::vector<int> edge_count( nodes.size(), 0 );
        std::map<int, std::vector<JobPtr> > group;
        for (Order::iterator it = order.begin(); it != order.end(); ++it ) {
            if ( ! in_degree( *it, graph ) ) {
                group[0].push_back( nodes.at( *it ) );
                _TRACE("-> " << *it << " - " << edge_count[*it] << " - " << nodes.at(*it)->name() );
            }
            else {             
                Graph::in_edge_iterator j, j_end;
                int maxdist = 0;
                for( tie( j, j_end ) = in_edges( *it, graph ); j != j_end; ++j ) {
                    maxdist = std::max( edge_count[source(*j, graph)], maxdist);
                    edge_count[*it] = maxdist + 1;
                }
                group[edge_count.at( *it )].push_back( nodes.at( *it ) );
                _TRACE("-> " << *it << " - " << edge_count[*it] << " - " << nodes.at(*it)->name() );
            }
        }
        // populate runorder queue
        std::map<int, std::vector<JobPtr> >::iterator mit=group.begin();
        for ( ; mit != group.end(); mit++ )
            jq.push_back( mit->second );
        return jq;
    }

    void
    JobManager::job_queue_run( JobQueue & jobs ) {
        std::map<JobPtr, std::pair<time_t, int> > timer;
    
        // process agents in batches run in parallel
        for( unsigned int i=0; i < jobs.size(); i++ ) {
            std::queue<JobResponse> jq;
            std::vector<JobPtr> batch = jobs[i];

            // dispatch agent tasks asychronously
            for( unsigned int j=0; j < batch.size(); j++ ) {
                JobPtr job = batch[j];
                // determine timeout value for this job
                int timeout = job->timeout();
                if ( timeout ) {
                    timer[job] = std::pair<time_t, int>(time(NULL), timeout );
                }
                jq.push( job->run() );
            }

            // poll async tasks until all have completed, or there is
            // a failure, timeout, etc.
            while ( ! jq.empty() ) {
                JobResponse & resp = jq.front();
                _DEBUG("Polling '" << resp.job()->name() << "' ... ");
                resp.status()->sync();
                _DEBUG("Progress: " << resp.status()->progress());
                if ( resp.status()->progress() != 100 ) {
                    // if this job has a timeout, check it
                    if ( timer.count( resp.job() ) ) { 
                        int runtime = time(NULL) - timer[resp.job()].first;
                        if ( timer[resp.job()].second && runtime >= timer[resp.job()].second ) {
                            gbTHROW( ERR_INTERNAL_SERVER_ERROR(
                                "Agent '" + resp.job()->name() + "' exceeded timeout!"
                            ) );
                        }
                    }
                    usleep(10000); // sleep .1 seconds
                }
                else if ( resp.status()->code() != 0 ) {
                    gbTHROW( ERR_INTERNAL_SERVER_ERROR(
                        "Agent '" + resp.job()->name() + "' returned status code " +
                        boost::lexical_cast<std::string>(resp.status()->code())
                    ) );
                }
                else {
                    _DEBUG("Agent '" << resp.job()->name() << "' completed successfully.");
                    jq.pop();                
                }
            } 
        }
    }
    
    void JobManager::requireSchemas(bool b) { 
        JobManager::require_schemas = b;
    }
}
