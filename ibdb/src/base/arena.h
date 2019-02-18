#ifndef IBDB_BASE_ARENA_H
#define IBDB_BASE_ARENA_H
#include <vector>
#include "atomic_pointer.h"
#include "noncopyable.h"
#include <assert.h>
#include <stddef.h>

namespace ibdb {
namespace base {

class Arena : Noncopyable {
private:
    char* AllocateFallBack(size_t bytes);
    char* AllocateNewBlock(size_t block_bytes);
    // Allocation state
    //分配状态
    char* alloc_ptr_;
    size_t alloc_bytes_remaining_;

    std::vector<char*> blocks_;
    AtomicPointer memory_usage_;

public:
    Arena(/* args */);
    ~Arena();

    char* Allocate(size_t bytes);
    char* AllocateAligned(size_t bytes);

    size_t MemoryUsage() const {
        return reinterpret_cast<uintptr_t>(memory_usage_.NoBarrierLoad());
    }
};

inline char* Arena::Allocate(size_t bytes) {
    assert(bytes > 0);
    if (bytes <= alloc_bytes_remaining_) {
        char* result = alloc_ptr_;
        alloc_ptr_ += bytes;
        alloc_bytes_remaining_ -= bytes;
        return result;
    }
    return AllocateFallBack(bytes);
}

} // base
} // ibdb


#endif // IBDB_BASE_ARENA_H