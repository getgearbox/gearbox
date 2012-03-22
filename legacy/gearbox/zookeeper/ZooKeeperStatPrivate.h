#ifndef GEARBOX_ZOOKEEPER_STAT_PRIVATE_H
#define GEARBOX_ZOOKEEPER_STAT_PRIVATE_H

namespace Gearbox {
    class ZooKeeper::Stat::Private {

    public:
        Private( const Private & stat) {
            this->stat_ = stat.stat_;
        }

        ~Private() {}
        
        Private() {
          memset(&stat_, 0, sizeof(stat_));
        }

        int32_t version() {
            return stat_.version;
        }
        
        int64_t modTxId() {
            return stat_.mzxid;
        }

        int32_t dataLength() {
            return stat_.dataLength;
        }
        
        int32_t numChildren() {
            return stat_.numChildren;
        }

        int64_t mtime() {
            return stat_.mtime;
        }
        
        int64_t ctime() {
            return stat_.ctime;
        }
        

        ::Stat stat_; // zookeeper Stat struct
    };
}

#endif // GEARBOX_ZOOKEEPER_STAT_PRIVATE_H
