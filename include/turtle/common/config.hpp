#ifndef TURTLE_COMMON_CONFIG_H
#define TURTLE_COMMON_CONFIG_H

#include <cstdint>
#include <sys/types.h>

namespace turtle {

enum class FileFormat : uint8_t { Record = 0, Index = 1 };

using PageId = uint32_t;
using TableOid = uint32_t;
using IndexId = uint32_t;
using FrameId = uint32_t;
using FileId = uint32_t;
using TxnId = uint64_t;
using TimestampT = int64_t;

static constexpr uint32_t PAGE_SIZE = 4096;

static constexpr PageId INVALID_PAGE_ID = static_cast<PageId>(-1);
static constexpr FrameId INVALID_FRAME_ID =
    static_cast<FrameId>(-1); // invalid frame id
static constexpr TxnId INVALID_TXN_ID =
    static_cast<TxnId>(-1); // invalid transaction id
static constexpr TimestampT INVALID_TS = static_cast<TimestampT>(-1);
// static constexpr int INVALID_LSN = -1;       // invalid log sequence number

} // namespace turtle

#endif // !TURTLE_COMMON_CONFIG_H
