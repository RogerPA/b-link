// Copyright
#ifndef SOURCE_B_LINK_HPP_
#define SOURCE_B_LINK_HPP_

#include <utility>

namespace EDA {
namespace Concurrent {

template <std::size_t B, typename Type>
class BLinkTree {
 public:
  typedef Type data_type;

  BLinkTree() {}

  ~BLinkTree() {}

  std::size_t size() const {}

  bool empty() const {}

  bool search(const data_type& value) const {}

  void insert(const data_type& value) {}

  void remove(const data_type& value) {}

 private:
  data_type* data_;
};

}  // namespace Concurrent
}  // namespace EDA

#endif  // SOURCE_B_LINK_HPP_
