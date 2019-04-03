/*
 * @Author: MagnetoWang 
 * @Date: 2019-03-29 09:50:57 
 * @Last Modified by:   MagnetoWang 
 * @Last Modified time: 2019-03-29 09:50:57 
 */
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