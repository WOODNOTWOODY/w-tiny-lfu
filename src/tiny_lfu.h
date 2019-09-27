/**
 * @file tiny_lfu.h
 * @author sunxiaohui(sunxiaohui_0618@163.com)
 * @date 2018/09/04 19:52
 * @brief 
 *  
 **/

#ifndef W_TINY_LFU_TINY_LFU_H
#define W_TINY_LFU_TINY_LFU_H

#include <cstring>
#include <string>

namespace w_tiny_lfu {

// not thread-safe tiny-lfu
class TinyLFU {
public:
    typedef unsigned int (*HashFunction)(char* src);

    explicit TinyLFU(int size);
    
    ~TinyLFU();

    /** set count */
    void set_count(const std::string& key);

    /** is allow cache */
    bool is_allowed_cache(const std::string& key);

    /** decrease count */
    void decrease_count(const std::string& key);

private:
    /** get count */
    uint8_t _get_count(const std::string& key);

private:
    /** size */
    int _size;
    /** max count */
    uint8_t _max_count;
    /** eviction count */
    uint8_t _eviction_count;
    /** hash function */
    HashFunction _hash_function_array[6];
    /** counter array */
    uint8_t* _counter_array[6];
};

}

#endif // W_TINY_LFU_TINY_LFU_H 
