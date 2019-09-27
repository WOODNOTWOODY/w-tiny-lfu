/**
 * @file hash_table.h
 * @author sunxiaohui(sunxiaohui_0618@163.com)
 * @date 2019/07/16 15:23
 * @brief 
 *  
 **/

#ifndef W_TINY_LFU_HASH_TABLE_H
#define W_TINY_LFU_HASH_TABLE_H

#include "type.h"

namespace w_tiny_lfu {

template <class T>
class HashTable {
public:
    HashTable() : _length(0), _elems(0), _list(nullptr) {
        _resize();
    }
    
    ~HashTable() {
        delete [] _list;
    }

    Node<T>* lookup(const std::string& key, uint32_t hash) {
        return *_find_pointer(key, hash);
    }

    Node<T>* insert(Node<T>* node) {
        Node<T>** ptr = _find_pointer(node->key, node->hash);
        Node<T>* old = *ptr;
        node->next_hash = (old == nullptr ? nullptr : old->next_hash);
        *ptr = node;
        if (old == nullptr) {
            ++_elems;
            if (_elems > _length) {
                _resize();
            }
        }
        return old;
    }

    Node<T>* remove(const std::string& key, uint32_t hash) {
        Node<T>** ptr = _find_pointer(key, hash);
        Node<T>* result = *ptr;
        if (result != nullptr) {
            *ptr = result->next_hash;
            --_elems;
        }
        return result;
    }
private:
    Node<T>** _find_pointer(const std::string& key, uint32_t hash) {
        Node<T>** ptr = &_list[hash & (_length - 1)];
        while (*ptr != nullptr && ((*ptr)->hash != hash || key != (*ptr)->key)) {
            ptr = &((*ptr)->next_hash);
        }
        return ptr;
    }

    void _resize() {
        uint32_t new_length = 4;
        while (new_length < _elems) {
            new_length *= 2;
        }

        Node<T>** new_list = new Node<T>* [new_length];
        memset(new_list, 0, sizeof(new_list[0]) * new_length);
        for (uint32_t i = 0; i < _length; i++) {
            Node<T>* node = _list[i];
            while (node != nullptr) {
                Node<T>* next = node->next_hash;
                uint32_t hash = node->hash;
                Node<T>** ptr = &new_list[hash & (new_length - 1)];
                node->next_hash = *ptr;
                *ptr = node;
                node = next;
            }
        }

        delete [] _list;
        _list = new_list;
        _length = new_length;
    }
private:
    uint32_t _length;
    uint32_t _elems;
    Node<T>** _list;
};

}

#endif // W_TINY_LFU_HASH_TABLE_H
