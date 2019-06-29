#ifndef EXAM_SET_H
#define EXAM_SET_H

#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <iterator>


template<typename T>
struct set {
    struct base_node {
        base_node *left;
        base_node *right;
        base_node *parent;

        base_node() : left(nullptr), right(nullptr), parent(nullptr) {}

        base_node(base_node *left, base_node *right, base_node *parent) : left(left), right(right), parent(parent) {}

        virtual ~base_node() = default;

    };

    struct node : base_node {
        T data;

        explicit node(const T &data) : base_node(nullptr, nullptr, nullptr), data(data) {}

        node(const T &data, base_node *left, base_node *right, base_node *par) : base_node(left, right, par),
                                                                                 data(data) {}

        ~node() {}
    };

    base_node fake_root;
    base_node *fake_ptr = &fake_root;

public:

    template<typename V>
    struct set_iterator {
    public:
        explicit set_iterator(base_node &p) : ptr(&p) {}

        set_iterator() : ptr(nullptr) {}

        set_iterator(set_iterator const &other) : ptr(other.ptr) {}

        set_iterator &operator++() {
            if (ptr->right != nullptr) {
                ptr = minimum(ptr->right);
                return *this;
            }
            base_node *tmp = ptr->parent;
            while (tmp && ptr == tmp->right) {
                ptr = tmp;
                tmp = tmp->parent;
            }
            ptr = tmp;
            return *this;
        }

        set_iterator &operator--() {
            if (ptr->left != nullptr) {
                ptr = maximum(ptr->left);
                return *this;
            }
            base_node *tmp = ptr->parent;
            while (tmp && ptr == tmp->left) {
                ptr = tmp;
                tmp = tmp->parent;
            }
            ptr = tmp;
            return *this;
        }

        const set_iterator operator++(int) {
            set_iterator prev(*ptr);
            ++(*this);
            return prev;
        }

        const set_iterator operator--(int) {
            set_iterator prev(*ptr);
            --(*this);
            return prev;
        }

        V &operator*() const {
            return static_cast<node *>(ptr)->data;
        }

        V *operator->() const {
            return &(static_cast<node *>(ptr)->data);
        }

        friend bool operator==(set_iterator const &a, set_iterator const &b) {
            return a.ptr == b.ptr;
        }

        friend bool operator!=(set_iterator const &a, set_iterator const &b) {
            return a.ptr != b.ptr;
        }

        base_node *get() {
            return ptr;
        }

        base_node *minimum(base_node *cur) {
            if (cur->left == nullptr) {
                return cur;
            }
            return minimum(cur->left);
        }

        base_node *maximum(base_node *cur) {
            if (cur->right == nullptr) {
                return cur;
            }
            return maximum(cur->right);
        }

        typedef std::ptrdiff_t difference_type;
        typedef V value_type;
        typedef V *pointer;
        typedef V &reference;
        typedef std::bidirectional_iterator_tag iterator_category;

        base_node *ptr;
    };

    typedef set_iterator<const T> iterator;
    typedef set_iterator<const T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    set() noexcept {
        fake_ptr->left = nullptr;
        fake_ptr->right = nullptr;
    }

    set(set const &other) : set() {
        auto cur = other.begin();
        while (cur != other.end()) {
            insert(*cur);
            cur++;
        }
    }

    set &operator=(set const &other) {
        set new_other(other);
        swap(*this, new_other);
        return *this;
    }

    ~set() {
        clear();
    }

    bool operator==(set const &rhs) {
        auto j = rhs.begin();
        for (auto i = begin(); i != end(); i++) {
            if (j == rhs.end()) {
                return false;
            }
            if (*i != *j) {
                return false;
            }
        }
        return j != rhs.end();
    }

    void clear() {
        destroy_subtree(fake_ptr->left);
        fake_ptr->left = nullptr;
    }

    bool operator!=(set const &rhs) {
        return !(operator==(rhs));
    }

    const_iterator begin() const noexcept {
        auto cur = fake_ptr;
        while (cur->left != nullptr) {
            cur = cur->left;
        }
        return const_iterator(*(cur));
    }

    const_iterator end() const noexcept {
        return const_iterator(*(fake_ptr));
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }


    std::pair<const_iterator, bool> insert(T const &elem) {
        auto pre_search = find(elem);
        if (pre_search != end()) {
            return std::make_pair(pre_search, true);
        }
        return std::make_pair(insert_impl(static_cast<node *>(fake_ptr->left), elem), false);
    }

    const_iterator find(T const &elem) const {
        return find_impl(fake_ptr->left, elem);
    }

    bool empty() const {
        return (fake_ptr->left == nullptr && fake_ptr->right == nullptr);
    }

    const_iterator lower_bound(T const &elem) const {
        return lower_bound_impl(static_cast<node *>(fake_ptr->left), elem);
    }

    const_iterator upper_bound(T const &elem) const {
        return upper_bound_impl(static_cast<node *>(fake_ptr->left), elem);
    }

    const_iterator erase(const_iterator pos) {
        if (pos == end()) {
            throw std::runtime_error("erase try in end()");
        }
        return erase_impl(static_cast<node *>(fake_ptr->left), static_cast<node *>(pos.ptr));
    }

    template<typename K>
    friend void swap(set<K> &a, set<K> &b);

private:
    const_iterator find_impl(base_node *cur, T const &elem) const {
        if (cur == fake_ptr) {
            cur = cur->left;
        }
        if (cur == nullptr) {
            return end();
        }
        if (static_cast<node *>(cur)->data == elem) {
            return const_iterator(*cur);
        }
        if (static_cast<node *>(cur)->data > elem) {
            return find_impl(cur->left, elem);
        }
        return find_impl(cur->right, elem);
    }

    const_iterator insert_impl(node *cur, T const &elem) {
        if (cur == nullptr) {
            fake_ptr->left = new node(elem, nullptr, nullptr, fake_ptr);
            return const_iterator(*fake_ptr->left);
        }
        node *in_node = new node(elem);
        while (cur != nullptr) {
            if (in_node->data > cur->data) {
                if (cur->right != nullptr) {
                    cur = static_cast<node *>(cur->right);
                } else {
                    in_node->parent = cur;
                    cur->right = in_node;
                    break;
                }
            } else if (in_node->data < cur->data) {
                if (cur->left != nullptr) {
                    cur = static_cast<node *>(cur->left);
                } else {
                    in_node->parent = cur;
                    cur->left = in_node;
                    break;
                }
            }
        }
        return const_iterator(*in_node);
    }

    const_iterator lower_bound_impl(node *cur, T const &elem) const {
        for (auto i = begin(); i != end(); ++i) {
            if (static_cast<node *>(i.ptr)->data >= elem) {
                return i;
            }
        }
        return end();
    }

    const_iterator upper_bound_impl(node *cur, T const &elem) const {
        for (auto i = begin(); i != end(); ++i) {
            if (static_cast<node *>(i.ptr)->data > elem) {
                return i;
            }
        }
        return end();
    }

    const_iterator erase_impl(node *cur, node *v) {
        auto p = v->parent;
        if (v->left == nullptr && v->right == nullptr) {
            //  Deleted element is a leaf
            if (p->left == v) {
                p->left = nullptr;
            }
            if (p->right == v) {
                p->right = nullptr;
            }
            delete v;
            return end();
        } else if (v->left == nullptr || v->right == nullptr) {
            //  Deleted element has exactly one child
            if (v->left == nullptr) {
                if (p->left == v)
                    p->left = v->right;
                else
                    p->right = v->right;
                v->right->parent = p;
            } else {
                if (p->left == v)
                    p->left = v->left;
                else
                    p->right = v->left;
                v->left->parent = p;
            }
            delete v;
            return const_iterator(*p);
        } else {
            //  Deleted element has exactly two children
            auto suc = static_cast<node *>(next(v));
            if (suc->parent->right == suc) {
                v->parent->left == v ? v->parent->left = suc : v->parent->right = suc;
                suc->parent = v->parent;
                suc->left = v->left;
                v->left->parent = suc;
                v->left = nullptr;
                v->right = nullptr;
                v->parent = nullptr;
                delete v;
                return const_iterator(*suc);
            } else {
                suc->parent->left = suc->right;
                if (suc->right != nullptr) {
                    suc->right->parent = suc->parent;
                }
                suc->right = nullptr;
                suc->parent = nullptr;
                v->parent->left == v ? v->parent->left = suc : v->parent->right = suc;
                suc->parent = v->parent;
                suc->right = v->right;
                suc->left = v->left;
                v->left->parent = suc;
                v->right->parent = suc;
                v->left = nullptr;
                v->right = nullptr;
                v->parent = nullptr;
                delete v;
                return const_iterator(*suc);
            }
        }

    }

    base_node *next(base_node *ptr) {
        if (ptr->right != nullptr) {
            return minimum(ptr->right);
        }
        base_node *tmp = ptr->parent;
        while (tmp && ptr == tmp->right) {
            ptr = tmp;
            tmp = tmp->parent;
        }
        return tmp;
    }

    base_node *minimum(base_node *cur) {
        if (cur->left == nullptr) {
            return cur;
        }
        return minimum(cur->left);
    }

    void destroy_subtree(base_node *cur) {
        if (cur == nullptr)
            return;
        destroy_subtree(cur->left);
        destroy_subtree(cur->right);
        delete (static_cast<node *>(cur));
    }

};

template<typename K>
void swap(set<K> &a, set<K> &b) {
    std::swap(a.fake_root, b.fake_root);
    if (a.fake_ptr->left) {
        a.fake_ptr->left->parent = &a.fake_root;
    }
    if (b.fake_ptr->left) {
        b.fake_ptr->left->parent = &b.fake_root;
    }
}


#endif  //EXAM_SET_H
