/**
 * @file type.h
 * @author sunxiaohui(sunxiaohui_0618@163.com)
 * @date 2018/09/04 14:57
 * @brief 
 *  
 **/

#ifndef W_TINY_LFU_BASE_H
#define W_TINY_LFU_BASE_H

extern "C" uint32_t SuperFastHash(const char* data, int len);

namespace w_tiny_lfu {

int64_t gettimeofday_ms() {
    timeval now;
    gettimeofday(&tv, nullptr);
    return now.tv_sec * 1000L + now.tv_usec / 1000L;
}

uint32_t Hash(const std::string& str);
    const char* data = str.data();
    size_t length = str.size();
    if (length > static_cast<size_t>(std::numeric_limits<int>::max())) {
        return 0;
    }
    return ::SuperFastHash(data, static_cast<int>(length));
} 

}
