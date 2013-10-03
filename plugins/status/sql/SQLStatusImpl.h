#ifndef GEARBOX_SQLSTATUS_IMPL_H_
#define GEARBOX_SQLSTATUS_IMPL_H_

#include <gearbox/job/StatusImplV1.h>
#include <gearbox/job/StatusCollectionImplV1.h>

#include <gearbox/store/dbconn.h>

// Status Plugin interface
extern "C" {
    void * sql_status_new(const Gearbox::ConfigFile & cfg, bool collection);
    const char * name();
}

namespace Gearbox {
    class SQLStatusImpl : public StatusImplV1 {
    public:
        typedef StatusImplV1 super;
        SQLStatusImpl(const ConfigFile & cfg);

        SQLStatusImpl(const SQLStatusImpl &);


        virtual StatusImpl* clone() const;

        virtual ~SQLStatusImpl();
        virtual const std::vector<std::string> & messages() const;
        virtual void add_message( const std::string & message );

        virtual const std::vector<std::string> & children() const;
        virtual void add_child( const std::string & child );

        virtual void progress(unsigned int);
        virtual unsigned int progress() const;

        virtual const Json & meta() const;
        virtual void meta( const std::string & key, const Json & value );
        virtual void meta( const Json & meta );

        virtual void parent_uri(const std::string & parent);
        virtual const std::string & parent_uri() const;

        virtual const std::string & operation() const;

        virtual const std::string & resource_uri() const;

        virtual const std::string & component() const;

        virtual void code(int c);
        virtual int code() const;

        virtual time_t ctime() const;

        virtual time_t mtime() const;

        virtual void ytime(int64_t t);
        virtual int64_t ytime() const;

        virtual bool state( Status::State state );
        virtual Status::State state() const;

        virtual void failures(uint32_t count);
        virtual uint32_t failures() const;

        virtual void sync();
        virtual void insert();
        virtual void load();

        virtual void end(int code, Status::State s);
        virtual void fail( int status );
        virtual void cancel();
        virtual void success();

        void   on(Status::Event e, const Job & job);
        JobPtr on(Status::Event e, const JobManager & jm) const;

        virtual void really_load();
        std::string serialize() const;

        virtual uint32_t concurrency() const;
        virtual void starting();
        virtual void stopping();

        virtual const char * impltype() const;
    private:
        SQLStatusImpl & operator=(const SQLStatusImpl &); // unimplemented
        bool messages_loaded;
        bool children_loaded;
        bool meta_loaded;
        bool loaded;
        // has Status been created by StatusManager via fetch() or create()
        bool should_load;
    };

    class SQLStatusCollectionImpl : public StatusCollectionImplV1 {
        typedef StatusCollectionImplV1 super;
    public:
        SQLStatusCollectionImpl(const ConfigFile & c);
        ~SQLStatusCollectionImpl();

        virtual StatusImpl * pop();
        virtual bool empty();

        virtual void filter_progress(unsigned int min=0, unsigned int max=100);
        virtual void filter_code(int min=0, int max=0);
        virtual void filter_operation(const std::string & op);
        virtual void filter_component(const std::string & c);
        virtual void filter_mtime(time_t min=0, time_t max=UINT_MAX);
        virtual void filter_ctime(time_t min=0, time_t max=UINT_MAX);
        virtual void filter_state(const std::string & op);
        virtual void filter_uri(const std::string & op);
        virtual void limit(unsigned int count=0);
    private:
        SQLStatusCollectionImpl( const SQLStatusCollectionImpl & );
        SQLStatusCollectionImpl & operator=(const SQLStatusCollectionImpl &); // unimplemented
        void run_query();
        Gearbox::Database::LoggedStatement lst;
        SQLStatusImpl cursor;
        bool fetched;
        bool executed;
        unsigned int count;
    };
}

#endif
