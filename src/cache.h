/**
 * @file cache.h
 * @author sunxiaohui(sunxiaohui_0618@163.com)
 * @date 2019/07/16 14:55
 * @brief 
 *  
 **/

#ifndef W_TINY_LFU_CACHE_H
#define W_TINY_LFU_CACHE_H

#include <mutex>
#include "hash_table.h"
#include "lru_cache.h"
#include "tiny_lfu.h"
#include "type.h"
#include "base.h"

namespace w_tiny_lfu {

template <class T>
class CacheShard {
public:
    explicit CacheShard(int capacity) :
        _is_loading(false),
        _capacity(capacity),
        _lfu(capacity),
        _window_lru(capacity * 0.01, NL_WINDOW_LRU), 
        _probation_lru(capacity * 0.99 * 0.2, NL_PROBATION_LRU),
        _protection_lru(capacity * 0.99 * 0.8, NL_PROTECTION_LRU) {
    }

    ~CacheShard() {
    }

    std::shared_ptr<T> lookup(const std::string& key, uint32_t hash,
            int64_t* timestamp_ms) {
        std::lock_guard<std::mutex> lock(_mutex);
        Node<T>* node = _data.lookup(key, hash);
        if (node != nullptr) {
            _hit(node);
            if (timestamp_ms != nullptr) {
                *timestamp_ms = node->timestamp_ms;
            }
            return node->value;
        }
        return nullptr;
    }

    std::shared_ptr<T> pure_lookup(const std::string& key, uint32_t hash) {
        std::lock_guard<std::mutex> lock(_mutex);
        Node<T>* node = _data.lookup(key, hash);
        if (node != nullptr) {
            return node->value;
        }
        return nullptr;
    }

    std::shared_ptr<T> insert(const std::string& key, uint32_t hash, T* value) {
        std::lock_guard<std::mutex> lock(_mutex);
        Node<T>* node = _data.lookup(key, hash);
        if (node != nullptr) {
            node->timestamp_ms = gettimeofday_ms();
            node->value.reset(value);
            _hit(node);
        } else {
            node = new Node<T>(key, hash, value, gettimeofday_ms());
            _data.insert(node);
            Node<T>* eviction_node = nullptr;
            _window_lru.insert(node, &eviction_node);
            if (eviction_node != nullptr) {
                Node<T>* remove_node = nullptr;
                if (_is_loading || _lfu.is_allowed_cache(eviction_node->key)) {
                    _probation_lru.insert(eviction_node, &remove_node);
                } else {
                    remove_node = eviction_node;
                }
                if (remove_node != nullptr) {
                    _data.remove(remove_node->key, remove_node->hash);
                    delete remove_node;
                }
            }
            _lfu.set_count(key);
        }
        return node->value;
    }
    
    void update(const std::string& key, uint32_t hash, T* value) {
        std::lock_guard<std::mutex> lock(_mutex);
        Node<T>* node = _data.lookup(key, hash);
        if (node != nullptr) {
            node->timestamp_ms = gettimeofday_ms();
            node->value.reset(value);
        } 
    }

    void set_loading(bool is_loading) {
        _is_loading = is_loading;
    }
private:
    void _hit(Node<T>* node) {
        if (node->loc == NL_WINDOW_LRU) {
            _window_lru.hit(node);
        } else if (node->loc == NL_PROBATION_LRU) {
            _probation_lru.remove(node);
            Node<T>* eviction_node = nullptr;
            _protection_lru.insert(node, &eviction_node);
            if (eviction_node != nullptr) {
                Node<T>* remove_node = nullptr;
                _probation_lru.insert(eviction_node, &remove_node);
                if (remove_node != nullptr) {
                    _data.remove(remove_node->key, remove_node->hash);
                    delete remove_node;
                }
            }
        } else if (node->loc == NL_PROTECTION_LRU) {
            _protection_lru.hit(node);
        }
        _lfu.set_count(node->key);
    }
private:
    bool _is_loading;
    int _capacity;
    std::mutex _mutex;
    HashTable<T> _data;
    TinyLFU _lfu;
    LRUCache<T> _window_lru;
    LRUCache<T> _probation_lru;
    LRUCache<T> _protection_lru;
};

static const int kNumShardBits = 4;
static const int kNumShards = 1 << kNumShardBits;

template <class T>
class Cache {
public:
    explicit Cache(int capacity) {
        const int per_num = (capacity + (kNumShards - 1)) / kNumShards;
        for (int i = 0; i < kNumShards; i++) {
            _shards[i].reset(new CacheShard<T>(per_num));
        }
    }
    ~Cache() = default;
    
    std::shared_ptr<T> lookup(const std::string& key, 
            int64_t* timestamp_ms = nullptr) {
        const uint32_t hash = Hash(key);
        return _shards[_shard_index(hash)]->lookup(key, hash, timestamp_ms);
    }
    
    std::shared_ptr<T> pure_lookup(const std::string& key) {
        const uint32_t hash = Hash(key);
        return _shards[_shard_index(hash)]->pure_lookup(key, hash);
    }

    std::shared_ptr<T> insert(const std::string& key, T* value) {
        const uint32_t hash = Hash(key);
        return _shards[_shard_index(hash)]->insert(key, hash, value);
    }
    
    void update(const std::string& key, T* value) {
        const uint32_t hash = Hash(key);
        return _shards[_shard_index(hash)]->update(key, hash, value);
    }

    void set_loading(bool is_loading) {
        for (int i = 0; i < kNumShards; i++) {
            _shards[i]->set_loading(is_loading);
        }
    }
private:
    static uint32_t _shard_index(uint32_t hash) {
        return hash >> (32 - kNumShardBits);
    } 
private:
    std::unique_ptr<CacheShard<T>> _shards[kNumShards];
};

}

#endif // W_TINY_LFU_CACHE_H
