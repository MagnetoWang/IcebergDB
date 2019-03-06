#ifndef IBDB_BASE_SLICE_H_
#define IBDB_BASE_SLICE_H_

#include<string>
#include <string.h>
#include <assert.h>
namespace ibdb {
namespace base {

class Slice {
public:
    // constructed function
    Slice() : data_(""), size_(0), need_free_(false) {}
    Slice(const char* d, size_t n, bool nf) : data_(d), size_(n), need_free_(nf) {}
    Slice(const std::string& s) : data_(s.data()), size_(s.size()), need_free_(false) {}
    Slice(const char* s) : data_(s), size_(strlen(s)), need_free_(false) {}
    Slice(const char* s, const int n) : data_(s), size_(n), need_free_(false) {}

    //destructor function
    ~Slice() {}
    
    // assignment function
    Slice(const Slice&) = default;
    Slice& operator=(const Slice&) = default;
    
    // element access
    const char* data() const { return data_; }
    size_t size() const { return size_; }
    bool need_free() const { return need_free_; }
   
    // operations
    bool empty() const { return size_ == 0; }
    char operator[](size_t n) const {
        assert(n < size());
        return data_[n];
    }
    void remove_prefix(size_t n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }

    int compare(const Slice& b) const;
    void clear() { data_ = ""; size_ = 0; need_free_ = false; }
    std::string ToString() const { return std::string(data_, size_); }
    bool starts_with(const Slice& x) const {
        return ((size_ >= x.size()) && 
                (memcmp(data_,x.data_, x.size_) == 0));
    }


private:
    const char* data_;
    size_t size_; // = strlen(data_)
    bool need_free_;
};

inline bool operator==(const Slice& x, const Slice& y) {
    return ((x.size() == y.size()) &&
            (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) {
    return !(x == y);
}

inline int Slice::compare(const Slice& b) const {
    const size_t min_len = (size_ < b.size()) ? size_ : b.size();
    int result = memcmp(data_, b.data(), min_len);
    if (result == 0) {
        if (size_ < b.size()) {
            result = -1;
        }
        if (size_ > b.size()) {
            result = 1;
        }
    }
    return result;
}

}
}



#endif //IBDB_BASE_SLICE_H_