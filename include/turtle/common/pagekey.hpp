#include "turtle/common/config.hpp"
#include <functional>

namespace turtle {
  struct PageKey {
    FileId file_id;
    PageId page_id;

    bool operator==(const PageKey& other) const {
      return file_id == other.file_id && page_id == other.page_id;
    }
  };

  struct PageKeyHash {
    size_t operator()(const PageKey& key) const {
      return std::hash<uint64_t>()(
        (static_cast<uint64_t>(key.file_id) << 32) | key.page_id
      );
    }
  };
}
