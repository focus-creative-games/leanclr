#include "pe_image_reader.h"

#include "alloc/general_allocation.h"
#include "utils/binary_reader.h"
#include "alloc/mem_pool.h"
#include "cli_image.h"

#include <cstring>
#include "core/stl_compat.h"

namespace leanclr
{
namespace metadata
{

constexpr uint32_t PE_LFANEW_OFFSET = 0x3c;
constexpr uint32_t PE_SIGNATURE_SIZE = 0x4;
constexpr uint16_t OPTIONAL_HEADER_SIZE32 = 224;
constexpr uint16_t OPTIONAL_HEADER_SIZE64 = 240;
constexpr uint32_t CLI_HEADER_OFFSET32 = 208;
constexpr uint32_t CLI_HEADER_OFFSET64 = 224;

#define RET_BAD_IMAGE_ON_FALSE(expr)        \
    do                                      \
    {                                       \
        if (!(expr))                        \
        {                                   \
            RET_ERR(RtErr::BadImageFormat); \
        }                                   \
    } while (0)

RtResult<CliImage*> PeImageReader::ReadCliImage(alloc::MemPool& imagePrivatePool)
{
    CliImage* imagePtr = imagePrivatePool.malloc_any_zeroed<CliImage>();
    if (!imagePtr)
    {
        return RtErr::OutOfMemory;
    }

    utils::BinaryReader reader(_image_base, _image_size);

    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position(PE_LFANEW_OFFSET));

    uint32_t lfanew;
    RET_BAD_IMAGE_ON_FALSE(reader.try_read_u32(lfanew));

    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position((size_t)lfanew));

    const char* ptr_sig;
    RET_BAD_IMAGE_ON_FALSE(reader.try_peek_any_ptr_range(PE_SIGNATURE_SIZE, ptr_sig));
    RET_BAD_IMAGE_ON_FALSE(std::memcmp(ptr_sig, "PE\0\0", PE_SIGNATURE_SIZE) == 0);
    RET_BAD_IMAGE_ON_FALSE(reader.try_advance(PE_SIGNATURE_SIZE));

    // Decode file records field by field. File offsets do not imply native
    // alignment, and a mapped image can itself begin at an unaligned address.
    const size_t pe_header_pos = reader.get_position();
    uint16_t section_count, optional_header_size;
    RET_BAD_IMAGE_ON_FALSE(reader.try_advance(2) && reader.try_read_u16(section_count));
    RET_BAD_IMAGE_ON_FALSE(reader.try_advance(12) && reader.try_read_u16(optional_header_size) && reader.try_advance(2));
    RET_BAD_IMAGE_ON_FALSE(optional_header_size == OPTIONAL_HEADER_SIZE32 || optional_header_size == OPTIONAL_HEADER_SIZE64);
    RET_BAD_IMAGE_ON_FALSE(reader.try_advance(optional_header_size));
    RET_BAD_IMAGE_ON_FALSE(section_count <= reader.remaining() / 40);

    CliSection* sections = imagePrivatePool.calloc_any<CliSection>(section_count);
    if (!sections && section_count != 0)
        RET_ERR(RtErr::OutOfMemory);
    for (uint16_t i = 0; i < section_count; ++i)
    {
        uint32_t virtual_size, virtual_address, raw_size, raw_offset;
        RET_BAD_IMAGE_ON_FALSE(reader.try_advance(8) && reader.try_read_u32(virtual_size) && reader.try_read_u32(virtual_address) &&
                               reader.try_read_u32(raw_size) && reader.try_read_u32(raw_offset) && reader.try_advance(16));
        RET_BAD_IMAGE_ON_FALSE(virtual_size <= UINT32_MAX - virtual_address);
        RET_BAD_IMAGE_ON_FALSE(raw_offset <= _image_size && raw_size <= _image_size - raw_offset);
        CliSection& section = sections[i];
        section.virtual_address_begin = virtual_address;
        section.virtual_address_end = virtual_address + virtual_size;
        section.raw_data_begin = raw_offset;
        section.raw_data_size = raw_size;
    }
    imagePtr->set_sections(sections, section_count);
    imagePtr->set_image_data(_image_base, _image_size);

    const uint32_t cli_header_entry = optional_header_size == OPTIONAL_HEADER_SIZE32 ? CLI_HEADER_OFFSET32 : CLI_HEADER_OFFSET64;
    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position(pe_header_pos + 20 + cli_header_entry));
    uint32_t cli_rva, cli_size;
    RET_BAD_IMAGE_ON_FALSE(reader.try_read_u32(cli_rva) && reader.try_read_u32(cli_size));
    const auto cli_offset = imagePtr->rva_to_image_offset(cli_rva);
    RET_BAD_IMAGE_ON_FALSE(cli_offset.has_value());
    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position(cli_offset.value()));
    uint32_t metadata_rva, metadata_size, entry_point;
    RET_BAD_IMAGE_ON_FALSE(reader.try_advance(8) && reader.try_read_u32(metadata_rva) && reader.try_read_u32(metadata_size) && reader.try_advance(4) &&
                           reader.try_read_u32(entry_point));
    imagePtr->set_entry_point_token(entry_point);

    const auto metadata_offset = imagePtr->rva_to_image_offset(metadata_rva);
    RET_BAD_IMAGE_ON_FALSE(metadata_offset.has_value());
    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position(metadata_offset.value()) && metadata_size <= reader.remaining());
    imagePtr->set_metadata_offset_length(metadata_offset.value(), metadata_size);

    RET_OK(imagePtr);
}

#undef RET_BAD_IMAGE_ON_FALSE
} // namespace metadata
} // namespace leanclr
