#ifndef TURTLE_COMMON_CONFIG_H
#define TURTLE_COMMON_CONFIG_H

#include <cstdint>
#include <sys/types.h>

namespace turtle {

enum class FileFormat : uint8_t { Record = 0, Index = 1 };

using PageId = uint32_t;
using FrameId = uint32_t;
using FileId = uint32_t;

static constexpr PageId INVALID_PAGE_ID = static_cast<PageId>(-1);
static constexpr uint32_t PAGE_SIZE = 4096;

} // namespace turtle

#endif // !TURTLE_COMMON_CONFIG_H
