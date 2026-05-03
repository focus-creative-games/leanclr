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

struct PEHeader
{
    uint16_t machine;
    uint16_t sections;
    uint32_t timestamp;
    uint32_t ptr_symbol_table;
    uint32_t num_symbols;
    uint16_t optional_header_size;
    uint16_t characteristics;
};

static_assert(sizeof(PEHeader) == 20, "PEHeader size mismatch");

struct PESectionHeader
{
    char name[8];
    uint32_t virtual_size;
    uint32_t virtual_address;
    uint32_t size_of_raw_data;
    uint32_t pointer_to_raw_data;
    uint32_t pointer_to_relocations;
    uint32_t pointer_to_line_numbers;
    uint16_t number_of_relocations;
    uint16_t number_of_line_numbers;
    uint32_t characteristics;
};

struct PEDirEntry
{
    uint32_t rva;
    uint32_t size;
};

struct CLIHeader
{
    uint32_t cb;
    uint16_t major_runtime_version;
    uint16_t minor_runtime_version;
    PEDirEntry meta_data;
    uint32_t flags;
    uint32_t entry_point_token;
};

constexpr uint32_t PE_LFANEW_OFFSET = 0x3c;
constexpr uint32_t PE_SIGNATURE_SIZE = 0x4;
constexpr uint16_t OPTIONAL_HEADER_SIZE32 = 224;
constexpr uint16_t OPTIONAL_HEADER_SIZE64 = 240;
constexpr uint32_t CLI_HEADER_OFFSET32 = 208;
constexpr uint32_t CLI_HEADER_OFFSET64 = 224;

#define RET_BAD_IMAGE_ON_FALSE(expr)               \
    do                                             \
    {                                              \
        if (!(expr))                               \
        {                                          \
            RET_ASSERT_ERR(RtErr::BadImageFormat); \
        }                                          \
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
    RET_BAD_IMAGE_ON_FALSE(reader.try_read_any<uint32_t>(lfanew));

    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position((size_t)lfanew));

    const char* ptr_sig;
    RET_BAD_IMAGE_ON_FALSE(reader.try_peek_any_ptr_range(PE_SIGNATURE_SIZE, ptr_sig));
    RET_BAD_IMAGE_ON_FALSE(std::memcmp(ptr_sig, "PE\0\0", PE_SIGNATURE_SIZE) == 0);
    RET_BAD_IMAGE_ON_FALSE(reader.try_advance(PE_SIGNATURE_SIZE));

    RET_BAD_IMAGE_ON_FALSE(reader.is_aligned(4));

    const PEHeader* pe_header;
    RET_BAD_IMAGE_ON_FALSE(reader.try_peek_any_ptr<PEHeader>(pe_header));
    const size_t pe_header_pos = reader.get_position();

    RET_BAD_IMAGE_ON_FALSE(pe_header->optional_header_size == OPTIONAL_HEADER_SIZE32 || pe_header->optional_header_size == OPTIONAL_HEADER_SIZE64);

    RET_BAD_IMAGE_ON_FALSE(reader.try_advance(sizeof(PEHeader) + pe_header->optional_header_size));

    RET_BAD_IMAGE_ON_FALSE(reader.is_aligned(4));
    const PESectionHeader* headers;
    RET_BAD_IMAGE_ON_FALSE(reader.try_peek_any_ptr_range(pe_header->sections, headers));

    CliSection* sections = imagePrivatePool.calloc_any<CliSection>(pe_header->sections);

    for (uint16_t i = 0; i < pe_header->sections; ++i)
    {
        const PESectionHeader& header = headers[i];
        CliSection& section = sections[i];
        section.virtual_address_begin = header.virtual_address;
        section.virtual_address_end = header.virtual_address + header.virtual_size;
        section.raw_data_begin = header.pointer_to_raw_data;
    }
    imagePtr->set_sections(sections, pe_header->sections);
    imagePtr->set_image_data(_image_base, _image_size);

    const uint32_t cli_header_entry = pe_header->optional_header_size == OPTIONAL_HEADER_SIZE32 ? CLI_HEADER_OFFSET32 : CLI_HEADER_OFFSET64;
    const size_t dir_entry_pos = pe_header_pos + 20 + static_cast<size_t>(cli_header_entry);

    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position(dir_entry_pos));
    const PEDirEntry* pedir_entries;
    RET_BAD_IMAGE_ON_FALSE(reader.try_peek_any_ptr(pedir_entries));

    std::optional<uint32_t> cli_header_file_offset = imagePtr->rva_to_image_offset(pedir_entries->rva);
    RET_BAD_IMAGE_ON_FALSE(cli_header_file_offset.has_value());

    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position(cli_header_file_offset.value()));

    const CLIHeader* cli_header;
    RET_BAD_IMAGE_ON_FALSE(reader.try_peek_any_ptr<CLIHeader>(cli_header));

    imagePtr->set_entry_point_token(cli_header->entry_point_token);

    std::optional<uint32_t> metadata_file_offset = imagePtr->rva_to_image_offset(cli_header->meta_data.rva);
    RET_BAD_IMAGE_ON_FALSE(metadata_file_offset.has_value());
    RET_BAD_IMAGE_ON_FALSE(reader.try_set_position(metadata_file_offset.value()));

    const uint8_t* metadata_start;
    RET_BAD_IMAGE_ON_FALSE(reader.try_peek_any_ptr_range(cli_header->meta_data.size, metadata_start));

    imagePtr->set_metadata_offset_length(metadata_file_offset.value(), cli_header->meta_data.size);

    RET_OK(imagePtr);
}

#undef RET_BAD_IMAGE_ON_FALSE
} // namespace metadata
} // namespace leanclr
