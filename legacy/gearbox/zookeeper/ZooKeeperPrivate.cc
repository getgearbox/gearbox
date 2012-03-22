// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/zookeeper/ZooKeeperPrivate.h>
#include <gearbox/zookeeper/ZooKeeperStatPrivate.h>
#include <gearbox/core/Errors.h>

#define LOGCAT "gearbox.zookeeper"
#include <gearbox/core/logger.h>

using namespace Gearbox;

static ZooKeeper::EventType eventTypeConvert(int type) {
    return 
        type == ZOO_CREATED_EVENT     ? ZooKeeper::EVENT_CREATED     :
        type == ZOO_DELETED_EVENT     ? ZooKeeper::EVENT_DELETED     :
        type == ZOO_CHANGED_EVENT     ? ZooKeeper::EVENT_CHANGED     :
        type == ZOO_CHILD_EVENT       ? ZooKeeper::EVENT_CHILD       :
        type == ZOO_NOTWATCHING_EVENT ? ZooKeeper::EVENT_NOTWATCHING :
                                        ZooKeeper::EVENT_UNKNOWN;
}

void throw_from_zkcode(int code, const std::string & msg) {
    switch((-1 * code) + 1000) {
    case 1001: throw ERR_CODE_1001(msg + ": " + zerror(code));
    case 1002: throw ERR_CODE_1002(msg + ": " + zerror(code));
    case 1003: throw ERR_CODE_1003(msg + ": " + zerror(code));
    case 1004: throw ERR_CODE_1004(msg + ": " + zerror(code));
    case 1005: throw ERR_CODE_1005(msg + ": " + zerror(code));
    case 1006: throw ERR_CODE_1006(msg + ": " + zerror(code));
    case 1007: throw ERR_CODE_1007(msg + ": " + zerror(code));
    case 1008: throw ERR_CODE_1008(msg + ": " + zerror(code));
    case 1009: throw ERR_CODE_1009(msg + ": " + zerror(code));
        
    case 1100: throw ERR_CODE_1100(msg + ": " + zerror(code));
    case 1101: throw ERR_CODE_1101(msg + ": " + zerror(code));
    case 1102: throw ERR_CODE_1102(msg + ": " + zerror(code));
    case 1103: throw ERR_CODE_1103(msg + ": " + zerror(code));
    case 1108: throw ERR_CODE_1108(msg + ": " + zerror(code));
    case 1110: throw ERR_CODE_1110(msg + ": " + zerror(code));
    case 1111: throw ERR_CODE_1111(msg + ": " + zerror(code));
    case 1112: throw ERR_CODE_1112(msg + ": " + zerror(code));
    case 1113: throw ERR_CODE_1113(msg + ": " + zerror(code));
    case 1114: throw ERR_CODE_1114(msg + ": " + zerror(code));
    case 1115: throw ERR_CODE_1115(msg + ": " + zerror(code));
    case 1116: throw ERR_CODE_1116(msg + ": " + zerror(code));
    case 1117: throw ERR_CODE_1117(msg + ": " + zerror(code));
    case 1118: throw ERR_CODE_1118(msg + ": " + zerror(code));
    }
    throw ERR_ZSYSTEMERROR(msg + ": " + zerror(code));
}

struct TrampolineContext {
    ZooKeeper * zook;
    ZooKeeper::WatchCallback callback;
    void * context;
    TrampolineContext(ZooKeeper * zk, ZooKeeper::WatchCallback cb, void * ctxt) 
        : zook(zk), callback(cb), context(ctxt) {}
};

static void trampoline(
    zhandle_t *zh,
    int type,
    int state,
    const char * path,
    void * watcherCtx) {

    // Tree watchers will be triggered for session events as well as for tree
    // events.  However, when invoked for a session event, the context is not a
    // single use TrampolineContext object and we must not treat it as such or
    // delete it.

    if (type == ZOO_SESSION_EVENT)
        return;

    TrampolineContext * tCtx = (TrampolineContext*)watcherCtx;
    
    // void (*WatchCallback)(
    //      ZooKeeper & zook,
    //      const std::string & path,
    //      EventType type,
    //      void * context
    // );

    try {
        (*(tCtx->callback))(
            *(tCtx->zook),
            std::string(path),
            eventTypeConvert(type),
            tCtx->context
        );
    }
    catch ( const std::exception & e ) {
        _ERROR( "Caught Exception from user watcher on path " << path << ": " << e.what() );
    }
    catch ( ... ) {
        _ERROR( "Caught Unknown Exception from user watcher on path " << path);
    }

    delete tCtx;
}

static const char * toStateStr(int state) {
    return state == ZOO_EXPIRED_SESSION_STATE ? "ZOO_EXPIRED_SESSION_STATE" :
           state == ZOO_AUTH_FAILED_STATE     ? "ZOO_AUTH_FAILED_STATE"     :
           state == ZOO_CONNECTING_STATE      ? "ZOO_CONNECTING_STATE"      :
           state == ZOO_ASSOCIATING_STATE     ? "ZOO_ASSOCIATING_STATE"     :
           state == ZOO_CONNECTED_STATE       ? "ZOO_CONNECTED_STATE"       :
           "UNKNOWN_STATE";
}


void ZooKeeper::Private::Watcher( zhandle_t *, int type, int state, const char *path, void* ctx ) {
    ZooKeeper::Private * obj = (ZooKeeper::Private*) ctx;

    if (state == ZOO_CONNECTED_STATE) {
        obj->connected_ = true;
    } else {
        obj->connected_ = false;
    }

    if( type == ZOO_SESSION_EVENT ) {
        _TRACE("got SESSION_EVENT with state: " << toStateStr(state) << " on path " << path);
        if( state == ZOO_EXPIRED_SESSION_STATE ) {
            // expired state means the server gave up on us, so our session is no
            // longer valid. Reset the client_id and try to reconnect
            if( obj->zk_ ) {
                zookeeper_close(obj->zk_);
                obj->zk_ = NULL;
                obj->client_id_ = NULL;
                obj->connect();
            }
        }
    }
    else {
        ZooKeeper::EventPtr evt(new ZooKeeper::Event());
        evt->path = path;
        evt->type = eventTypeConvert(type);
        _DEBUG( "Queueing Event " << ZooKeeper::eventTypeToString(evt->type) << " for path " << path );
        obj->events_.push_back(evt);
    }
}

void ZooKeeper::Private::connect() {
    if( this->connected_ ) return;
    
    try {
        this->zk_ = zookeeper_init(
            connection_.c_str(),
            Watcher,
            timeout_, // recv_timeout undocumented?
            this->client_id_,
            this,
            0
        );

        time_t expires = time(0) + (timeout_/1000);
        while(!connected_ && time(0) < expires) {
            this->yield();
        }
    }

    if( !connected_ ) {
        throw ERR_INTERNAL_SERVER_ERROR("Failed to connect to ZooKeeper instance on: " + connection_);
    }

    client_id_ = zoo_client_id(zk_);
}

ZooKeeper::Private::Private( const std::string & connection, int timeout, ZooKeeper * owner )
    : zk_(NULL), 
      connected_(false), 
      timeout_(timeout), 
      connection_(connection), 
      client_id_(NULL),
      owner_(owner) {
    this->connect();
}

ZooKeeper::Private::~Private() {
    if( this->zk_ ) {
        zookeeper_close(this->zk_);
    }
}

static void SyncCallback(int rc, const void * data) {
    SyncContext * ctx = (SyncContext*)data;
    ctx->rc = rc;
    ctx->done = true;
}

static void StatCallback(int rc, const ::Stat * stat, const void * data) {
    StatContext * ctx = (StatContext*)data;
    if( stat ) {
        ctx->stat = *stat;
    }
    SyncCallback(rc, data);
}

static void DataCallback(int rc, const char * value, int value_len, const ::Stat * stat, const void * data) {
    DataContext * ctx = (DataContext*)data;
    if( value ) {
        ctx->data.assign(value, value_len);
    }
    StatCallback(rc,stat,data);
}

static void PathCallback(int rc, const char * value, const void * data) {
    PathContext * ctx = (PathContext*)data;
    if( value ) {
        ctx->path.assign(value);
    }
    SyncCallback(rc,data);
}

static void ChildrenCallback(int rc, const struct String_vector *strings, const void *data) {
    ChildrenContext * ctx = (ChildrenContext*)data;
    if( strings ) {
        if( strings->count ) { 
            for (int i = 0; i < strings->count; i++ ) {
                ctx->children.push_back( strings->data[i] );
            }
        }
    }
    SyncCallback(rc,data);
}


void ZooKeeper::Private::sync(SyncContext & ctx) {
    while(1) {
        this->yield();
        if( ctx.done ) {
            break;
        }
    }
}

bool ZooKeeper::Private::exists(
    const std::string & path,
    bool watch
) {
    std::string npath = normalize_path(path);
    this->connect();
    StatContext ctx("exists", this->stat_.impl->stat_);
    int zc = zoo_aexists(this->zk_, npath.c_str(), watch, StatCallback, &ctx);
    if( !zc == ZOK ) {
        throw_from_zkcode(zc, "zoo_aexists");
    }
    this->sync(ctx);
    return ctx.rc == ZOK ? true : false;
}

bool ZooKeeper::Private::exists(
    const std::string & path,
    WatchCallback callback,
    void * context
) {
    std::string npath = normalize_path(path);
    this->connect();
    StatContext ctx("exists", this->stat_.impl->stat_);
    int zc = zoo_awexists(
        this->zk_,
        npath.c_str(),
        trampoline, 
        new TrampolineContext(this->owner_, callback, context),
        StatCallback,
        &ctx
    );
    if( !zc == ZOK ) {
        throw_from_zkcode(zc, "zoo_awexists");
    }
    this->sync(ctx);
    return ctx.rc == ZOK ? true : false;
}

ZooKeeper::Stat ZooKeeper::Private::stat() {
    return this->stat_;
}

ZooKeeper::Stat ZooKeeper::Private::stat( const std::string & path ) {
    this->exists(path);
    return this->stat_;
}
    
void ZooKeeper::Private::get(
    const std::string & path,
    std::string & data,
    bool watch
) {
    std::string npath = normalize_path(path);
    this->connect();

    if( this->exists(npath) ) {
        int len = stat_.dataLength();
        data.resize(len);
        DataContext ctx("get", this->stat_.impl->stat_, data);
        int zc = zoo_aget(this->zk_, npath.c_str(), watch, DataCallback, &ctx);
        if( !zc == ZOK ) {
            throw_from_zkcode(zc, "zoo_aget");
        }
        this->sync(ctx);
        if( ctx.rc == ZOK ) return;
        throw_from_zkcode(ctx.rc, "failed to get " + npath);
    }
    throw_from_zkcode(ZNONODE, "failed to get " + npath);
}

void ZooKeeper::Private::get(
    const std::string & path,
    std::string & data,
    WatchCallback callback,
    void * context
) {
    std::string npath = normalize_path(path);
    this->connect();

    if( this->exists(npath) ) {
        int len = stat_.dataLength();
        data.resize(len);
        DataContext ctx("get", this->stat_.impl->stat_, data);
        int zc = zoo_awget(
            this->zk_, 
            npath.c_str(), 
            trampoline, 
            new TrampolineContext(this->owner_, callback, context),
            DataCallback,
            &ctx
        );
        if( !zc == ZOK ) {
            throw_from_zkcode(zc, "zoo_awget");
        }
        this->sync(ctx);
        if( ctx.rc == ZOK ) return;
        throw_from_zkcode(ctx.rc, "failed to get " + npath);
    }
    throw_from_zkcode(ZNONODE, "failed to get " + npath);
}

void ZooKeeper::Private::set(
    const std::string & path,
    const std::string & data,
    int32_t version
) {
    std::string npath = normalize_path(path);
    this->connect();
    StatContext ctx("set", this->stat_.impl->stat_);
    int zc = zoo_aset(this->zk_, npath.c_str(), data.data(), data.size(), version, StatCallback, &ctx);
    if( zc != ZOK ) {
        throw_from_zkcode(zc, "zoo_aset");
    }
    this->sync(ctx);
    if( ctx.rc == ZOK ) return;
    throw_from_zkcode(ctx.rc, "failed to set " + npath);
}

static int flagsFromNodeType(ZooKeeper::NodeType type) {
    switch(type) {
    case ZooKeeper::TYPE_EPHEMERAL: return ZOO_EPHEMERAL;
    case ZooKeeper::TYPE_SEQUENCE: return ZOO_SEQUENCE;
    case ZooKeeper::TYPE_EPHEM_SEQ: return ZOO_EPHEMERAL | ZOO_SEQUENCE;
    case ZooKeeper::TYPE_PLAIN:
    default: return 0;
    }
}

void ZooKeeper::Private::create(
    std::string & path, // may be updated if sequential node
    const std::string & data,
    ZooKeeper::NodeType type
) {
    std::string npath = normalize_path(path);
    this->connect();
    
    std::string newpath;
    PathContext ctx("create", newpath);
    int zc = zoo_acreate(
        this->zk_,
        npath.c_str(),
        data.data(),
        data.size(),
        &ZOO_OPEN_ACL_UNSAFE, 
        flagsFromNodeType(type),
        PathCallback,
        &ctx
    );
    if( zc != ZOK ) {
        throw_from_zkcode(zc, "zoo_acreate");
    }
    this->sync(ctx);
    if( ctx.rc == ZOK ) {
        if( type & TYPE_SEQUENCE ) {
            path.assign(newpath);
        }
        return;
    }
    throw_from_zkcode(ctx.rc, "failed to create " + npath);
}

void ZooKeeper::Private::create(
    const std::string & path, // may be updated if sequential node
    const std::string & data,
    ZooKeeper::NodeType type
) {
    if( type & TYPE_SEQUENCE ) {
        _WARN("ZooKeeper::create called with TYPE_SEQUENCE but path arg is const");
        _WARN("path cannot be updated for sequence");
    }
    std::string npath = normalize_path(path);
    this->connect();
    std::string newpath;
    PathContext ctx("create", newpath);
    int zc = zoo_acreate(
        this->zk_,
        npath.c_str(),
        data.data(),
        data.size(),
        &ZOO_OPEN_ACL_UNSAFE, 
        flagsFromNodeType(type),
        PathCallback,
        &ctx
    );
    if( zc != ZOK ) {
        throw_from_zkcode(zc, "zoo_acreate");
    }
    this->sync(ctx);
    if( ctx.rc == ZOK ) return;
    throw_from_zkcode(ctx.rc, "failed to create " + npath);
}

void ZooKeeper::Private::del(
    const std::string & path,
    int32_t version
) {
    std::string npath = normalize_path(path);
    this->connect();
    SyncContext ctx("delete");
    int zc = zoo_adelete(this->zk_, npath.c_str(), version, SyncCallback, &ctx);
    if( zc != ZOK ) {
        throw_from_zkcode(zc, "zoo_adelete");
    }
    this->sync(ctx);
    if( ctx.rc == ZOK ) return;
    throw_from_zkcode(ctx.rc, "failed to delete " + npath);
}

void ZooKeeper::Private::children(
    const std::string & path,
    std::vector<std::string> & entries,
    bool watch
) {
    std::string npath = normalize_path(path);
    this->connect();
    entries.clear();
    ChildrenContext ctx("get_children", entries);
    int zc = zoo_aget_children(this->zk_, npath.c_str(), watch, ChildrenCallback, &ctx);
    if( zc != ZOK ) {
        throw_from_zkcode(zc, "zoo_aget_children");
    }
    this->sync(ctx);
    if( ctx.rc == ZOK ) return;
    throw_from_zkcode(ctx.rc, "failed to get children for " + npath);
}

void ZooKeeper::Private::children(
    const std::string & path,
    std::vector<std::string> & entries,
    WatchCallback callback,
    void * context
) {
    std::string npath = normalize_path(path);
    this->connect();
    entries.clear();
    ChildrenContext ctx("get_children", entries);
    int zc = zoo_awget_children(
        this->zk_, 
        npath.c_str(), 
        trampoline, 
        new TrampolineContext(this->owner_, callback, context),
        ChildrenCallback,
        &ctx
    );

    if( zc != ZOK ) {
        throw_from_zkcode(zc, "zoo_awget_children");
    }
    this->sync(ctx);
    if( ctx.rc == ZOK ) return;
    throw_from_zkcode(ctx.rc, "failed to get children for " + npath);
}

void ZooKeeper::Private::yield( int timeout ) {
    time_t expires = time(0) + timeout;
    time_t timeLeft = timeout;
    struct timeval tv;
    while(timeLeft >= 0) {
        this->yield(&tv);
        if (tv.tv_sec > timeLeft) {
            tv.tv_sec = timeLeft;
        }
        timeLeft = expires - time(0);
    }
}

void ZooKeeper::Private::yield( struct timeval * tv ) {
    int fd = 0;
    int interest = 0;
    struct timeval TV;
    if(!tv) tv = &TV;

    zookeeper_interest(this->zk_, &fd, &interest, tv);

    fd_set rfds, wfds, efds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    if( fd != -1 ) {
        if (interest&ZOOKEEPER_READ) {
            FD_SET(fd, &rfds);
        }
        if (interest&ZOOKEEPER_WRITE) {
            FD_SET(fd, &wfds);
        }
    }
    else {
        fd = 0;
    }
    select(fd+1, &rfds, &wfds, &efds, tv);
    
    int events = 0;
    if (FD_ISSET(fd, &rfds)) {
        events |= ZOOKEEPER_READ;
    }
    if (FD_ISSET(fd, &wfds)) {
        events |= ZOOKEEPER_WRITE;
    }
    
    zookeeper_process(this->zk_, events);
}

unsigned int ZooKeeper::Private::countEvents() {
    return events_.size();
}

ZooKeeper::EventPtr ZooKeeper::Private::nextEvent() {
    if( events_.empty() ) return ZooKeeper::EventPtr();
    ZooKeeper::EventPtr evt = events_.front();
    events_.pop_front();
    return evt;
}
