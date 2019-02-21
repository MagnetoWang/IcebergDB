#ifndef IBDB_BASE_SKIPLIST_H
#define IBDB_BASE_SKIPLIST_H

#include <assert.h>
#include <stdlib.h>
#include "atomic_pointer.h"
namespace ibdb {
namespace base {

template<typename Key, class Comparator>
class SkipList : Noncopyable {
public:
    explicit SkipList(Comparator cmp, Arena* arena);
    ~SkipList();
    void Insert(const Key& key);
    bool Contains(const Key& key) const;

    class Iterator
    {
    public:
        explicit Iterator(const SkipList* list);
        bool Valid() const;
        const Key& key() const;
        void Next();
        void Prev();
        void Seek(const Key& target);
        void SeekToFirst();
        void SeekToLast();
        ~Iterator();
    private:
        const SkipList* list_;
        Node* node_;
    };
private:
    struct Node;
    enum { kMaxHeight = 12};
    Comparator const compare_;
    Arena* const arena_;
    Node* const head_;
    AtomicPointer max_height_;
    // double conversion from intptr_t to int
    inline int GetMaxHeight() const {
        return static_cast<int>(reinterpret_cast<intptr_r>(max_height_.NoBarrier_Load()));
    }

    Random rnd_;
    Node* NewNode(const Key& key, int height);
    int RandomHeight();
    bool Equal(const Key& a, const Key& b) const {
        return (compare_(a, b) == 0);
    }

    bool KeyIsAfterNode(const Key& key, Node* n) const;
    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;
    Node* FindLessThan(const Key& key) const;
    Node* FindLast() const;

    // 不允许复制
    // no copying allowed
    SkipList(const SkipList&);
    void operator=(const SkipList&);
};

}
}


#endif // IBDB_BASE_SKIPLIST_H