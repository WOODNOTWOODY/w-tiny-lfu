/**
 * @file type.h
 * @author sunxiaohui(sunxiaohui_0618@163.com)
 * @date 2018/09/04 14:57
 * @brief 
 *  
 **/

#ifndef W_TINY_LFU_TYPE_H
#define W_TINY_LFU_TYPE_H

#include <memory>

namespace w_tiny_lfu {

// node的位置
enum NodeLocation : uint8_t {
    NL_ALONE          = 0,        // 独立节点
    NL_WINDOW_LRU     = (1 << 0), // 在window lru中
    NL_PROBATION_LRU  = (1 << 2), // 在probation lru中
    NL_PROTECTION_LRU = (1 << 3), // 在protection lru中
};

// cache node
template <typename T>
struct Node {
    NodeLocation loc;
    std::string key;
    uint32_t hash; // used for fast sharding and comparisons
    std::shared_ptr<T> value;
    int64_t timestamp_ms;
    Node* prev; // prev node
    Node* next; // next bucket
    Node* next_hash; // next node

    Node() : loc(NL_ALONE), hash(0), timestamp_ms(0), 
            prev(nullptr), next(nullptr), next_hash(nullptr) {}
    Node(const std::string& k, uint32_t hash, T* v, int64_t time_ms) : 
            loc(NL_ALONE), key(k), hash(hash), value(v), timestamp_ms(time_ms),
            prev(nullptr), next(nullptr), next_hash(nullptr) {}
};

}

#endif // W_TINY_LFU_TYPE_H
