#ifndef GEARBOX_TRANSIENTSTATUS_IMPL_H_
#define GEARBOX_TRANSIENTSTATUS_IMPL_H_

#include <gearbox/job/StatusImplV1.h>
#include <gearbox/job/StatusCollectionImplV1.h>

#include <deque>

namespace Gearbox {
    class TransientStatusData;
    typedef std::deque< boost::shared_ptr<TransientStatusData> > TransDB;

    class TransientStatusImpl : public StatusImplV1 {
        typedef StatusImplV1 super;
    public:
        TransientStatusImpl(const ConfigFile & cfg);
        TransientStatusImpl(const TransientStatusImpl &);
        virtual ~TransientStatusImpl();
        virtual StatusImpl * clone() const;
        virtual void sync();
        virtual void insert();
        virtual void load();
        virtual const std::string & base_uri() const;
        virtual const char * impltype() const;

        virtual void name( const std::string & name );
        virtual void messages( const std::vector<std::string> & messages );
        virtual void add_message( const std::string & message );
        virtual void children( const std::vector<std::string> & children );
        virtual void add_child( const std::string & child );
        virtual void operation(const std::string & op);
        virtual void resource_uri(const std::string & rsrc_uri);
        virtual void component(const std::string & c);
        virtual void meta(const std::string & key, const Json & value);
        virtual void meta(const Json & meta);
        virtual void progress(unsigned int);
        virtual void code(int c);
        virtual void parent_uri(const std::string & parent);
        virtual void ctime(time_t t);
        virtual void mtime(time_t t);
        virtual void ytime(int64_t t);
        virtual void failures(uint32_t count);
        virtual bool state( Status::State state );

        virtual void   on(Status::Event e, const Job & job);
        virtual JobPtr on(Status::Event e, const JobManager & jm) const;

        using StatusImplV1::concurrency;
        virtual void concurrency(uint32_t count);
        virtual void starting();
        virtual void stopping();

    private:
        TransientStatusImpl & operator=(const TransientStatusImpl &); // unimplemented
        boost::shared_ptr<TransientStatusData> data;
    };

    // Not Implented .. but we might need to for unit testing
    // class TransientStatusCollectionImpl : public StatusCollectionImplV1 {
    //     typedef StatusCollectionImplV1 super;
    // public:
    //     TransientStatusCollectionImpl(const ConfigFile & c);
    //     TransientStatusCollectionImpl( const TransientStatusCollectionImpl & );
    //     ~TransientStatusCollectionImpl();

    //     virtual StatusImpl * pop();
    //     virtual bool empty();
        
    //     virtual void filter_progress(unsigned int min=0, unsigned int max=100);
    //     virtual void filter_code(int min=0, int max=0);
    //     virtual void filter_operation(const std::string & op);
    //     virtual void filter_mtime(time_t min=0, time_t max=UINT_MAX);
    //     virtual void filter_ctime(time_t min=0, time_t max=UINT_MAX);

    // private:
    //     TransientStatusCollectionImpl & operator=(const TransientStatusCollectionImpl &); //unimplemented
    // };
}

#endif
