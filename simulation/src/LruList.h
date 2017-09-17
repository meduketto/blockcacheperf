/*
 * 
 */

#ifndef LRU_LIST_H
#define LRU_LIST_H 1

#include "common.h"

#include <list>
#include <unordered_map>

template<class KEY_T>
class LruList {
public:
    LruList() { }

    void put(const KEY_T& key) {
        auto iter = itemMap_.find(key);
        if (iter != itemMap_.end()) {
            items_.erase(iter->second);
            itemMap_.erase(iter);
        }
        items_.push_front(key);
        itemMap_.insert(std::make_pair(key, items_.begin()));
    }

    KEY_T evictLeastRecent(void) {
        auto last = items_.end();
        --last;
        KEY_T key = *last;
        itemMap_.erase(key);
        items_.pop_back();
        return key;
    }

    int64_t size() const {
        return itemMap_.size();
    }

    bool has(const KEY_T& key) {
        return itemMap_.count(key) > 0;
    }

    void remove(const KEY_T& key) {
        auto iter = itemMap_.find(key);
        if (iter != itemMap_.end()) {
            items_.erase(iter->second);
            itemMap_.erase(iter);
        }
    }

private:
    std::list<KEY_T> items_;
    std::unordered_map<KEY_T,typename std::list<KEY_T>::iterator> itemMap_;
};


#endif /* LRU_LIST_H */
