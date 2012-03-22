#ifndef GEARBOX_STATUS_IMPL_H
#define GEARBOX_STATUS_IMPL_H

#include <vector>
#include <string>
#include <time.h>
#include <gearbox/core/ConfigFile.h>
#include <gearbox/job/Status.h>
#include <gearbox/job/StatusManager.h>

namespace Gearbox {

    class StatusImpl {
    public:
        StatusImpl(const ConfigFile & cfg);
        StatusImpl( const StatusImpl & );

        virtual ~StatusImpl();
        virtual int version() const = 0;
        
        virtual StatusImpl * clone() const = 0;

        virtual const std::vector<std::string> & messages() const;
        virtual void messages( const std::vector<std::string> & messages );
        virtual void add_message( const std::string & message );
        
        virtual const std::vector<std::string> & children() const;
        virtual void children( const std::vector<std::string> & children );
        virtual void add_child( const std::string & child );

        virtual const std::string & name() const;
        virtual void name(const std::string & n);
        
        virtual const std::string & operation() const;
        virtual void operation(const std::string & op);
        
        virtual const std::string & resource_uri() const;
        virtual void resource_uri(const std::string & rsrc_uri);

        virtual const std::string & component() const;
        virtual void component(const std::string & c);

        virtual std::string uri() const;

        virtual const Json & meta() const;
        virtual void meta( const std::string & key, const Json & value );
        virtual void meta( const Json & meta );

        virtual unsigned int progress() const;
        virtual void progress(unsigned int);
        
        virtual void fail( int code );
        virtual void cancel();
        virtual void success();

        virtual void   on(Status::Event e, const Job & job) = 0;
        virtual JobPtr on(Status::Event e, const JobManager & jm) const = 0;

        virtual int code() const;
        virtual void code(int c);

        virtual const std::string & parent_uri() const;
        virtual void parent_uri(const std::string & parent);
        virtual StatusPtr parent() const;

        virtual time_t ctime() const;
        virtual void ctime(time_t t);

        virtual time_t mtime() const;
        virtual void mtime(time_t t);

        virtual int64_t ytime() const;
        virtual void ytime(int64_t t);
        
        virtual void base_uri( const std::string & base_uri );
        virtual const std::string & base_uri() const;

        virtual bool state( Status::State state );
        virtual Status::State state() const;

        virtual void failures(uint32_t count);
        virtual uint32_t failures() const;
        
        virtual void sync() = 0;
        virtual void insert() = 0;
        virtual void load() = 0;
        
        virtual uint32_t concurrency() const;
        virtual void concurrency(uint32_t count);

        virtual void starting() = 0;
        virtual void stopping() = 0;

        virtual const char * impltype() const = 0;

        virtual std::string serialize() const;
        virtual void status_manager( const StatusManager & sm );

    protected:
        const ConfigFile & cfg() const;

    private:
        StatusImpl & operator=(const StatusImpl &);
        class Private;
        Private * impl;
    };
}
#endif
