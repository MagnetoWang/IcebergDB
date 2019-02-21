#include "skiplist.h"
#include "random.h"
#include "arena.h"

namespace ibdb {
namespace base
{
// 结构体Node
template<typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
    explicit Node(const Key& k) : key(k) {}
    // can't modify any type include pointer type
    // 不允许修改任何类型，如果是指针类型，那么不能修改指针的地址，但是可以修改指针的内容
    Key const key;

    Node* Next(int n) {
        assert(n >= 0);
        return reinterpret_cast<Node*>([next_[n].Acquire_load()]);
    }
    void SetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].Release_Store(x);
    }
    Node* NoBarrierNext(int n) {
        assert(n >= 0);
        return reinterpret_cast<Node*>(next_[n].NoBarrierLoad())
    }
    void NoBarrierSetNext(int n, Node* x) {
        assert(n >= 0);
        next_[n].NoBarrierStore(x);
    }
private:
    AtomicPointer next_[1];
};

/**
 * @brief new a node
 * @tparam Key 
 * @tparam Comparator 
 * @param key 
 * @param height 
 * @return SkipList<Key, Comparator>::Node* 
 * 
 * 内部通过arena来字节对齐
 * 
 */
template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode
(const Key& key, int height) {
    char* mem = arena_->AllocateAligned(
        sizeof(Node) + sizeof(AtomicPointer) * (height - 1)
    );
    return new (mem)Node(key);
}

template<typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Iterator
(const SkipList* list) {
    list_ = list;
    node_ = nullptr;
}

template<typename Key, class Comparator>
inline bool SkipList<Key, Comparator>::Iterator::Valid
() const {
    return node_ != nullptr;
}

template<typename Key, class Comparator>
inline const Key& SkipList<Key, Comparator>::Iterator::key
() const {
    assert(Valid());
    return node_->key;
}

// 跳到最低一层的下一个节点位置
template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Next
() {
    assert(Valid());
    node_ = node_->Next(0);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Prev
() {
    assert(Valid());
    node_ = list_->FindLessThan(node_->key);
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Seek
(const Key& target) {
    // assert(Valid())
    node_ = list_->FindGreaterOrEqual(target, nullptr);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToFirst
() {
    node_ = list_->head_->Next(0);
}

template<typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToLast
(){
    node_ = list_->FindLast();
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}

// false 节点不在当前key的后面
// true 节点在当前key的后面
template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::KeyIsAfterNode
(const Key& key, Node* n) const {
    // 当前节点是否为最后一个节点
    return (n != nullptr) && (compare_(n->key, key) < 0);
}

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key,Comparator>::FindGreaterOrEqual
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

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key,Comparator>::FindLessThan
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

template<typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key,Comparator>::FindLast
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

template<typename Key, class Comparator>
int SkipList<Key,Comparator>::RandomHeight() {
    static const unsigned int kBranching = 4;
    int height = 1;
    while(height < kMaxHeight && ((rnd_.Next() % kBranching) == 0)) {
        height++;
    }
    assert(height > 0);
    assert(height <= kMaxHeight);
    return height;
}

template<typename Key, class Comparator>
SkipList<Key,Comparator>::SkipList(Comparator cmp, Arena* arena)
    :   compare_(cmp),
        arena_(arena),
        head_(NewNode(0, kMaxHeight)),
        max_height_(reinterpret_cast<void*>(1)),
        rnd_(0xdeadbeef) {
            for (int i = 0; i < kMaxHeight; i++) {
                head_->SetNext(i, nullptr);
            }
        }

template<typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(const Key& key) {
    Node* prev[kMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev);
    
    assert(x == nullptr || !Equal(key, x->key));

    int height = RandomHeight();
    if (height > GetMaxHeight()) {
        for (int i = GetMaxHeight(); i < height; i++) {
            prev[i] = head_;
        }
        max_height_.NoBarrierStore(reinterpret_cast<void*>(height));
    }
    x = NewNode(key, height);
    for (int i = 0; i < height; i++) {
        x->NoBarrierSetNext(i, prev[i]->NoBarrierNext(i));
        prev[i]->SetNext(i, x);
    }
}

template<typename Key, class Comparator>
bool SkipList<Key,Comparator>::Contains(const Key& key) const {
    Node* x = FindGreaterOrEqual(key, nullptr);
    if (x != nullptr && Equal(key, x->key)) {
        return true;
    } else {
        return false;
    }
}

} // base
} // ibdb