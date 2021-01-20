// Copyright
#ifndef SOURCE_B_LINK_HPP_
#define SOURCE_B_LINK_HPP_

#include <array>
#include <assert.h>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>

namespace EDA {
  namespace Concurrent {

    //Using Visual Studio _DEBUG macro
    #ifdef _DEBUG
      #define Validate_expression(expression, message) assert(expression)
    #else
      #define Validate_expression(expression, message)\
              try { if (!expression) throw std::runtime_error(message); }\
              catch (const std::exception& error_) { std::cerr << "ERROR: " << error_.what() << '\n'; exit(1); }
    #endif

    template <std::size_t k, typename Type>
    class BLinkTree {
    private:
      typedef Type data_type;

      struct BLinkNode;
      struct NodeField {
        NodeField(data_type key, std::shared_ptr<BLinkNode> left) : key_value(key), next_level_(left) {}

        std::shared_ptr<BLinkNode> next_level_;
        data_type key_value;
      };

      struct BLinkNode {
      private:
        std::shared_ptr<BLinkNode> split() {
          return nullptr;
        }

      public:
        BLinkNode(std::size_t level = 0) : level_(level), size_(0) {}

        NodeField operator[](const std::size_t& position) {
          Validate_expression(position < 2 * k, "The given position is out of range of the node.");
          return NodeField(KEYS[position], SONS[position]);
        }

        bool is_leaf() { return (level_ == 0); }

        bool is_linkptr(std::shared_ptr<BLinkNode>& ptr_) const { return (link_pointer.get() == ptr_.get()); }

        bool key_is_stored(const data_type& key_value) {
          for (std::size_t i(0); i < size(); ++i)
            if (KEYS[i] == key_value)
              return true;//success
          return false;//failure
        }

        std::size_t size() { return size_; }

        void insert(const NodeField& entry) {
          if (size() == 2 * k)
            std::shared_ptr<BLinkNode> newnode = split();
          else {
            KEYS[size_] = entry.key_value;
            SONS[size_++] = entry.next_level_;
          }
        }

        void link_with(const std::shared_ptr<BLinkNode>& next) {
          link_pointer = next;
        }

        std::shared_ptr<BLinkNode>& scannode(const data_type& key_value) const {
          for (std::size_t i(0); i < size(); ++i) {
            NodeField& entry_ = (*this)[i];
            if (entry_.key_value >= key_value)
              return entry_.next_level_;
          }
          if (level_ != 0) {
            NodeField hk_field(high_key, size());
            if (high_key >= key_value)
              return hk_field.next_level_;
          }
          return link_pointer;
        }

      private:
        std::shared_ptr<BLinkNode> link_pointer;
        std::shared_ptr<data_type> high_key;
        std::array<data_type, 2 * k> KEYS;
        std::array<std::shared_ptr<BLinkNode>, 2 * k + 1> SONS;
        std::size_t size_;
        std::size_t level_;
      };

      void move_right() {}

    public:

      explicit BLinkTree() {
        Validate_expression(k != 0, "The value of k should not be zero.");
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
          temp_current = temp_current->scannode(value);
          if (!(current->is_linkptr(temp_current))) break;
          current = temp_current;
        }
        return current->key_is_stored(value);
      }

      void insert(const data_type& value) {
        std::stack<std::shared_ptr<BLinkNode>> ancestors;
        std::shared_ptr<BLinkNode> current = root;
        std::shared_ptr<BLinkNode> temp_current;
        while (!current->is_leaf()) {
          temp_current = current;
          current = current->scannode(value);
          if (!temp_current->is_linkptr(current))
            ancestors.push(temp_current);
        }
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
