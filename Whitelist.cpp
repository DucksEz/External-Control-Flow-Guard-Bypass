#include <cstdint>

#define RELOC_FLAG(RelInfo) (((RelInfo) >> 12) == IMAGE_REL_BASED_DIR64)

#define KPageHash     0x3397FC9F
#define KByteHash     0x46

#define HashPage(Page) \
    ((((uintptr_t)(Page) >> 12) ^ KPageHash))

#define ValidationByte(Page) \
    ((((uintptr_t)(Page) >> 44) ^ KByteHash))

#define WhitelistRegion(Start, Size)                                                      \
{                                                                                              \
    uint8_t stack_block[0x40] = {};                                                            \
    uintptr_t AlignedStart = (uintptr_t)(Start) & ~0xFFFULL;                                   \
    uintptr_t AlignedEnd   = ((uintptr_t)(Start) + (Size) + 0xFFFULL) & ~0xFFFULL;             \
    for (uintptr_t Page = AlignedStart; Page < AlignedEnd; Page += 0x1000)                     \
    {                                                                                          \
        uint32_t page_hash = HashPage(Page);                                                   \
        uint8_t validation = ValidationByte(Page);                                             \
        *reinterpret_cast<uint32_t*>(stack_block + 0x18) = page_hash;                          \
        *reinterpret_cast<uint8_t*>(stack_block + 0x1C) = validation;                          \
        insert_set(whitelist,                                                                  \
                   stack_block + 0x28,                                                         \
                   stack_block + 0x18);                                                        \
    }                                                                                          \
}

// Example Of Usage WhitelistRegion((std::uintptr_t)pBase, pOpt->SizeOfImage);
