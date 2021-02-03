// Copyright
#ifndef SOURCE_B_LINK_HPP_
#define SOURCE_B_LINK_HPP_

#include <array>
#include <assert.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>

#define NODE_OVERFLOW 1
#define STILL_SAFE 0

namespace EDA {
  namespace Concurrent {
    //Using Visual Studio _DEBUG macro
    #ifdef _DEBUG
      #define Validate_expression(expression, message) assert(expression)
    #else
      #define Validate_expression(expression, message)                                             \
              try { if (!expression) throw std::runtime_error(message); }                          \
              catch (const std::exception& error_) {                                               \
                std::cerr << "[ERROR]: " << error_.what() << '\n';                                 \
                exit(EXIT_FAILURE);                                                                \
              }
    #endif

    template <std::size_t k, typename data_type, typename key_type>
    class BLinkTree {
      //BLink Components : NodeField, BLinkNode
      const std::size_t MAX_FIELDS_X_NODE = 2 * k;

      struct BLinkNode;

      struct NodeField {
        NodeField() {}

        //Non leaf's constructor
        NodeField(const key_type key_value,
          const std::shared_ptr<BLinkNode>& next_level,
          const std::shared_ptr<NodeField>& next_field
        ) : key_value_(key_value),
            next_level_(next_level),
            next_field_(next_field) {}

        //Leaf's constructor
        NodeField(const key_type key_value,
          std::unique_ptr<data_type>& data_value,
          const std::shared_ptr<NodeField>& next_field
        ) : key_value_(key_value),
            next_field_(next_field) {
          data_value_ = std::move(data_value);
        }

        key_type get_key() const noexcept { return key_value_; }

        std::shared_ptr<NodeField>& get_next_field() const noexcept { return next_field_; }

        std::shared_ptr<BLinkNode>& get_next_level() const noexcept { return next_level_; }

        const std::unique_ptr<data_type>& get_data() const noexcept { return data_value_; }

        void set_next_field(const std::shared_ptr<NodeField>& next_field) {
          next_field_ = next_field;
        }

      private:
        key_type key_value_;
        std::shared_ptr<BLinkNode> next_level_;
        std::shared_ptr<NodeField> next_field_;
        std::unique_ptr<data_type> data_value_;
      };

      struct BLinkNode {
        typedef std::reference_wrapper<std::shared_ptr<NodeField>> PtrFieldRef;

        std::mutex blink_locker;
        std::condition_variable working_on;

        BLinkNode(std::size_t level = 0) : level_(level), size_(0) {
          high_key = std::numeric_limits<key_type>::min();
        }

        const key_type get_high_key() noexcept { return high_key; }

        const std::size_t get_level() noexcept { return level_; }

        void lock() {
          doing_modification = true;
        }

        void unlock() {
          doing_modification = false;
        }
        
        bool is_leaf() { return (level_ == 0); }

        bool is_link_of(const std::shared_ptr<BLinkNode>& ptr_) const { 
          return (this == ptr_->link_pointer.get());
        }

        bool has(const key_type key_value) {
          std::unique_lock<std::mutex> process_locker(blink_locker);
          while (doing_modification) { working_on.wait(process_locker); }
          std::shared_ptr<NodeField> cfield = find_place(key_value).get();
          if(cfield != nullptr)
            if (cfield->get_key() == key_value)
              return true;//success
          return false;//failure
        }

        const std::unique_ptr<data_type>& find(const key_type key_value) {
          std::unique_lock<std::mutex> process_locker(blink_locker);
          while (doing_modification) { working_on.wait(process_locker); }
          std::shared_ptr<NodeField> cfield = find_place(key_value).get();
          if (cfield != nullptr) {
            if (cfield->get_key() == key_value)
              return cfield->get_data();//success
          }
          else {
            throw std::runtime_error("El dato buscado no se encuentra insertado.");
          }//failure
        }

        std::size_t size() const noexcept { 
          return size_; 
        }

        bool insert(const key_type key_value, 
                    const data_type& data_value) {
          std::unique_lock<std::mutex> process_locker(blink_locker);
          while (doing_modification) { working_on.wait(process_locker); }
          PtrFieldRef cfield_refw = find_place(key_value);
          std::unique_ptr<data_type> data_ptr(new data_type(data_value));
          cfield_refw.get() = std::make_shared<NodeField>(key_value, data_ptr,
                                         cfield_refw->get());
          update_high_key(key_value);
          return (++size_ > MAX_FIELDS_X_NODE);
        }

        bool insert(const key_type key_value, 
                    const std::shared_ptr<BLinkNode>& next_level) {
          std::unique_lock<std::mutex> process_locker(blink_locker);
          while (doing_modification) { working_on.wait(process_locker); }
          PtrFieldRef cfield_refw = find_place(key_value);
          cfield_refw.get() = std::make_shared<NodeField>(key_value, next_level,
                                          cfield_refw->get());
          update_high_key(key_value);
          return (++size_ > MAX_FIELDS_X_NODE);
        }

        void link_with(const std::shared_ptr<BLinkNode>& node_ptr) { 
          link_pointer = node_ptr; 
        }

        void swap(std::shared_ptr<BLinkNode>& node_ptr) {
          std::shared_ptr<NodeField> temp = head;
          head = node_ptr->head;
          node_ptr->head = temp;
        }

        std::shared_ptr<BLinkNode> scannode(const key_type key_value) {
          std::unique_lock<std::mutex> process_locker(blink_locker);
          while (doing_modification) { working_on.wait(process_locker); }
          std::shared_ptr<NodeField> cfield = find_place(key_value).get();
          return (cfield != nullptr) ? cfield->get_next_level() : link_pointer;
        }

      private:
        key_type high_key;
        std::shared_ptr<BLinkNode> link_pointer;
        std::shared_ptr<NodeField> head;
        PtrFieldRef last_fpb = std::ref(head);
        std::size_t size_;
        std::size_t level_;

        bool doing_modification;

        PtrFieldRef find_place(const key_type& key_value) {
          PtrFieldRef current_field_ref = std::ref(head);
          while (current_field_ref.get() != nullptr) {
            if (current_field_ref.get()->get_key() >= key_value)
              break;
            current_field_ref = std::ref(current_field_ref.get()->get_next_field());
          }
          return current_field_ref;
        }

        void push_back(const std::shared_ptr<NodeField>& next_field) {
          last_fpb.get() = next_field;
          last_fpb = std::ref(last_fpb.get()->get_next_field());
          high_key = next_field->get_key();
          ++size_;
        }

        void update_high_key(const key_type new_key) {
          high_key = std::max(new_key, high_key);
        }

        std::shared_ptr<BLinkNode> split() {
          std::shared_ptr<BLinkNode> other_half(get_level());
          std::size_t half_size = size_ / 2;
          size_ = size_ - size_ / 2;
          std::queue<std::shared_ptr<NodeField>> fn_fields;
          std::shared_ptr<NodeField> cfield = head;
          while (half_size--) {
            fn_fields.push(cfield);
            cfield = cfield->get_next_field();
          }
          head = cfield;
          while (!fn_fields.empty()) {
            fn_fields.front()->set_next_field(nullptr);
            other_half.push_back(fn_fields.front());
            fn_fields.pop();
          }
          other_half.swap(*this);
          return other_half;
        }
      };

    public:
      //TODO: Complete insertion and split
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

      bool has_key(const key_type key_value) {
        std::shared_ptr<BLinkNode> chosen = choose_node(key_value);
        return chosen->has(value);
      }

      data_type& search(const key_type key_value) const {
        std::shared_ptr<BLinkNode> chosen = choose_node(key_value);
        try {
          return *chosen->find(value);
        }
        catch (std::runtime_error& error) {//no fatal
          std::cerr << "[!]:" << error.what() << '\n';
        }
      }

      void insert(const key_type key_value, const data_type& data_value) {
        std::stack<std::shared_ptr<BLinkNode>> ancestors;
        std::shared_ptr<BLinkNode> current = root;
        std::shared_ptr<BLinkNode> temp_current;
        while (!current->is_leaf()) {
          temp_current = current;
          current = current->scannode(value);
          if (!current->is_link_of(temp_current))
            ancestors.push(temp_current);
        }//search process
        current->lock();
        std::unique_lock<std::mutex> process_locker(current->blink_locker);
        move_right(key_value, current);
        if (current->has(key_value)) {
          std::cerr << "[!]: La clave del dato a insertar ya existe en el arbol.\n";
          current->unlock();
          current->working_on.notify_one();
          return;
        }
        std::shared_ptr<BLinkNode> oldnode = current;
        bool tree_status = STILL_SAFE;
        //safe node
        while (!ancestors.empty()) {//Do insertion
          tree_status = (current->is_leaf()) ? (current->insert(key_value, data_value)) : 
                                    (current->insert(oldnode->get_high_key(), oldnode));
          switch (tree_status) {
          case STILL_SAFE:
            current->unlock();
            current->working_on.notify_one();
            break;
          case NODE_OVERFLOW:
            std::shared_ptr<BLinkNode> newnode = current->split();
            oldnode = current;
            current = ancestors.top();
            current->insert(newnode->get_high_key(), newnode);
            ancestors.pop();
            current->lock();
            move_right(current->get_high_key(), current);
            oldnode->unlock();
            oldnode->working_on.notify_one();
            break;
          }
        }
        if (tree_status == NODE_OVERFLOW) {//new root
          root->lock();
          std::shared_ptr<BLinkNode> newroot(root->get_level() + 1), newnode = root->split();
          newroot->insert(root->get_high_key(), root);
          newroot->insert(newnode->get_high_key(), newnode);
          root->unlock();
          root->working_on.notify_one();
        }
      }

      void remove(const key_type key_value) {}

    private:
      std::shared_ptr<BLinkNode> root;
      std::size_t size_of_tree;

      const std::shared_ptr<BLinkNode>& choose_node(const key_type key_value) const {
        std::shared_ptr<BLinkNode> cnode = root;
        while (!cnode->is_leaf()) {
          cnode = cnode->scannode(key_value);
        }
        for (std::shared_ptr<BLinkNode> tcnode = cnode->scannode(key_value);
                                                  tcnode->is_link_of(cnode);
                                                            cnode = tcnode) {}
        return cnode;
      }

      void move_right(const key_type key_value, std::shared_ptr<BLinkNode>& current) {
        for (std::shared_ptr<BLinkNode> t = current->scannode(key_value);
                                                  t->is_link_of(current);
                                                            current = t) {
          t->lock();
          current->unlock();
          current = t;
        }
      }
    };

  }  // namespace Concurrent
}  // namespace EDA

#endif  // SOURCE_B_LINK_HPP_
