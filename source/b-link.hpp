// Copyright
#ifndef SOURCE_B_LINK_HPP_
#define SOURCE_B_LINK_HPP_

#include <array>
#include <assert.h>
#include <memory>
#include <stack>
#include <utility>

namespace EDA {
namespace Concurrent {

template <std::size_t k, typename Type>
class BLinkTree {
private:
  typedef Type data_type;

  struct BLinkNode;
  struct NodeField {
    NodeField() {}

    bool on_leaf() const { return !(next_level.get()); }

    data_type get_key() const { return key_value; }

    std::shared_ptr<BLinkNode> get_ptr2nxtlvl() const { return next_level; }

  private:
    std::shared_ptr<BLinkNode> next_level;
    data_type key_value;
  };

  struct BLinkNode {
    BLinkNode() {}

    NodeField operator[](const std::size_t& position) {
      assert(position < 2 * k + 1);
      return entries[position];
    }

    bool is_leaf() {
      return (*this)[0].on_leaf();
    }

    bool is_linkptr(std::shared_ptr<BLinkNode>& ptr_) const { return (link_pointer.get() == ptr_.get()); }

    bool key_is_stored(const data_type& key_value) {
      for (NodeField& entry_ : entries)
        if (entry_.get_key() == key_value)
          return true;//success
      return false;//failure
    }

    std::size_t size() { return size_; }

    void insert(const NodeField& entry) {
      if (size() == 2 * k + 1)
        std::shared_ptr<BLinkNode> newnode = split();
      else
        entries[size_++] = entry;
    }

    std::shared_ptr<BLinkNode>& scannode(const data_type& key_value) const {
      for (NodeField& entry_ : entries)
        if (entry_.get_key() >= key_value)
          return entry_.get_ptr2nxtlvl();
      return link_pointer;
    }

    std::shared_ptr<BLinkNode> split() {
      return nullptr;
    }

  private:
    std::shared_ptr<BLinkNode> link_pointer;
    std::array<NodeField, 2 * k + 1> entries;
    std::size_t size_;
  };

public:

  explicit BLinkTree() {
    assert(k != 0);
    root = std::make_shared<BLinkNode>();
    size_of_tree = 0;
  }

  virtual ~BLinkTree() {
    root.reset();
  }

  std::size_t size() const { return size_of_tree; }

  bool empty() const {
    return (root->size() == 0);
  }

  bool search(const data_type& value) const {
    std::shared_ptr<BLinkNode> current = root;
    while (current.get()) {
      if (current->is_leaf()) break;
      current = current->scannode(value);
    }
    std::shared_ptr<BLinkNode> temp_current = current;
    while (true) {
      temp_current = temp_current.scannode(value);
      if (!(current->is_linkptr(temp_current))) break;
      current = temp_current;
    }
    return current->key_is_stored(value);
  }

  void insert(const data_type& value) {
    std::stack<std::shared_ptr<BLinkNode>> ancestors;
    std::shared_ptr<BLinkNode> current = root;
    /*...*/
  }

  void remove(const data_type& value) {}

private:
  std::shared_ptr<BLinkNode> root;
  std::size_t size_of_tree;
};

}  // namespace Concurrent
}  // namespace EDA

#endif  // SOURCE_B_LINK_HPP_
