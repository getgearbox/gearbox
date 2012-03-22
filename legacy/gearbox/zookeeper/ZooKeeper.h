#ifndef GEARBOX_ZOOKEEPER_H
#define GEARBOX_ZOOKEEPER_H
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <gearbox/core/Errors.h>

namespace Gearbox {
    class ZooKeeper {
        class Private; // forward declare;
    public:

        class Stat {
        public:
            int32_t version();
            int64_t modTxId();
            int32_t dataLength();
            int32_t numChildren();
            int64_t mtime();
            int64_t ctime();
            Stat(const Stat &);
            ~Stat();
            Stat();
            Stat operator=(const Stat &);
        private:
            class Private; // foward declare;
            friend class ZooKeeper::Private;
            Private * impl;
        };
        
        enum NodeType {
            TYPE_PLAIN = 0,
            TYPE_EPHEMERAL,
            TYPE_SEQUENCE,
            TYPE_EPHEM_SEQ
        };
    
        enum EventType { 
            EVENT_UNKNOWN = 0,
            EVENT_CREATED,
            EVENT_DELETED,
            EVENT_CHANGED,
            EVENT_CHILD,
            EVENT_NOTWATCHING
        };

        static std::string eventTypeToString(EventType e);

        struct Event {
            EventType type;
            std::string path;
        };
        
        typedef boost::shared_ptr<Event> EventPtr;

        typedef void (*WatchCallback)(
            ZooKeeper & zook,
            const std::string & path,
            EventType type,
            void * context
        );

        ZooKeeper( const std::string & connection, int timeout = 10000 );
        ~ZooKeeper();

        bool exists(
            const std::string & path,
            bool watch = false
        );

        bool exists(
            const std::string & path,
            WatchCallback callback,
            void * context
        );

        Stat stat();
        Stat stat( const std::string & path );
        
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
            NodeType type = TYPE_PLAIN
        );
        
        // first arg supports const only if
        // not NodeType & TYPE_SEQUENCE
        void create(
            const std::string & path,
            const std::string & data,
            NodeType type = TYPE_PLAIN
        );

        // recursively make nodes, with empty node values
        void createFullPath(const std::string & path);

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

        void yield();

        unsigned int countEvents();
        EventPtr nextEvent();

        static std::string join_paths(const std::string& path1, const std::string& path2);
        static std::string normalize_path(const std::string& path);

    private:
        // Copy Constructor private
        ZooKeeper( const ZooKeeper &);
        // Copy Assignment private
        const ZooKeeper & operator=(const ZooKeeper &);

        Private * impl;
    };

DEFINE_ERROR(ZSYSTEMERROR,1001);
DEFINE_ERROR(ZRUNTIMEINCONSISTENCY,1002); /*!< A runtime inconsistency was found */
DEFINE_ERROR(ZDATAINCONSISTENCY,1003); /*!< A data inconsistency was found */
DEFINE_ERROR(ZCONNECTIONLOSS,1004); /*!< Connection to the server has been lost */
DEFINE_ERROR(ZMARSHALLINGERROR,1005); /*!< Error while marshalling or unmarshalling data */
DEFINE_ERROR(ZUNIMPLEMENTED,1006); /*!< Operation is unimplemented */
DEFINE_ERROR(ZOPERATIONTIMEOUT,1007); /*!< Operation timeout */
DEFINE_ERROR(ZBADARGUMENTS,1008); /*!< Invalid arguments */
DEFINE_ERROR(ZINVALIDSTATE,1009); /*!< Invliad zhandle state */

DEFINE_ERROR(ZAPIERROR,1100);
DEFINE_ERROR(ZNONODE,1101); /*!< Node does not exist */
DEFINE_ERROR(ZNOAUTH,1102); /*!< Not authenticated */
DEFINE_ERROR(ZBADVERSION,1103); /*!< Version conflict */
DEFINE_ERROR(ZNOCHILDRENFOREPHEMERALS,1108); /*!< Ephemeral nodes may not have children */
DEFINE_ERROR(ZNODEEXISTS,1110); /*!< The node already exists */
DEFINE_ERROR(ZNOTEMPTY,1111); /*!< The node has children */
DEFINE_ERROR(ZSESSIONEXPIRED,1112); /*!< The session has been expired by the server */
DEFINE_ERROR(ZINVALIDCALLBACK,1113); /*!< Invalid callback specified */
DEFINE_ERROR(ZINVALIDACL,1114); /*!< Invalid ACL specified */
DEFINE_ERROR(ZAUTHFAILED,1115); /*!< Client authentication failed */
DEFINE_ERROR(ZCLOSING,1116); /*!< ZooKeeper is closing */
DEFINE_ERROR(ZNOTHING,1117); /*!< (not error) no server responses to process */
DEFINE_ERROR(ZSESSIONMOVED,1118); /*!<session moved to another server, so operation is ignored */ 

} // namespace

#endif // GEARBOX_ZOOKEEPER_H
