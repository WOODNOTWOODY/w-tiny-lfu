/**
 * @file lru_cache.h
 * @author sunxiaohui(sunxiaohui_0618@163.com)
 * @date 2018/09/04 15:23
 * @brief 
 *  
 **/

#ifndef W_TINY_LFU_LRU_CACHE_H
#define W_TINY_LFU_LRU_CACHE_H

#include "type.h"

namespace w_tiny_lfu {

// not thread-safe lru
template <class T>
class LRUCache {
public:
    LRUCache(int capacity, NodeLocation loc) :
            _capacity(capacity), _size(0), _loc(loc) {
        _lru.next = &_lru;
        _lru.prev = &_lru;
    }

    ~LRUCache() = default;

    void insert(Node<T>* node, Node<T>** eviction_node) {
        // evict
        if (_size >= _capacity && _lru.next != &_lru) {
            *eviction_node = _lru.next;
            _remove(*eviction_node);
            (*eviction_node)->loc = NL_ALONE;
            _size--;
        }

        node->loc = _loc;
        _append(node);
        _size++;
    }

    void remove(Node<T>* node) {
        _remove(node);
        node->loc = NL_ALONE;
        _size--;
    }

    void hit(Node<T>* node) {
        _remove(node);
        _append(node);
    }
private:
    void _append(Node<T>* node) {
        node->next = &_lru;
        node->prev = _lru.prev;
        node->prev->next = node;
        node->next->prev = node;
    }

    void _remove(Node<T>* node) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
private:
    int _capacity;
    int _size;
    Node<T> _lru;
    NodeLocation _loc;
private:
    DISALLOW_COPY_AND_ASSIGN(LRUCache);
};

}

#endif // W_TINY_LFU_LRU_CACHE_H 
