#include <gearbox/core/strlcpy.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <queue>
#include <gearbox/core/util.h>
namespace bfs=boost::filesystem;
extern "C" {
//////////////////////////////////////////////////////////////////////////////
//
// structs copied from zookeeper.h and zookeeper.jute.h
//
//////////////////////////////////////////////////////////////////////////////
typedef struct {
    int64_t client_id;
    char passwd[16];
} clientid_t;

struct Stat {
    int64_t czxid;
    int64_t mzxid;
    int64_t ctime;
    int64_t mtime;
    int32_t version;
    int32_t cversion;
    int32_t aversion;
    int64_t ephemeralOwner;
    int32_t dataLength;
    int32_t numChildren;
    int64_t pzxid;
};

struct String_vector {
    int32_t count;
    char * *data;
};

enum ZOO_ERRORS {
    ZOK = 0,
    ZNONODE = -101,
    ZNODEEXISTS = -110,
    ZNOTEMPTY = -111
};

extern const int ZOO_CREATED_EVENT;
extern const int ZOO_DELETED_EVENT;
extern const int ZOO_CHANGED_EVENT;
extern const int ZOO_CHILD_EVENT;
extern const int ZOO_SESSION_EVENT;
extern const int ZOO_NOTWATCHING_EVENT;

extern const int ZOO_EXPIRED_SESSION_STATE;
extern const int ZOO_AUTH_FAILED_STATE;
extern const int ZOO_CONNECTING_STATE;
extern const int ZOO_ASSOCIATING_STATE;
extern const int ZOO_CONNECTED_STATE;

extern const int ZOO_SEQUENCE;

//////////////////////////////////////////////////////////////////////////////
//
// typedefs copied from zookeeper.h
//
//////////////////////////////////////////////////////////////////////////////
typedef struct _zhandle zhandle_t;

typedef void (*watcher_fn)(zhandle_t *zh, int type, 
        int state, const char *path,void *watcherCtx);

typedef void (*stat_completion_t)(int rc, const struct Stat *stat,
        const void *data);

typedef void (*data_completion_t)(int rc, const char *value, int value_len,
        const struct Stat *stat, const void *data);

typedef void
        (*string_completion_t)(int rc, const char *value, const void *data);

typedef void (*strings_completion_t)(int rc,
        const struct String_vector *strings, const void *data);

typedef void (*void_completion_t)(int rc, const void *data);

//////////////////////////////////////////////////////////////////////////////
//
// now the sub implementations
//
//////////////////////////////////////////////////////////////////////////////

clientid_t cid;
struct ZooEvent {
    int type;
    int state;
    std::string path;
    bool null_path;
    ZooEvent(int t,int s,const char* p) :type(t), state(s){
        null_path = p ? false : true;
        if( !null_path ) {
            path = p;
        }
    }
};
typedef boost::shared_ptr<ZooEvent> ZooEventPtr;

enum WatchType {
    ANY    = 0,
    CREATE = 1,
    DELETE = 2,
    CHANGE = 4,
    CHILD  = 8   
};

struct Callback {
    watcher_fn fn;
    void * context;
    int type;
    int txn_id;
    Callback(watcher_fn f, void*c, int t, int id) : fn(f), context(c), type(t), txn_id(id) {}
};

typedef boost::shared_ptr<Callback> CallbackPtr;

std::queue<ZooEventPtr> events;

std::map<std::string,CallbackPtr> watchers;

std::map<std::string,int64_t> txnIds;
std::map<std::string,int64_t> versions;

int txn_id = 0;

int count_children( const std::string & path ) {
    int count = 0;
    if( bfs::is_directory(path) ) {
        for (bfs::directory_iterator itr(path); itr!=bfs::directory_iterator(); ++itr) {
            std::string file = itr->path().filename();
            // skip . files in the count
            if( file.substr(0,1) == "." ) {
                continue;
            }
            count++;
        }
    }
    return count;
}

void addEvent( const std::string & path, int type, ZooEventPtr evt) {
    std::map<std::string,CallbackPtr>::iterator end = watchers.end();
    std::map<std::string,CallbackPtr>::iterator item;
    // watcher exists
    if( (item = watchers.find(path)) != end ) {
        // and it watcher was set previous to current txn
        if( txn_id > item->second->txn_id ) {
            // and watchers is looking for this type 
            if( item->second->type & type ) {
                events.push(evt);
                return;
            }
        }
    }
}

void addCreateEvent(const std::string & path) {
    addEvent(path, CREATE, ZooEventPtr( new ZooEvent(ZOO_CREATED_EVENT,ZOO_CONNECTED_STATE, path.c_str()) ) );
    bfs::path p(path);
    std::string parent = p.parent_path().string();
    addEvent(parent, CHILD, ZooEventPtr( new ZooEvent(ZOO_CHILD_EVENT,ZOO_CONNECTED_STATE, parent.c_str()) ) );
}
void addDeleteEvent(const std::string & path) {
    addEvent(path, DELETE, ZooEventPtr( new ZooEvent(ZOO_DELETED_EVENT,ZOO_CONNECTED_STATE, path.c_str()) ) );
    bfs::path p(path);
    std::string parent = p.parent_path().string();
    addEvent(parent, CHILD, ZooEventPtr( new ZooEvent(ZOO_CHILD_EVENT,ZOO_CONNECTED_STATE, parent.c_str()) ) );
}

void addUpdateEvent(const std::string & path) {
    addEvent(path, CHANGE, ZooEventPtr( new ZooEvent(ZOO_CHANGED_EVENT,ZOO_CONNECTED_STATE, path.c_str()) ) );
}

zhandle_t *zookeeper_init(
    const char *host, watcher_fn fn,
    int recv_timeout, const clientid_t *clientid, void *context, int flags) {
    cid.client_id = 1234;
    strlcpy(cid.passwd,"password",16);
    events.push( ZooEventPtr(new ZooEvent(ZOO_SESSION_EVENT,ZOO_CONNECTED_STATE,NULL)));
    watchers[""] = CallbackPtr( new Callback(fn,context,ANY,txn_id) );
    return (zhandle_t*)1;
}

const clientid_t *zoo_client_id(zhandle_t *zh) {
    return &cid;
}

int zookeeper_close(zhandle_t *zh) {
    return ZOK;
}

int zoo_aexists(
    zhandle_t *zh, const char *path, int watch, 
    stat_completion_t completion, const void *data) {
    
    if( watch ) {
        // use global watcher
        CallbackPtr global = watchers[""];
        watchers[path] = CallbackPtr( new Callback( global->fn,global->context, CREATE | DELETE | CHANGE, txn_id ) );
    }

    std::string p("./.zookeeper");
    p += path;
    if( bfs::exists(p) ) {
        Stat s;
        s.numChildren = count_children(p);
        if( bfs::exists( p + "/.content" ) ) {
            s.dataLength = bfs::file_size(p + "/.content");
        }
        else {
            s.dataLength = 0;
        }
        s.mzxid = txnIds[path];
        s.version = versions[path];
        s.mtime = time(NULL) * 1000;
        s.ctime = time(NULL) * 1000;
        completion(ZOK, &s, data);
    }
    else {
        completion(ZNONODE, NULL,data);
    }
    
    return ZOK;
}

int zoo_awexists(
    zhandle_t *zh, const char *path, 
    watcher_fn watcher, void* watcherCtx, 
    stat_completion_t completion, const void *data) {
    watchers[path] = CallbackPtr( new Callback(watcher,watcherCtx, CREATE | DELETE | CHANGE, txn_id ) );    
    return zoo_aexists(zh,path,false,completion,data);
}


// get => delete | changed
int zoo_aget(
    zhandle_t *zh, const char *path, int watch, 
    data_completion_t completion, const void *data ) {

    if( watch ) {
        // use global watcher
        CallbackPtr global = watchers[""];
        watchers[path] = CallbackPtr( new Callback( global->fn,global->context, DELETE | CHANGE, txn_id ) );
    }

    std::string p("./.zookeeper");
    p += path;
    if( bfs::exists(p) ) {
        Stat s;
        s.numChildren = count_children(p);
        std::string contents;
        p += "/.content";
        if( bfs::exists(p) ) {
            s.dataLength = bfs::file_size(p);
            contents = slurp(p);
        }
        else {
            s.dataLength = 0;
        }
        s.mzxid = txnIds[path];
        s.version = versions[path];
        s.mtime = time(NULL) * 1000;
        s.ctime = time(NULL) * 1000;
        completion(ZOK, contents.data(), contents.size(), &s, data);
    }
    else {
        completion(ZNONODE, NULL, 0, NULL, data);
    }

    return ZOK;
}

int zoo_awget(zhandle_t *zh, const char *path, 
        watcher_fn watcher, void* watcherCtx, 
              data_completion_t completion, const void *data) {
    watchers[path] = CallbackPtr( new Callback(watcher,watcherCtx, DELETE | CHANGE, txn_id ) );    
    return zoo_aget(zh,path,false,completion,data);
}

int zoo_aset(zhandle_t *zh, const char *path, const char *buffer, int buflen, 
             int version, stat_completion_t completion, const void *data) {
    std::string p("./.zookeeper");
    p += path;
    if( bfs::exists(p) ) {
        Stat s;
        s.numChildren = count_children(p);

        p += "/.content";
        std::ofstream file(p.c_str());
        std::string content(buffer,buflen);
        file << content;
        file.close();
        
        txn_id++;
        addUpdateEvent(path);

        s.dataLength = bfs::file_size(p);
        txnIds[path] = txn_id;
        versions[path]++;
        s.mzxid = txnIds[path];
        s.version = versions[path];
        s.mtime = time(NULL) * 1000;
        s.ctime = time(NULL) * 1000;
        completion(ZOK, &s, data);
    }
    else {
        completion(ZNONODE, NULL, data);
    }
    return ZOK;
}

int zoo_acreate(
    zhandle_t *zh, const char *path, const char *value, 
    int valuelen, const struct ACL_vector *acl, int flags,
    string_completion_t completion, const void *data) {

    std::string p("./.zookeeper");
    p += path;
    if( bfs::exists(p) ) {
        completion(ZNODEEXISTS, NULL, data);
    } else {
        bfs::path bp(p);
        if( !bfs::exists( bp.parent_path() ) ) {
            // parent does not exist
            completion(ZNONODE, NULL, data);
            return ZOK;
        }
        
        txn_id++;
        addCreateEvent(path);
        txnIds[path] = txn_id;
        versions[path] = 0;
        
        std::string zoopath = path;
        if( flags & ZOO_SEQUENCE ) {
            std::ostringstream oss;
            oss << path << std::setfill('0') << std::setw(10) << count_children(bp.parent_path().string()) + 2;
            p = "./.zookeeper/" + oss.str();
            zoopath = oss.str();
        }
        bfs::create_directory(p);
        if( valuelen ) {
            p += "/.content";
            std::ofstream file(p.c_str());
            std::string content(value,valuelen);
            file << content;
            file.close();
        }
        completion(ZOK, zoopath.c_str(), data);
    }
    
    return ZOK;
}

int zoo_adelete(zhandle_t *zh, const char *path, int version, 
                void_completion_t completion, const void *data) {
    std::string p("./.zookeeper");
    p += path;
    if( bfs::exists(p) ) {
        int children = count_children(p);
        if( children ) {
            completion(ZNOTEMPTY,data);
        }
        
        txn_id++;
        versions[path] = -1;
        addDeleteEvent(path);
        
        bfs::remove_all(p);
        completion(ZOK,data);
    }
    else {
        completion(ZNONODE,data);
    }
    return ZOK;
}

int zoo_aget_children(
    zhandle_t *zh, const char *path, int watch, 
    strings_completion_t completion, const void *data) {
    if( watch ) {
        // use global watcher
        CallbackPtr global = watchers[""];
        watchers[path] = CallbackPtr( new Callback( global->fn,global->context, CHILD, txn_id ) );
    }

    std::string p("./.zookeeper");
    p += path;
    if( bfs::exists(p) ) {
        std::vector<std::string> dirs;
        if( bfs::is_directory(p) ) {
            for (bfs::directory_iterator itr(p); itr!=bfs::directory_iterator(); ++itr) {
                std::string file = itr->path().filename();
                // skip . files in the count
                if( file.substr(0,1) == "." ) {
                    continue;
                }
                dirs.push_back(file);
            }
        }
        String_vector svec;
        if(dirs.size()) {
            svec.data = new char*[dirs.size()];
            for( unsigned int i=0; i< dirs.size(); i++ ) {
                // this will only work if dirs outlives
                // svec ... which it should in this stub
                svec.data[i] = const_cast<char*>(dirs[i].c_str());
            }
            svec.count = dirs.size();
        }
        else {
            svec.data = NULL;
            svec.count = 0;
        }
        completion(ZOK, &svec, data);
        if( svec.data != NULL ) {
            delete [] svec.data;
        }
    }
    else {
        completion(ZNONODE, NULL, data);
    }
    
    return ZOK;
}

int zoo_awget_children(zhandle_t *zh, const char *path,
        watcher_fn watcher, void* watcherCtx, 
        strings_completion_t completion, const void *data) {

    watchers[path] = CallbackPtr( new Callback(watcher,watcherCtx, CHILD, txn_id ) );    
    return zoo_aget_children(zh,path,false,completion,data);
}

int select(
    int nfds, fd_set *readfds,
    fd_set *writefds, fd_set *errorfds,
    struct timeval *timeout) {
    // need to fake out select for the yield() process to not throw
    return 1;
}

int zookeeper_interest(zhandle_t *zh, int *fd, int *interest, 
                       struct timeval *tv) {
    *fd=5;
    *interest=5;
    memset(tv,0,sizeof(*tv));
    return ZOK;
}

int zookeeper_process(zhandle_t *zh, int evt) {
    while( !events.empty() ) {
        ZooEventPtr event = events.front();
        events.pop();
        int mask = ANY;
        if( event->type == ZOO_CREATED_EVENT ) {
            mask = CREATE;
        }
        else if( event->type == ZOO_DELETED_EVENT ) {
            mask = DELETE;
        }
        else if( event->type == ZOO_CHANGED_EVENT ) {
            mask = CHANGE;
        }
        else if( event->type == ZOO_CHILD_EVENT ) {
            mask = CHILD;
        }
        
        std::map<std::string,CallbackPtr>::iterator end = watchers.end();
        std::map<std::string,CallbackPtr>::iterator item;
        if( ! event->null_path ) {
            if( (item = watchers.find(event->path)) != end ) {
                if( item->second->type & mask ) {
                    // found the event, fire it!
                    item->second->fn(zh,event->type,event->state, event->path.c_str(), item->second->context);
                    // remove watcher
                    watchers.erase(item);
                    continue;
                }
            }
        }
        if( (item = watchers.find("")) != end ) {
            // found global item, so fire it!
            const char * path = NULL;
            if( !event->null_path ) {
                path = event->path.c_str();
            }
            item->second->fn(zh,event->type,event->state,path, item->second->context);
            continue;
        }
    }
    return ZOK;
}

} // extern "C"
