#ifndef IBDB_BASE_NONCOPYABLE_H
#define IBDB_BASE_NONCOPYABLE_H
namespace ibdb {
namespace base {

class Noncopyable {
public:
    Noncopyable(const Noncopyable&) = delete;
    void operator=(const Noncopyable&) = delete;
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};
}
}

#endif // IBDB_BASE_NONCOPYABLE_H