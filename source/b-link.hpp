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
      #define Validate_expression(expression, message)                      \
              try { if (!expression) throw std::runtime_error(message); }   \
              catch (const std::exception& error_) {                        \
                std::cerr << "ERROR: " << error_.what() << '\n';            \
                exit(EXIT_FAILURE);                                         \
              }
    #endif

    template <std::size_t k, typename data_type, typename key_type>
    class BLinkTree {
      //BLink Components : NodeField, BLinkNode
      struct BLinkNode;

      struct NodeField {
        NodeField() {}

        NodeField(const key_type& key_value,
          std::shared_ptr<BLinkNode>& next_level,
          std::unique_ptr<NodeField>& next_field
        ) : key_value_(key_value),
            next_level_(next_level),
            next_field_(std::move(next_field)) {}

        NodeField(const key_type& key_value,
          std::unique_ptr<data_type>& data_value,
          std::unique_ptr<NodeField>& next_field
        ) : key_value_(key_value),
            next_field_(next_field),
            data_value_(std::move(data_value)) {}

        key_type key_value_;
        std::shared_ptr<BLinkNode> next_level_;
        std::unique_ptr<data_type> data_value_;
        std::unique_ptr<NodeField> next_field_;
      };
      //TODO: Refactory BLinkNode
      struct BLinkNode {
        BLinkNode(std::size_t level = 0) : level_(level), size_(0) {}
        
        bool is_leaf() { return (level_ == 0); }

        bool is_linkptr(const std::shared_ptr<BLinkNode>& ptr_) const { return (link_pointer.get() == ptr_.get()); }

        bool key_is_stored(const key_type& key_value) {
          
          return false;//failure
        }

        std::size_t size() const noexcept { return size_; }

        void insert(const NodeField& entry) {
          
        }

        void link_with(const std::shared_ptr<BLinkNode>& node_ptr) {
          link_pointer = node_ptr;
        }

        std::shared_ptr<BLinkNode>& scannode(const key_type& key_value) const {
          
          return link_pointer;
        }

      private:
        key_type high_key;
        std::shared_ptr<BLinkNode> link_pointer;
        std::unique_ptr<NodeField> head;
        std::size_t size_;
        std::size_t level_;
        bool is_locked;

        std::shared_ptr<BLinkNode> split() {
          return nullptr;
        }
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
