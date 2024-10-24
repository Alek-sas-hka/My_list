#include <algorithm>
#include <memory>

// BaseNode

class BaseNode {
 public:
  BaseNode() : following(this), previous(this) {}

  ~BaseNode() = default;

  BaseNode& operator=(const BaseNode& other) = default;

  BaseNode* previous;
  BaseNode* following;
};

// Node

template <typename T>
class Node : public BaseNode {
 private:
  T value_;

 public:
  T& get_value() { return value_; }

  Node() = default;

  Node(const T& val) : value_(val), BaseNode() {}

  ~Node() = default;

  Node& operator=(const BaseNode& src) {
    previous = src.previous;
    following = src.following;
    return *this;
  }

  Node& operator=(const Node& src) {
    previous = src.previous;
    following = src.following;
    value_ = src.value_;
    return *this;
  }
};

// List

template <typename T, typename Allocator = std::allocator<T>>
class List {
 private:
  using alloc_traits = std::allocator_traits<Allocator>;
  using node_alloc_traits =
      typename alloc_traits::template rebind_traits<Node<T>>;
  typename alloc_traits::template rebind_alloc<Node<T>> node_alloc_;
  BaseNode fictitious_;
  Allocator list_alloc_;
  size_t size_ = 0;

 public:
  template <bool IsConst, bool IsReversed>
  class Iterator;
  using const_reverse_iterator = Iterator<true, true>;
  using value_type = T;
  using iterator = Iterator<false, false>;
  using allocator_type = Allocator;
  using reverse_iterator = Iterator<false, true>;
  using const_iterator = Iterator<true, false>;

  iterator end() { return iterator(const_cast<BaseNode*>(&fictitious_)); }

  const_iterator cbegin() const {
    return const_iterator(fictitious_.following);
  }

  void swap(List& first, List& second) {
    std::swap(first.fictitious_, second.fictitious_);
    std::swap(first.size_, second.size_);
  }

  iterator begin() { return iterator(fictitious_.following); }

  void remove_one(size_t& idx, auto& node) {
    node_alloc_traits::destroy(node_alloc_,
                               static_cast<Node<T>*>(node->previous));
    node_alloc_traits::deallocate(node_alloc_,
                                  static_cast<Node<T>*>(node->previous), 1);
    ++idx;
    node = static_cast<Node<T>*>(node->following);
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(fictitious_.previous);
  }

  List& operator=(const List<T, Allocator>& src) {
    List<T, Allocator> tmp(src);
    swap(*this, tmp);
    fictitious_.previous->following = &fictitious_;
    fictitious_.following->previous = &fictitious_;

    if (list_alloc_ != src.list_alloc_ &&
        node_alloc_traits::propagate_on_container_copy_assignment::value) {
      list_alloc_ = src.list_alloc_;
    }
    if (!tmp.empty()) {
      tmp.fictitious_.previous->following = &tmp.fictitious_;
      tmp.fictitious_.following->previous = &tmp.fictitious_;
    }
    if (tmp.empty()) {
      tmp.fictitious_.previous = &tmp.fictitious_;
      tmp.fictitious_.following = &tmp.fictitious_;
    }
    return *this;
  }

  reverse_iterator rbegin() { return reverse_iterator(fictitious_.previous); }

  List(const List<T, Allocator>& src) {
    Node<T>* actual = node_alloc_traits::allocate(node_alloc_, 1);
    auto new_end = src.end();
    auto new_begin = src.begin();
    try {
      node_alloc_traits::construct(node_alloc_, actual, *new_begin);
      fictitious_.following = actual;
      actual->previous = static_cast<Node<T>*>(&fictitious_);
      ++new_begin;
      ++size_;
      for (; new_begin != new_end; ++new_begin) {
        actual->following = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(
            node_alloc_, static_cast<Node<T>*>(actual->following), *new_begin);
        ++size_;
        actual->following->previous = actual;
        actual = static_cast<Node<T>*>(actual->following);
      }
      actual->following = static_cast<Node<T>*>(&fictitious_);
      fictitious_.previous = actual;
    } catch (...) {
      remove(size_ - 1);
      dealloc(actual);
      throw;
    }

    list_alloc_ =
        alloc_traits::select_on_container_copy_construction(src.list_alloc_);
    node_alloc_ =
        alloc_traits::select_on_container_copy_construction(src.node_alloc_);
  }

  const_iterator begin() const { return const_iterator(fictitious_.following); }

  List(size_t count, const T& value, const Allocator& alloc = Allocator()) {
    Node<T>* actual = node_alloc_traits::allocate(node_alloc_, 1);
    list_alloc_ = alloc;
    try {
      node_alloc_traits::construct(node_alloc_, actual, value);
      ++size_;
      fictitious_.following = actual;
      actual->previous = static_cast<Node<T>*>(&fictitious_);

      for (size_t i = 0; i < count - 1; ++i) {
        actual->following = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(
            node_alloc_, static_cast<Node<T>*>(actual->following), value);
        ++size_;
        actual->following->previous = actual;
        actual = static_cast<Node<T>*>(actual->following);
      }
    } catch (...) {
      remove(size_ - 1);
      dealloc(actual);
      throw;
    }

    actual->following = static_cast<Node<T>*>(&fictitious_);
    fictitious_.previous = actual;
  }

  const_iterator cend() const {
    return const_iterator(const_cast<BaseNode*>(&fictitious_));
  }

  List() = default;

  const_iterator end() const {
    return const_iterator(const_cast<BaseNode*>(&fictitious_));
  }

  explicit List(size_t count, const Allocator& alloc = Allocator()) {
    Node<T>* actual = node_alloc_traits::allocate(node_alloc_, 1);
    list_alloc_ = alloc;
    try {
      node_alloc_traits::construct(node_alloc_, actual);
      fictitious_.following = actual;
      actual->previous = static_cast<Node<T>*>(&fictitious_);
      ++size_;
      for (size_t i = 0; i < count - 1; ++i) {
        actual->following = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(node_alloc_,
                                     static_cast<Node<T>*>(actual->following));
        ++size_;
        actual->following->previous = actual;
        actual = static_cast<Node<T>*>(actual->following);
      }
    } catch (...) {
      remove(size_ - 1);
      dealloc(actual);
      throw;
    }
    actual->following = static_cast<Node<T>*>(&fictitious_);
    fictitious_.previous = actual;
  }

  reverse_iterator rend() {
    return reverse_iterator(const_cast<BaseNode*>(&fictitious_));
  }

  void remove(size_t deleted_size) {
    size_t idx = 0;
    Node<T>* actual = static_cast<Node<T>*>(fictitious_.following->following);

    while (idx < deleted_size) {
      remove_one(idx, actual);
    }
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(fictitious_.previous);
  }

  void dealloc(Node<T>* actual) {
    node_alloc_traits::deallocate(node_alloc_,
                                  static_cast<Node<T>*>(actual->following), 1);
    node_alloc_traits::destroy(node_alloc_, actual);
    node_alloc_traits::deallocate(node_alloc_, actual, 1);
  }

  List(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {
    auto init_end = init.end();
    auto iter = init.begin();
    list_alloc_ = alloc;
    Node<T>* actual = node_alloc_traits::allocate(node_alloc_, 1);
    try {
      node_alloc_traits::construct(node_alloc_, actual, *iter);
      fictitious_.following = actual;
      actual->previous = static_cast<Node<T>*>(&fictitious_);
      ++size_;
      ++iter;
      for (; iter < init_end; ++iter) {
        actual->following = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(
            node_alloc_, static_cast<Node<T>*>(actual->following), *iter);
        ++size_;
        actual->following->previous = actual;
        actual = static_cast<Node<T>*>(actual->following);
      }
      actual->following = static_cast<Node<T>*>(&fictitious_);
      fictitious_.previous = actual;
    } catch (...) {
      remove(size_ - 1);
      dealloc(actual);
      throw;
    }
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(const_cast<BaseNode*>(&fictitious_));
  }

  ~List() { remove(size_); }  //

  size_t size() const { return size_; }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(const_cast<BaseNode*>(&fictitious_));
  }

  void pop_front() noexcept {
    fictitious_.following = fictitious_.following->following;
    node_alloc_traits::destroy(
        node_alloc_, static_cast<Node<T>*>(fictitious_.following->previous));
    --size_;
    node_alloc_traits::deallocate(
        node_alloc_, static_cast<Node<T>*>(fictitious_.following->previous), 1);
    fictitious_.following->previous = &fictitious_;
  }

  const T& front() const {
    return static_cast<Node<T>>(fictitious_.following).get_value();
  }

  void push_front(const T& value) {
    Node<T>* tmp = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, tmp, value);
    } catch (...) {
      node_alloc_traits::deallocate(node_alloc_, tmp, 1);
    }

    tmp->following = static_cast<Node<T>*>(fictitious_.following);
    fictitious_.following->previous = tmp;
    ++size_;
    tmp->previous = static_cast<Node<T>*>(&fictitious_);
    fictitious_.following = tmp;
  }

  void push_front(T&& value) {
    Node<T>* tmp = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, tmp, value);
    } catch (...) {
      node_alloc_traits::deallocate(node_alloc_, tmp, 1);
    }

    tmp->following = static_cast<Node<T>*>(fictitious_.following);
    fictitious_.following->previous = tmp;
    ++size_;
    tmp->previous = static_cast<Node<T>*>(&fictitious_);
    fictitious_.following = tmp;
  }

  T& front() { return static_cast<Node<T>>(fictitious_.following).get_value(); }

  void pop_back() noexcept {
    fictitious_.previous = fictitious_.previous->previous;
    node_alloc_traits::destroy(
        node_alloc_, static_cast<Node<T>*>(fictitious_.previous->following));
    --size_;
    node_alloc_traits::deallocate(
        node_alloc_, static_cast<Node<T>*>(fictitious_.previous->following), 1);
    fictitious_.previous->following = &fictitious_;
  }

  void push_back(T&& value) {
    Node<T>* tmp = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, tmp, value);
    } catch (...) {
      node_alloc_traits::deallocate(node_alloc_, tmp, 1);
    }

    ++size_;
    tmp->previous = static_cast<Node<T>*>(fictitious_.previous);
    fictitious_.previous->following = tmp;
    tmp->following = static_cast<Node<T>*>(&fictitious_);
    fictitious_.previous = tmp;
  }

  T& back() { return static_cast<Node<T>>(fictitious_.previous).get_value(); }

  void push_back(const T& value) {
    Node<T>* tmp = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, tmp, value);
    } catch (...) {
      node_alloc_traits::deallocate(node_alloc_, tmp, 1);
    }
    ++size_;
    tmp->previous = static_cast<Node<T>*>(fictitious_.previous);
    fictitious_.previous->following = tmp;
    tmp->following = static_cast<Node<T>*>(&fictitious_);
    fictitious_.previous = tmp;
  }

  const T& back() const {
    return static_cast<Node<T>>(fictitious_.previous).get_value();
  }

  Allocator get_allocator() const { return list_alloc_; }

  bool empty() const { return size_ == 0; }
};

// Iterator

template <typename T, typename Allocator>
template <bool IsConst, bool IsReversed>
class List<T, Allocator>::Iterator {
 private:
  Node<T>* actual_;

 public:
  using difference_type = std::ptrdiff_t;
  using is_const = std::conditional_t<IsConst, const T, T>;
  using value_type = std::remove_cv_t<T>;
  using iterator_category = std::bidirectional_iterator_tag;

  using pointer = is_const*;
  pointer operator->() const { return &(actual_->get_value()); }

  using reference = is_const&;
  reference operator*() const { return actual_->get_value(); }

  Iterator& operator++() {
    if (!IsReversed) {
      actual_ = static_cast<Node<T>*>(actual_->following);
    }
    if (IsReversed) {
      actual_ = static_cast<Node<T>*>(actual_->previous);
    }
    return *this;
  }

  Iterator& operator--() {
    if (!IsReversed) {
      actual_ = static_cast<Node<T>*>(actual_->previous);
    }
    if (IsReversed) {
      actual_ = static_cast<Node<T>*>(actual_->following);
    }
    return *this;
  }

  Iterator() = default;

  Iterator(BaseNode* node) : actual_(static_cast<Node<T>*>(node)) {}

  ~Iterator() = default;

  bool operator!=(const Iterator<IsConst, IsReversed>& compared) const {
    return actual_ != compared.actual_;
  }

  bool operator==(const Iterator<IsConst, IsReversed>& compared) const {
    return actual_ == compared.actual_;
  }

  Iterator operator++(int) {
    auto temp(*this);
    ++*this;
    return temp;
  }

  Iterator operator--(int) {
    auto temp(*this);
    --*this;
    return temp;
  }
};
