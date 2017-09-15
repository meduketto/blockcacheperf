/*
 * 
 */

#ifndef LRU_LIST_H
#define LRU_LIST_H 1

#include "common.h"

#include <list>
#include <unordered_map>

template<class KEY_T, class VALUE_T>
class LruList {
public:
    LruList(int64_t size = 100): size_(size) { }

    void setSize(int64_t size) { size_ = size; }

    void put(const KEY_T& key, const VALUE_T& value) {
        auto iter = itemMap_.find(key);
        if (iter != itemMap_.end()) {
            items_.erase(iter->second);
            itemMap_.erase(iter);
        }
        items_.push_front(std::make_pair(key, value));
        itemMap_.insert(std::make_pair(key, items_.begin()));
        if ((int64_t) itemMap_.size() > size_) {
            evict();
        }
    }

    VALUE_T evict(void) {
        auto last = items_.end();
        --last;
        itemMap_.erase(last->first);
        VALUE_T r = last->second;
        items_.pop_back();
        return r;
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

    VALUE_T get(const KEY_T& key) {
        auto iter = itemMap_.find(key);
        items_.splice(items_.begin(), items_, iter->second);
        return iter->second->second;
    }

private:
    int64_t size_;
    std::list<std::pair<KEY_T,VALUE_T>> items_;
    std::unordered_map<KEY_T,typename std::list<std::pair<KEY_T,VALUE_T>>::iterator> itemMap_;
};


#endif /* LRU_LIST_H */
