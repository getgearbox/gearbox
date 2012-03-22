#ifndef GEARBOX_ZOOKEEPER_PRIVATE_H
#define GEARBOX_ZOOKEEPER_PRIVATE_H
#include <gearbox/zookeeper/ZooKeeper.h>
#include <gearbox/scoreboard/Scoreboard.h>

#include <zookeeper/zookeeper.h>
#include <list>

namespace Gearbox {
    struct SyncContext {
        std::string name;
        int rc;
        bool done;
        SyncContext (const std::string & n): name(n), rc(0), done(false), sw() {}
        virtual ~SyncContext() {}
    };
    
    struct StatContext : public SyncContext {
        ::Stat & stat;
        StatContext(const std::string & n, ::Stat & s) : SyncContext(n), stat(s) {};
    };
    
    struct DataContext : public StatContext {
        std::string & data;
        DataContext(const std::string & n, ::Stat & stat, std::string & d) : StatContext(n, stat), data(d) {}
    };
    
    struct PathContext : public SyncContext {
        std::string & path;
        PathContext(const std::string & n, std::string & p) : SyncContext(n), path(p) {}
    };

    struct ChildrenContext : public SyncContext {
        std::vector<std::string> & children;
        ChildrenContext(const std::string & n, std::vector<std::string> & c): SyncContext(n), children(c) {}
    };

    class ZooKeeper::Private {
    public:
        Private( const std::string & connection, int timeout, ZooKeeper *owner );
        ~Private();
        
        void connect();

        void sync(SyncContext & sync);

        bool exists(
            const std::string & path,
            bool watch = false
        );
        
        bool exists(
            const std::string & path,
            WatchCallback callback,
            void * context
        );

        ZooKeeper::Stat stat();
        ZooKeeper::Stat stat(const std::string & path);
    
        void get(
            const std::string & path,
            std::string & data,
            bool watch = false
        );

        void get(
            const std::string & path,
            std::string & data,
            WatchCallback callback,
            void * context
        );

        void set(
            const std::string & path,
            const std::string & data,
            int32_t version = -1
        );

        void create(
            std::string & path, // may be updated if sequential node
            const std::string & data,
            ZooKeeper::NodeType type
        );
        
        void create(
            const std::string & path,
            const std::string & data,
            ZooKeeper::NodeType type
        );

        void del(
            const std::string & path,
            int32_t version = -1
        );
    
        void children(
            const std::string & path,
            std::vector<std::string> & entries,
            bool watch = false
        );

        void children(
            const std::string & path,
            std::vector<std::string> & entries,
            WatchCallback callback,
            void * context
        );

        void yield( int timeout );
        void yield( struct timeval * tv = NULL );

        unsigned int countEvents();
        ZooKeeper::EventPtr nextEvent();

    private:
        zhandle_t * zk_;
        std::list<ZooKeeper::EventPtr> events_;
        bool connected_;
        int timeout_;
        const std::string connection_;
        const clientid_t * client_id_;
        ZooKeeper::Stat stat_;
        ZooKeeper * owner_;

        static void Watcher( zhandle_t *, int type, int state, const char *path, void* ctx );
    };
}

#endif // GEARBOX_ZOOKEEPER_PRIVATE_H
