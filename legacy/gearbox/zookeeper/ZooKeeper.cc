// Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
// Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.

#include <gearbox/zookeeper/ZooKeeper.h>
#include <gearbox/zookeeper/ZooKeeperPrivate.h>
#include <gearbox/zookeeper/ZooKeeperStatPrivate.h>
#include <gearbox/core/Uri.h> /* for Uri::canonicalize_path */

using namespace Gearbox;

ZooKeeper::Stat::Stat()
    : impl(new Private()) {
}

ZooKeeper::Stat::~Stat() {
    delete impl;
}

ZooKeeper::Stat::Stat( const ZooKeeper::Stat & stat ) {
    impl = new Private(*(stat.impl));
}

int32_t ZooKeeper::Stat::version() {
    return impl->version();
}

int64_t ZooKeeper::Stat::modTxId() {
    return impl->modTxId();
}

int32_t ZooKeeper::Stat::dataLength() {
    return impl->dataLength();
}

int32_t ZooKeeper::Stat::numChildren() {
    return impl->numChildren();
}

int64_t ZooKeeper::Stat::mtime() {
    return impl->mtime();
}

int64_t ZooKeeper::Stat::ctime() {
    return impl->ctime();
}


ZooKeeper::Stat ZooKeeper::Stat::operator=(const ZooKeeper::Stat & stat) {
    if( impl && this != &stat ) {
        delete impl;
    }
    impl = new Private(*(stat.impl));
    return *this;
}

std::string ZooKeeper::eventTypeToString(ZooKeeper::EventType evt) {
    switch(evt) {
    case ZooKeeper::EVENT_CREATED:      return "CREATED";
    case ZooKeeper::EVENT_DELETED:      return "DELETED";
    case ZooKeeper::EVENT_CHANGED:      return "CHANGED";
    case ZooKeeper::EVENT_CHILD:        return "CHILD";
    case ZooKeeper::EVENT_NOTWATCHING:  return "NOTWATCHING";
    default:                            return "UNKNOWN";
    }
}


ZooKeeper::ZooKeeper( const std::string & connection, int timeout ) 
    : impl(new Private(connection, timeout, this)) {
}

ZooKeeper::~ZooKeeper() {
    delete(impl);
}

bool ZooKeeper::exists(
    const std::string & path,
    bool watch
) {
    return impl->exists(path,watch);
}
    
bool ZooKeeper::exists(
    const std::string & path,
    WatchCallback callback,
    void * context
) {
    return impl->exists(path,callback,context);
}

ZooKeeper::Stat ZooKeeper::stat() {
    return impl->stat();
}

ZooKeeper::Stat ZooKeeper::stat( const std::string & path ) {
    return impl->stat(path);
}

void ZooKeeper::get(
    const std::string & path,
    std::string & data,
    bool watch
) {
    impl->get(path,data,watch);
}

void ZooKeeper::get(
    const std::string & path,
    std::string & data,
    WatchCallback callback,
    void * context
) {
    impl->get(path,data,callback,context);
}

void ZooKeeper::set(
    const std::string & path,
    const std::string & data,
    int32_t version
) {
    impl->set(path, data, version);
}

void ZooKeeper::create(
    std::string & path, // may be updated if sequential node
    const std::string & data,
    ZooKeeper::NodeType type
) {
    impl->create(path,data,type);
}

void ZooKeeper::create(
    const std::string & path,
    const std::string & data,
    ZooKeeper::NodeType type
) {
    impl->create(path,data,type);
}

void ZooKeeper::createFullPath(
    const std::string & path
) {
    size_t pos = 0;
    while( (pos=path.find('/', pos)) != std::string::npos ) {
        std::string subpath = path.substr(0,pos);
        if( !subpath.empty() && subpath != "/" ) {
            if( ! this->exists(subpath) ) {
                this->create(subpath, "");
            }
        }
        pos++;
    }
    if( ! this->exists(path) ) {
        this->create(path, "");
    }
}

void ZooKeeper::del(
    const std::string & path,
    int32_t version
) { 
    impl->del(path,version);
}
    
void ZooKeeper::children(
    const std::string & path,
    std::vector<std::string> & entries,
    bool watch
) {
    impl->children(path, entries, watch);
}

void ZooKeeper::children(
    const std::string & path,
    std::vector<std::string> & entries,
    WatchCallback callback,
    void * context
) {
    impl->children(path, entries, callback, context);
}

void ZooKeeper::yield( int timeout ) {
    impl->yield(timeout);
}

void ZooKeeper::yield() {
    impl->yield();
}

unsigned int ZooKeeper::countEvents() {
    return impl->countEvents();
}

ZooKeeper::EventPtr ZooKeeper::nextEvent() {
    return impl->nextEvent();
}

std::string Gearbox::ZooKeeper::normalize_path(const std::string& path)
{
    std::string res = Uri::canonicalize_path(path);
    // Unlike in URIs, in ZK paths trailing slashes need to be removed
    if (res.size() >= 2 && res[res.size()-1] == '/') {
	res.resize(res.size()-1);
    }
    return res;
}

std::string Gearbox::ZooKeeper::join_paths(const std::string& path1, const std::string& path2)
{
    std::string res = path1;
    res.append("/");
    res.append(path2);
    return normalize_path(res);
}
