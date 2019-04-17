#ifndef IBDB_STORAGE_CACHE_H
#define IBDB_STORAGE_CACHE_H
#include <map>
namespace ibdb {
namespace base {

template<typename Key, typename Value>
class LRUCacheMap {
public:
    void Insert(Key& key, Value& value) {
        cache_map_.insert(std::make_pair(key, value));
    }
    bool Contains(Key& key) {
        if (cache_map_.find(key) == cache_map_.end()) {
            return false;
        } else {
            return true;
        }
    }
    typename std::map<Key, Value>::iterator Find(Key& key) {
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return cache_map_.end();
        }
        return it;
    }
    void Remove(Key& key) {
        auto it = cache_map_.find(key);
        cache_map_.erase(it);
    }
private:
    std::map<Key, Value> cache_map_;
};

} // ibdb
} // storage

#endif // IBDB_STORAGE_CACHE_H