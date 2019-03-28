#ifndef IBDB_BASE_SKIPLIST_H
#define IBDB_BASE_SKIPLIST_H

#include <assert.h>
#include <stdlib.h>
#include "atomic_pointer.h"
#include "port/port.h"
#include "random.h"
#include "arena.h"
//TODO 总结如何设计一个良好的模板 最初的传值，然后指针结构，然后是引用
namespace ibdb {
namespace base {

template<typename Key, typename Value, class Comparator>
class SkipList : Noncopyable {
private:
    struct Node;
public:
    SkipList(Comparator cmp, Arena* arena);
    // ~SkipList();
    void Insert(const Key& key, Value& value);
    bool Contains(const Key& key) const;
    bool Remove(const Key& key);
    Node* FindEqual(const Key& key) const;
    Value& GetValue(const Key& key) const;
    Node* tail() const {return tail_;}
    Node* GetNode(const Key& key) const;

    class Iterator
    {
    public:
        explicit Iterator(const SkipList* list);
        bool Valid() const;
        const Key& key() const;
        const Value& value() const;
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
    //TODO 跳表高度flag化
    enum { kMaxHeight = 12};
    Comparator const compare_;
    Arena* const arena_;
    Node* const head_;
    Node* tail_;
    AtomicPointer max_height_;
    // double conversion from intptr_t to int
    // intptr_t 是为了适应不同平台指针对应的不同字节数
    inline int GetMaxHeight() const {
        return static_cast<int>(reinterpret_cast<intptr_t>(max_height_.NoBarrierLoad()));
    }

    Random rnd_;
    Node* NewNode(const Key& key, Value& value, int height);
    Node* NewNode(int height);
    int RandomHeight();
    bool Equal(const Key& a, const Key& b) const {
        return (compare_(a, b) == 0);
    }

    // bool EqualNode(const Node& a, const Node& b) const {
    //     return (compare_(a, b)==0);
    // }

    bool KeyIsAfterNode(const Key& key, Node* n) const;
    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;
    Node* FindLessThan(const Key& key) const;
    Node* FindLast() const;

    // 不允许复制
    // no copying allowed
    // SkipList(const SkipList&);
    // void operator=(const SkipList&);
};

// 结构体Node
template<typename Key, typename Value, class Comparator>
struct SkipList<Key, Value, Comparator>::Node {
    Node(const Key& k, Value& v, uint8_t h) : key_(k), value_(v), height_(h) {}
    Node(uint8_t h) : height_(h), key_(), value_() {}
    // can't modify any type include pointer type
    // 不允许修改任何类型，如果是指针类型，那么不能修改指针的地址，但是可以修改指针的内容
    Node(const Node&) = delete;
    void operator=(const SkipList&) {}

    Node* Next(int n) {
        assert(n >= 0);
        return reinterpret_cast<Node*>(next_[n].AcquireLoad());
    }
    void SetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].ReleaseStore(x);
    }
    Node* NoBarrierNext(int n) {
        assert(n >= 0);
        return reinterpret_cast<Node*>(next_[n].NoBarrierLoad());
    }
    void NoBarrierSetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].NoBarrierStore(x);
    }

    const Key& key() const {return key_;}
    Value& value() {return value_;}
    uint8_t height() const {return height_;}
private:
    AtomicPointer next_[1];
    Key const key_;
    Value value_;
    uint8_t height_;
};

/**
 * @brief new a node
 * @tparam Key 
 * @tparam Comparator 
 * @param key 
 * @param height 
 * @return SkipList<Key, Value, Comparator>::Node* 
 * 
 * 内部通过arena来字节对齐
 * 
 */
template<typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node* 
SkipList<Key, Value, Comparator>::NewNode
(const Key& key, Value& value, int height) {
    char* mem = arena_->AllocateAligned(
        sizeof(Node) + sizeof(AtomicPointer) * (height - 1)
    );
    return new (mem)Node(key, value, height);
}

template<typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node* 
SkipList<Key, Value, Comparator>::NewNode
(int height) {
    char* mem = arena_->AllocateAligned(
        sizeof(Node) + sizeof(AtomicPointer) * (height - 1)
    );
    return new (mem)Node(height);
}

template<typename Key, typename Value, class Comparator>
inline SkipList<Key, Value, Comparator>::Iterator::Iterator
(const SkipList* list) {
    list_ = list;
    node_ = nullptr;
}

template<typename Key, typename Value, class Comparator>
inline bool SkipList<Key, Value, Comparator>::Iterator::Valid
() const {
    return node_ != nullptr;
}

template<typename Key, typename Value, class Comparator>
inline const Key& SkipList<Key, Value, Comparator>::Iterator::key
() const {
    assert(Valid());
    return node_->key;
}

// 跳到最低一层的下一个节点位置
template<typename Key, typename Value, class Comparator>
inline void SkipList<Key, Value, Comparator>::Iterator::Next
() {
    assert(Valid());
    node_ = node_->Next(0);
}

template<typename Key, typename Value, class Comparator>
inline void SkipList<Key, Value, Comparator>::Iterator::Prev
() {
    assert(Valid());
    node_ = list_->FindLessThan(node_->key);
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}

template<typename Key, typename Value, class Comparator>
inline void SkipList<Key, Value, Comparator>::Iterator::Seek
(const Key& target) {
    // assert(Valid())
    node_ = list_->FindGreaterOrEqual(target, nullptr);
}

template<typename Key, typename Value, class Comparator>
inline void SkipList<Key, Value, Comparator>::Iterator::SeekToFirst
() {
    node_ = list_->head_->Next(0);
}

template<typename Key, typename Value, class Comparator>
inline void SkipList<Key, Value, Comparator>::Iterator::SeekToLast
(){
    node_ = list_->FindLast();
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}

// false 节点不在当前key的后面
// true 节点在当前key的后面
template<typename Key, typename Value, class Comparator>
bool SkipList<Key, Value, Comparator>::KeyIsAfterNode
(const Key& key, Node* n) const {
    // 当前节点是否为最后一个节点
    return (n != nullptr) && (compare_(n->key(), key) < 0);
}

template<typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node* 
SkipList<Key, Value, Comparator>::FindGreaterOrEqual
(const Key& key, Node** prev) const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while(true){
        Node* next = x->Next(level);
        if (KeyIsAfterNode(key, next)) {
            x = next;
        } else {
            if (prev != nullptr) {
                prev[level] = x;
            }
            if (level == 0) {
                return next;
            } else {
                level--;
            }
        }
    }
}

template<typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node* 
SkipList<Key, Value, Comparator>::FindLessThan
(const Key& key) const {
    Node* x =head_;
    int level = GetMaxHeight() - 1;
    while(true) {
        assert(x == head_ || compare_(x->key, key) < 0);
        Node* next = x->Next(level);
        if (next == nullptr || compare_(next->key, key) >= 0) {
            if (level == 0) {
                return x;
            } else {
                level--;
            }
        } else {
            x = next;
        }
    }
}

template<typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node* 
SkipList<Key, Value, Comparator>::FindLast
() const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    Node* next = x->Next(level);
    if (next == nullptr) {
        if (level == 0) {
            return x;
        } else {
            level--;
        }
    } else {
        x = next;
    }
}

template<typename Key, typename Value, class Comparator>
int SkipList<Key, Value, Comparator>::RandomHeight() {
    static const unsigned int kBranching = 4;
    int height = 1;
    while(height < kMaxHeight && ((rnd_.Next() % kBranching) == 0)) {
        height++;
    }
    assert(height > 0);
    assert(height <= kMaxHeight);
    return height;
}

template<typename Key, typename Value, class Comparator>
SkipList<Key, Value, Comparator>::SkipList(Comparator cmp, Arena* arena)
    :   compare_(cmp),
        arena_(arena),
        head_(NewNode(kMaxHeight)),
        max_height_(reinterpret_cast<void*>(1)),
        rnd_(0xdeadbeef) {
            // std::string value = "";
            // head_ = NewNode(0, value, kMaxHeight);
            tail_ = NewNode(kMaxHeight);
            for (int i = 0; i < kMaxHeight; i++) {
                head_->SetNext(i, nullptr);

            }
        }

template<typename Key, typename Value, class Comparator>
void SkipList<Key, Value, Comparator>::Insert(const Key& key, Value& value) {
    Node* prev[kMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev);

    assert(x == nullptr || !Equal(key, x->key()));

    int height = RandomHeight();
    if (height > GetMaxHeight()) {
        for (int i = GetMaxHeight(); i < height; i++) {
            prev[i] = head_;
        }
        max_height_.NoBarrierStore(reinterpret_cast<void*>(height));
    }
    x = NewNode(key, value, height);
    for (int i = 0; i < height; i++) {
        x->NoBarrierSetNext(i, prev[i]->NoBarrierNext(i));
        prev[i]->SetNext(i, x);
    }
}

template<typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node* 
SkipList<Key, Value, Comparator>::FindEqual(const Key& key) const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while(true) {
        Node* next = x->Next(level);
        if(KeyIsAfterNode(key, next)) {
            x = next;
        } else {
            if (level == 0) {
                if (next != nullptr && Equal(next->key(), key)) {
                    return next;
                }
                return nullptr;
            } else {
                level--;
            }
        }
    }
}

template<typename Key, typename Value, class Comparator>
bool SkipList<Key, Value, Comparator>::Contains(const Key& key) const {
    Node* x = FindEqual(key);
    if (x != nullptr && Equal(key, x->key())) {
        return true;
    } else {
        return false;
    }
}

// 需要判断是否存在,否则返回nullptr影响后面所有依赖的代码
template<typename Key, typename Value, class Comparator>
Value& SkipList<Key, Value, Comparator>::GetValue(const Key& key) const {
    Node* x = FindEqual(key);
    if (x != nullptr && Equal(key, x->key())) {
        return x->value();
    } else {
        return tail_->value();
    }
}

template<typename Key, typename Value, class Comparator>
typename SkipList<Key, Value, Comparator>::Node* SkipList<Key, Value, Comparator>::GetNode(const Key& key) const {
    Node* x = FindEqual(key);
    if (x != nullptr && Equal(key, x->key())) {
        return x;
    } else {
        return tail_;
    }
}

//找到目标节点和目标的上一个节点。然后直接把目标节点内容赋值到上一个节点即可！
// TODO 节点的回收问题
template<typename Key, typename Value, class Comparator>
bool SkipList<Key, Value, Comparator>::Remove(const Key& key) {
    if (!Contains(key)) {
        return false;
    }
    Node* prev[kMaxHeight];
    Node* target = FindGreaterOrEqual(key, prev);
    for (int i = 0; i < target->height(); i++) {
        prev[i]->SetNext(i, target->Next(i));
    }
    // delete target;
    return true;
}

} // base
} // ibdb


#endif // IBDB_BASE_SKIPLIST_H