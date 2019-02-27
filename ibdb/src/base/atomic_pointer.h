#ifndef IBDB_BASE_ATOMIC_POINTER_H
#define IBDB_BASE_ATOMIC_POINTER_H
#include <stdint.h>
#include <atomic>

namespace ibdb {
namespace base {

class AtomicPointer {
public:
    AtomicPointer() {}
    explicit AtomicPointer(void* v) : rep_(v) {}
    inline void* AcquireLoad() const {
        return rep_.load(std::memory_order_acquire);
    }
    inline void ReleaseStore(void* v) {
        rep_.store(v, std::memory_order_release);
    }
    inline void* NoBarrierLoad() const {
        return rep_.load(std::memory_order_relaxed);
    }
    inline void NoBarrierStore(void* v) {
        rep_.store(v, std::memory_order_relaxed);
    }
private:
    std::atomic<void*> rep_;
};

} // base
} // ibdb


#endif // IBDB_BASE_ATOMIC_POINTER_H
