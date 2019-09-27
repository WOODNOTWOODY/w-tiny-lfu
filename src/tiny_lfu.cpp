/**
 * @file tiny_lfu.cpp
 * @author sunxiaohui(sunxiaohui_0618@163.com)
 * @date 2018/09/04 20:12
 * @brief 
 *  
 **/

#include "tiny_lfu.h"

namespace w_tiny_lfu {

// RS Hash Function
static unsigned int rs_hash(char* str) {
    unsigned int b = 378551 ;
    unsigned int a = 63689 ;
    unsigned int hash = 0 ;

    while (*str) {
        hash = hash * a + (*str++);
        a *= b ;
    }

    return hash;
}

// JS Hash Function
static unsigned int js_hash(char* str) {
    unsigned int hash = 1315423911 ;

    while (*str) {
        hash ^= ((hash << 5) + (*str++) + (hash >> 2));
    }

    return hash;
}

// BKDR Hash Function
static unsigned int bkdr_hash(char* str) {
    unsigned int seed = 131 ; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0 ;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return hash;
}

// SDBM Hash Function
static unsigned int sdbm_hash(char* str) {
    unsigned int hash = 0 ;

    while (*str) {
        hash = (*str++) + (hash << 6) + (hash << 16) - hash ;
    }

    return hash;
}

// DJB Hash Function
static unsigned int djb_hash(char* str) {
    unsigned int hash = 5381 ;

    while (*str) {
        hash += (hash << 5) + (*str++);
    }

    return hash;
}

// AP Hash Function
static unsigned int ap_hash(char* str) {
    unsigned int hash = 0 ;

    for (int i = 0; *str; i++) {
        if ((i & 1) == 0) {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        } else {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }

    return hash;
}

TinyLFU::TinyLFU(int size) : _size(size),
    _max_count(255), _eviction_count(1) {
    _hash_function_array[0] = rs_hash;
    _hash_function_array[1] = js_hash;
    _hash_function_array[2] = bkdr_hash;
    _hash_function_array[3] = sdbm_hash;
    _hash_function_array[4] = djb_hash;
    _hash_function_array[5] = ap_hash;

    for (int i = 0; i < 6; i++) {
        _counter_array[i] = new uint8_t[size];
        memset(_counter_array[i], 0, size);
    }
}

TinyLFU::~TinyLFU() {
    for (int i = 0; i < 6; i++) {
        delete[] _counter_array[i];
    }
}

void TinyLFU::set_count(const std::string& key) {
    for (int i = 0; i < 6; i++) {
        char buf[key.size() + 1];
        snprintf(buf, key.size(), "%s", key.c_str());
        int index = _hash_function_array[i](buf) % _size;
        if (_counter_array[i][index] < _max_count) {
            _counter_array[i][index] = _counter_array[i][index] + 1;
        } 
    }
}

bool TinyLFU::is_allowed_cache(const std::string& key) {
    return _get_count(key) > _eviction_count;
}

void TinyLFU::decrease_count(const std::string& key) {
    uint8_t decrement = _get_count(key);
    for (int i = 0; i < 6; i++) {
        char buf[key.size() + 1];
        snprintf(buf, key.size(), "%s", key.c_str());
        int index = _hash_function_array[i](buf) % _size;
        if (_counter_array[i][index] > decrement) {
            _counter_array[i][index] = _counter_array[i][index] - decrement;
        } else {
            _counter_array[i][index] = 0;
        }
    }
}

uint8_t TinyLFU::_get_count(const std::string& key) {
    uint8_t min_value = 0;
    for (int i = 0; i < 6; i++) {
        char buf[key.size() + 1];
        snprintf(buf, key.size(), "%s", key.c_str());
        int index = _hash_function_array[i](buf) % _size;
        if (min_value > _counter_array[i][index]) {
            min_value = _counter_array[i][index];
        }
    }
    return min_value;
}

}
