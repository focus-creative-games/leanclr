#include <algorithm>
#include <cstring>

#include "cli_image.h"

#include "utils/binary_reader.h"
#include "alloc/mem_pool.h"
#include "utils/rt_vector.h"
namespace leanclr
{
namespace metadata
{
using utils::BinaryReader;

// Constants

// Tag bits for coded indices
struct TagBits
{
    static constexpr uint32_t RESOLUTION_SCOPE = 2;
    static constexpr uint32_t TYPE_DEF_OR_REF = 2;
    static constexpr uint32_t HAS_CUSTOM_ATTRIBUTE = 5;
    static constexpr uint32_t CUSTOM_ATTRIBUTE_TYPE = 3;
    static constexpr uint32_t HAS_FIELD_MARSHAL = 1;
    static constexpr uint32_t HAS_DECL_SECURITY = 2;
    static constexpr uint32_t MEMBER_REF_PARENT = 3;
    static constexpr uint32_t HAS_CONSTANT = 2;
    static constexpr uint32_t HAS_SEMANTICS = 1;
    static constexpr uint32_t METHOD_DEF_OR_REF = 1;
    static constexpr uint32_t MEMBER_FORWARDED = 1;
    static constexpr uint32_t IMPLEMENTATION = 2;
    static constexpr uint32_t TYPE_OR_METHOD_DEF = 1;
    static constexpr uint32_t HAS_CUSTOM_DEBUG_INFORMATION = 4;
};

// Tables used for coded index calculations
static const TableType HAS_CUSTOM_ATTRIBUTE_ASSOCIATE_TABLES[] = {
    TableType::Method,           TableType::Field,         TableType::TypeRef,
    TableType::TypeDef,          TableType::Param,         TableType::InterfaceImpl,
    TableType::MemberRef,        TableType::Module,        TableType::DeclSecurity,
    TableType::Property,         TableType::Event,         TableType::StandaloneSig,
    TableType::ModuleRef,        TableType::TypeSpec,      TableType::Assembly,
    TableType::AssemblyRef,      TableType::File,          TableType::ExportedType,
    TableType::ManifestResource, TableType::GenericParam,  TableType::GenericParamConstraint,
    TableType::MethodSpec,       TableType::Document,      TableType::LocalScope,
    TableType::LocalVariable,    TableType::LocalConstant, TableType::ImportScope,
};

static const TableType HAS_CUSTOM_DEBUG_INFORMATION[] = {
    TableType::Method,        TableType::Field,        TableType::TypeRef,          TableType::TypeDef,      TableType::Param,
    TableType::InterfaceImpl, TableType::MemberRef,    TableType::Module,           TableType::Property,     TableType::Event,
    TableType::StandaloneSig, TableType::ModuleRef,    TableType::TypeSpec,         TableType::Assembly,     TableType::AssemblyRef,
    TableType::File,          TableType::ExportedType, TableType::ManifestResource, TableType::GenericParam, TableType::GenericParamConstraint,
    TableType::MethodSpec,
};

// Metadata structures
struct MetadataRootPartial
{
    uint32_t signature;
    uint16_t major_version;
    uint16_t minor_version;
    uint32_t reserved;
    uint32_t length;
    uint8_t version_first_byte;
};

struct StreamHeader
{
    uint32_t offset;
    uint32_t size;
    uint8_t name[1]; // Variable length
};

RtResultVoid CliImage::load_streams()
{
    if (metadata_offset >= image_length)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    BinaryReader reader(image_data + metadata_offset, metadata_length);

    // Read metadata root header
    const MetadataRootPartial* meta_root = nullptr;
    if (!reader.try_peek_any_ptr(meta_root))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    if (meta_root->signature != 0x424A5342) // "BSJB"
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Skip version string (aligned to 4-byte boundary)
    if (!reader.try_advance(meta_root->length + 16 + 2))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Read stream header count
    uint16_t stream_header_count;
    if (!reader.try_read_u16(stream_header_count))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Process each stream header
    for (uint16_t i = 0; i < stream_header_count; ++i)
    {
        // Read offset and size
        uint32_t stream_offset;
        uint32_t stream_size;
        if (!reader.try_read_u32(stream_offset) || !reader.try_read_u32(stream_size))
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        // Peek at the stream name (null-terminated string)
        const char* heap_name = nullptr;
        if (!reader.try_peek_any_ptr(heap_name))
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        // Calculate name length (including null terminator)
        size_t name_len = std::strlen(heap_name);
        if (name_len > MAX_STREAM_NAME_LENGTH)
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        // Align position to 4-byte boundary after name
        size_t aligned_name_len = (name_len / 4 + 1) * 4;
        if (!reader.try_advance(aligned_name_len))
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        // Validate stream data is within bounds
        if (stream_offset + stream_size > metadata_length)
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        // Find matching heap and set its data
        CliHeap* cur_heap = nullptr;
        if (std::strcmp(heap_name, "#~") == 0)
            cur_heap = &tables_heap;
        else if (std::strcmp(heap_name, "#Strings") == 0)
            cur_heap = &string_heap;
        else if (std::strcmp(heap_name, "#US") == 0)
            cur_heap = &us_heap;
        else if (std::strcmp(heap_name, "#GUID") == 0)
            cur_heap = &guid_heap;
        else if (std::strcmp(heap_name, "#Blob") == 0)
            cur_heap = &blob_heap;
        else if (std::strcmp(heap_name, "#Pdb") == 0)
            cur_heap = &pdb_heap;

        if (!cur_heap)
            continue; // Unknown heap, skip

        cur_heap->name = reinterpret_cast<const uint8_t*>(heap_name);
        cur_heap->data = image_data + metadata_offset + stream_offset;
        cur_heap->size = stream_size;
    }

    RET_VOID_OK();
}

RtResultVoid CliImage::load_tables(alloc::MemPool& pool)
{
    if (!tables_heap.data || tables_heap.size < TABLE_HEAP_HEADER_SIZE)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    BinaryReader reader(tables_heap.data, tables_heap.size);

    // Skip reserved field (4 bytes)
    uint32_t reserved;
    if (!reader.try_read_u32(reserved))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Read version info
    uint8_t major_version;
    uint8_t minor_version;
    if (!reader.try_read_byte(major_version) || !reader.try_read_byte(minor_version))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    if (major_version != 2 || minor_version != 0)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Read heap sizes flags
    uint8_t heap_sizes;
    if (!reader.try_read_byte(heap_sizes))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    if ((heap_sizes & ~0x7) != 0)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    string_heap_size_4_byte = (heap_sizes & 0x1) != 0;
    guid_heap_size_4_byte = (heap_sizes & 0x2) != 0;
    blob_heap_size_4_byte = (heap_sizes & 0x4) != 0;

    // Skip reserved byte
    uint8_t reserved2;
    if (!reader.try_read_byte(reserved2))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Read valid table bits
    uint64_t valid_table_bits;
    if (!reader.try_read_u64(valid_table_bits))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    if ((valid_table_bits & ~((1ULL << MAX_TABLE_COUNT) - 1)) != 0)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    uint32_t valid_table_count = utils::MemOp::get_not_zero_bit_count(valid_table_bits);
    if (valid_table_count == 0)
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Read sorted table bits (not used currently)
    uint64_t sorted_table_bits;
    if (!reader.try_read_u64(sorted_table_bits))
        RET_ASSERT_ERR(RtErr::BadImageFormat);

    // Read row counts for all valid tables
    utils::Vector<uint32_t> row_counts(valid_table_count);
    for (uint32_t i = 0; i < valid_table_count; ++i)
    {
        if (!reader.try_read_u32(row_counts[i]))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    // Initialize valid tables and row counts
    uint32_t cur_valid_table_index = 0;
    for (size_t table_index = 0; table_index < MAX_TABLE_COUNT; ++table_index)
    {
        if ((valid_table_bits & (1ULL << table_index)) != 0)
        {
            CliTableMeta& cur = tables[table_index];
            cur.valid = true;
            cur.row_count = row_counts[cur_valid_table_index];
            cur_valid_table_index++;
        }
    }

    // Initialize table field metadata
    init_table_field_metas(pool);
    init_table_metas_final();

    // Set data pointers for each table
    for (size_t table_index = 0; table_index < MAX_TABLE_COUNT; ++table_index)
    {
        CliTableMeta& cur = tables[table_index];
        if (!cur.valid)
            continue;

        const uint8_t* table_data = nullptr;
        if (!reader.try_peek_any_ptr(table_data))
            RET_ASSERT_ERR(RtErr::BadImageFormat);

        cur.data = table_data;
        if (!reader.try_advance(cur.row_count * cur.total_field_size))
            RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    RET_VOID_OK();
}

std::optional<RidRange> CliImage::find_row_range_of_owner_at_sorted_table(TableType table_index, uint8_t field_index, uint32_t owner) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(table_index)];
    if (!table.valid || table.row_field_count == 0 || field_index >= table.row_field_count)
        return std::nullopt;

    const CliTableFieldMeta& field_meta = table.row_fields[field_index];
    if (!(field_meta.size == 2 || field_meta.size == 4))
        return std::nullopt;

    const uint8_t* data = table.data;
    uint32_t rows = table.row_count;
    uint32_t begin = 0;
    uint32_t end = rows;

    while (begin < end)
    {
        uint32_t mid = (begin + end) >> 1;
        const uint8_t* mid_row = data + static_cast<size_t>(mid) * table.total_field_size;
        uint32_t mid_owner = read_column_u32(mid_row, field_meta);
        if (mid_owner < owner)
            begin = mid + 1;
        else
            end = mid;
    }

    if (begin >= rows)
        return std::nullopt;

    const uint8_t* first_row = data + static_cast<size_t>(begin) * table.total_field_size;
    uint32_t first_owner = read_column_u32(first_row, field_meta);
    if (first_owner != owner)
        return std::nullopt;

    uint32_t last = begin + 1;
    while (last < rows)
    {
        const uint8_t* cur_row = data + static_cast<size_t>(last) * table.total_field_size;
        uint32_t cur_owner = read_column_u32(cur_row, field_meta);
        if (cur_owner != owner)
            break;
        ++last;
    }

    return RidRange{begin + 1, last + 1};
}

std::optional<uint32_t> CliImage::find_row_of_owner(TableType table_index, uint8_t field_index, uint32_t owner) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(table_index)];
    if (!table.valid || table.row_field_count == 0 || field_index >= table.row_field_count)
        return std::nullopt;

    const CliTableFieldMeta& field_meta = table.row_fields[field_index];
    if (!(field_meta.size == 2 || field_meta.size == 4))
        return std::nullopt;

    const uint8_t* data = table.data;
    uint32_t rows = table.row_count;
    uint32_t begin = 0;
    uint32_t end = rows;

    while (begin < end)
    {
        uint32_t mid = (begin + end) >> 1;
        const uint8_t* mid_row = data + static_cast<size_t>(mid) * table.total_field_size;
        uint32_t mid_owner = read_column_u32(mid_row, field_meta);
        if (mid_owner < owner)
            begin = mid + 1;
        else
            end = mid;
    }

    if (begin >= rows)
        return std::nullopt;

    const uint8_t* row = data + static_cast<size_t>(begin) * table.total_field_size;
    uint32_t found_owner = read_column_u32(row, field_meta);
    if (found_owner != owner)
        return std::nullopt;

    return std::make_optional(begin + 1);
}

std::optional<uint32_t> CliImage::find_last_row_less_equal(TableType table_index, uint8_t field_index, uint32_t compared_value) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(table_index)];
    if (!table.valid || table.row_field_count == 0 || field_index >= table.row_field_count)
        return std::nullopt;

    const CliTableFieldMeta& field_meta = table.row_fields[field_index];
    if (!(field_meta.size == 2 || field_meta.size == 4))
        return std::nullopt;

    const uint8_t* data = table.data;
    uint32_t rows = table.row_count;
    uint32_t begin = 0;
    uint32_t end = rows;

    while (begin < end)
    {
        uint32_t mid = (begin + end) >> 1;
        const uint8_t* mid_row = data + static_cast<size_t>(mid) * table.total_field_size;
        uint32_t mid_value = read_column_u32(mid_row, field_meta);
        if (mid_value <= compared_value)
            begin = mid + 1;
        else
            end = mid;
    }

    if (begin == 0)
        return std::nullopt;

    return std::make_optional(begin);
}

void CliImage::init_table_metas_final()
{
    for (size_t table_index = 0; table_index < MAX_TABLE_COUNT; ++table_index)
    {
        CliTableMeta& table = tables[table_index];
        if (!table.valid)
            continue;

        uint8_t total_field_size = 0;
        for (uint8_t field_index = 0; field_index < table.row_field_count; ++field_index)
        {
            const_cast<CliTableFieldMeta&>(table.row_fields[field_index]).offset = total_field_size;
            total_field_size += table.row_fields[field_index].size;
        }
        table.total_field_size = total_field_size;
    }
}

// Table field initialization
void CliImage::init_table_field_metas(alloc::MemPool& pool)
{
    // Module (0x00)
    {
        constexpr size_t FIELD_COUNT = 5;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_guid_index_byte();
            fields[3].size = compute_guid_index_byte();
            fields[4].size = compute_guid_index_byte();
            tables[static_cast<size_t>(TableType::Module)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Module)].row_fields = fields;
        }
    }

    // TypeRef (0x01)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size =
                compute_table_index_byte4(TableType::Module, TableType::ModuleRef, TableType::AssemblyRef, TableType::TypeRef, TagBits::RESOLUTION_SCOPE);
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_string_index_byte();
            tables[static_cast<size_t>(TableType::TypeRef)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::TypeRef)].row_fields = fields;
        }
    }

    // TypeDef (0x02)
    {
        constexpr size_t FIELD_COUNT = 6;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_string_index_byte();
            fields[3].size = compute_table_index_byte3(TableType::TypeDef, TableType::TypeRef, TableType::TypeSpec, TagBits::TYPE_DEF_OR_REF);
            fields[4].size = compute_table_index_byte1(TableType::Field);
            fields[5].size = compute_table_index_byte1(TableType::Method);
            tables[static_cast<size_t>(TableType::TypeDef)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::TypeDef)].row_fields = fields;
        }
    }

    // FieldPtr (0x03)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Field);
            tables[static_cast<size_t>(TableType::FieldPtr)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::FieldPtr)].row_fields = fields;
        }
    }

    // Field (0x04)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::Field)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Field)].row_fields = fields;
        }
    }

    // MethodPtr (0x05)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Method);
            tables[static_cast<size_t>(TableType::MethodPtr)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::MethodPtr)].row_fields = fields;
        }
    }

    // Method (0x06)
    {
        constexpr size_t FIELD_COUNT = 6;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = 2;
            fields[2].size = 2;
            fields[3].size = compute_string_index_byte();
            fields[4].size = compute_blob_index_byte();
            fields[5].size = compute_table_index_byte1(TableType::Param);
            tables[static_cast<size_t>(TableType::Method)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Method)].row_fields = fields;
        }
    }

    // ParamPtr (0x07)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Param);
            tables[static_cast<size_t>(TableType::ParamPtr)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::ParamPtr)].row_fields = fields;
        }
    }

    // Param (0x08)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = 2;
            fields[2].size = compute_string_index_byte();
            tables[static_cast<size_t>(TableType::Param)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Param)].row_fields = fields;
        }
    }

    // InterfaceImpl (0x09)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::TypeDef);
            fields[1].size = compute_table_index_byte3(TableType::TypeDef, TableType::TypeRef, TableType::TypeSpec, TagBits::TYPE_DEF_OR_REF);
            tables[static_cast<size_t>(TableType::InterfaceImpl)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::InterfaceImpl)].row_fields = fields;
        }
    }

    // MemberRef (0x0A)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size =
                compute_table_index_byte4(TableType::Method, TableType::ModuleRef, TableType::TypeDef, TableType::TypeRef, TagBits::MEMBER_REF_PARENT);
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::MemberRef)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::MemberRef)].row_fields = fields;
        }
    }

    // Constant (0x0B)
    {
        constexpr size_t FIELD_COUNT = 4;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 1;
            fields[1].size = 1;
            fields[2].size = compute_table_index_byte3(TableType::Param, TableType::Field, TableType::Property, TagBits::HAS_CONSTANT);
            fields[3].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::Constant)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Constant)].row_fields = fields;
        }
    }

    // CustomAttribute (0x0C)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_bytes(HAS_CUSTOM_ATTRIBUTE_ASSOCIATE_TABLES, sizeof(HAS_CUSTOM_ATTRIBUTE_ASSOCIATE_TABLES) / sizeof(TableType),
                                                       TagBits::HAS_CUSTOM_ATTRIBUTE);
            fields[1].size = compute_table_index_byte2(TableType::Method, TableType::MemberRef, TagBits::CUSTOM_ATTRIBUTE_TYPE);
            fields[2].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::CustomAttribute)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::CustomAttribute)].row_fields = fields;
        }
    }

    // FieldMarshal (0x0D)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte2(TableType::Field, TableType::Param, TagBits::HAS_FIELD_MARSHAL);
            fields[1].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::FieldMarshal)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::FieldMarshal)].row_fields = fields;
        }
    }

    // DeclSecurity (0x0E)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = compute_table_index_byte3(TableType::TypeDef, TableType::Method, TableType::Assembly, TagBits::HAS_DECL_SECURITY);
            fields[2].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::DeclSecurity)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::DeclSecurity)].row_fields = fields;
        }
    }

    // ClassLayout (0x0F)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = 4;
            fields[2].size = compute_table_index_byte1(TableType::TypeDef);
            tables[static_cast<size_t>(TableType::ClassLayout)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::ClassLayout)].row_fields = fields;
        }
    }

    // FieldLayout (0x10)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = compute_table_index_byte1(TableType::Field);
            tables[static_cast<size_t>(TableType::FieldLayout)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::FieldLayout)].row_fields = fields;
        }
    }

    // StandaloneSig (0x11)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::StandaloneSig)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::StandaloneSig)].row_fields = fields;
        }
    }

    // EventMap (0x12)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::TypeDef);
            fields[1].size = compute_table_index_byte1(TableType::Event);
            tables[static_cast<size_t>(TableType::EventMap)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::EventMap)].row_fields = fields;
        }
    }

    // EventPtr (0x13)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Event);
            tables[static_cast<size_t>(TableType::EventPtr)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::EventPtr)].row_fields = fields;
        }
    }

    // Event (0x14)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_table_index_byte3(TableType::TypeDef, TableType::TypeRef, TableType::TypeSpec, TagBits::TYPE_DEF_OR_REF);
            tables[static_cast<size_t>(TableType::Event)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Event)].row_fields = fields;
        }
    }

    // PropertyMap (0x15)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::TypeDef);
            fields[1].size = compute_table_index_byte1(TableType::Property);
            tables[static_cast<size_t>(TableType::PropertyMap)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::PropertyMap)].row_fields = fields;
        }
    }

    // PropertyPtr (0x16)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Property);
            tables[static_cast<size_t>(TableType::PropertyPtr)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::PropertyPtr)].row_fields = fields;
        }
    }

    // Property (0x17)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::Property)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Property)].row_fields = fields;
        }
    }

    // MethodSemantics (0x18)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = compute_table_index_byte1(TableType::Method);
            fields[2].size = compute_table_index_byte2(TableType::Event, TableType::Property, TagBits::HAS_SEMANTICS);
            tables[static_cast<size_t>(TableType::MethodSemantics)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::MethodSemantics)].row_fields = fields;
        }
    }

    // MethodImpl (0x19)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::TypeDef);
            fields[1].size = compute_table_index_byte2(TableType::Method, TableType::MemberRef, TagBits::METHOD_DEF_OR_REF);
            fields[2].size = compute_table_index_byte2(TableType::Method, TableType::MemberRef, TagBits::METHOD_DEF_OR_REF);
            tables[static_cast<size_t>(TableType::MethodImpl)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::MethodImpl)].row_fields = fields;
        }
    }

    // ModuleRef (0x1A)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_string_index_byte();
            tables[static_cast<size_t>(TableType::ModuleRef)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::ModuleRef)].row_fields = fields;
        }
    }

    // TypeSpec (0x1B)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::TypeSpec)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::TypeSpec)].row_fields = fields;
        }
    }

    // ImplMap (0x1C)
    {
        constexpr size_t FIELD_COUNT = 4;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = compute_table_index_byte2(TableType::Field, TableType::Method, TagBits::MEMBER_FORWARDED);
            fields[2].size = compute_string_index_byte();
            fields[3].size = compute_table_index_byte1(TableType::ModuleRef);
            tables[static_cast<size_t>(TableType::ImplMap)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::ImplMap)].row_fields = fields;
        }
    }

    // FieldRva (0x1D)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = compute_table_index_byte1(TableType::Field);
            tables[static_cast<size_t>(TableType::FieldRva)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::FieldRva)].row_fields = fields;
        }
    }

    // EncLog (0x1E) - Skipped
    // EncMap (0x1F) - Skipped

    // Assembly (0x20)
    {
        constexpr size_t FIELD_COUNT = 9;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = 2;
            fields[2].size = 2;
            fields[3].size = 2;
            fields[4].size = 2;
            fields[5].size = 4;
            fields[6].size = compute_blob_index_byte();
            fields[7].size = compute_string_index_byte();
            fields[8].size = compute_string_index_byte();
            tables[static_cast<size_t>(TableType::Assembly)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Assembly)].row_fields = fields;
        }
    }

    // AssemblyProcessor (0x21)
    {
        constexpr size_t FIELD_COUNT = 1;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            tables[static_cast<size_t>(TableType::AssemblyProcessor)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::AssemblyProcessor)].row_fields = fields;
        }
    }

    // AssemblyOs (0x22)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = 4;
            fields[2].size = 4;
            tables[static_cast<size_t>(TableType::AssemblyOs)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::AssemblyOs)].row_fields = fields;
        }
    }

    // AssemblyRef (0x23)
    {
        constexpr size_t FIELD_COUNT = 9;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = 2;
            fields[2].size = 2;
            fields[3].size = 2;
            fields[4].size = 4;
            fields[5].size = compute_blob_index_byte();
            fields[6].size = compute_string_index_byte();
            fields[7].size = compute_string_index_byte();
            fields[8].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::AssemblyRef)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::AssemblyRef)].row_fields = fields;
        }
    }

    // AssemblyRefProcessor (0x24)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = compute_table_index_byte1(TableType::AssemblyRef);
            tables[static_cast<size_t>(TableType::AssemblyRefProcessor)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::AssemblyRefProcessor)].row_fields = fields;
        }
    }

    // AssemblyRefOs (0x25)
    {
        constexpr size_t FIELD_COUNT = 4;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = 4;
            fields[2].size = 4;
            fields[3].size = compute_table_index_byte1(TableType::AssemblyRef);
            tables[static_cast<size_t>(TableType::AssemblyRefOs)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::AssemblyRefOs)].row_fields = fields;
        }
    }

    // File (0x26)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = compute_string_index_byte();
            fields[2].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::File)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::File)].row_fields = fields;
        }
    }

    // ExportedType (0x27)
    {
        constexpr size_t FIELD_COUNT = 5;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = 4;
            fields[2].size = compute_string_index_byte();
            fields[3].size = compute_string_index_byte();
            fields[4].size = compute_table_index_byte3(TableType::File, TableType::ExportedType, TableType::Assembly, TagBits::IMPLEMENTATION);
            tables[static_cast<size_t>(TableType::ExportedType)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::ExportedType)].row_fields = fields;
        }
    }

    // ManifestResource (0x28)
    {
        constexpr size_t FIELD_COUNT = 4;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 4;
            fields[1].size = 4;
            fields[2].size = compute_string_index_byte();
            fields[3].size = compute_table_index_byte2(TableType::File, TableType::AssemblyRef, TagBits::IMPLEMENTATION);
            tables[static_cast<size_t>(TableType::ManifestResource)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::ManifestResource)].row_fields = fields;
        }
    }

    // NestedClass (0x29)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::TypeDef);
            fields[1].size = compute_table_index_byte1(TableType::TypeDef);
            tables[static_cast<size_t>(TableType::NestedClass)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::NestedClass)].row_fields = fields;
        }
    }

    // GenericParam (0x2A)
    {
        constexpr size_t FIELD_COUNT = 4;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = 2;
            fields[2].size = compute_table_index_byte2(TableType::TypeDef, TableType::Method, TagBits::TYPE_OR_METHOD_DEF);
            fields[3].size = compute_string_index_byte();
            tables[static_cast<size_t>(TableType::GenericParam)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::GenericParam)].row_fields = fields;
        }
    }

    // MethodSpec (0x2B)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte2(TableType::Method, TableType::MemberRef, TagBits::METHOD_DEF_OR_REF);
            fields[1].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::MethodSpec)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::MethodSpec)].row_fields = fields;
        }
    }

    // GenericParamConstraint (0x2C)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::GenericParam);
            fields[1].size = compute_table_index_byte3(TableType::TypeDef, TableType::TypeRef, TableType::TypeSpec, TagBits::TYPE_DEF_OR_REF);
            tables[static_cast<size_t>(TableType::GenericParamConstraint)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::GenericParamConstraint)].row_fields = fields;
        }
    }

    // Document (0x30)
    {
        constexpr size_t FIELD_COUNT = 4;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_blob_index_byte();
            fields[1].size = compute_guid_index_byte();
            fields[2].size = compute_blob_index_byte();
            fields[3].size = compute_guid_index_byte();
            tables[static_cast<size_t>(TableType::Document)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::Document)].row_fields = fields;
        }
    }

    // MethodDebugInformation (0x31)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Document);
            fields[1].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::MethodDebugInformation)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::MethodDebugInformation)].row_fields = fields;
        }
    }

    // LocalScope (0x32)
    {
        constexpr size_t FIELD_COUNT = 6;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Method);
            fields[1].size = compute_table_index_byte1(TableType::ImportScope);
            fields[2].size = compute_table_index_byte1(TableType::LocalVariable);
            fields[3].size = compute_table_index_byte1(TableType::LocalConstant);
            fields[4].size = 4;
            fields[5].size = 4;
            tables[static_cast<size_t>(TableType::LocalScope)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::LocalScope)].row_fields = fields;
        }
    }

    // LocalVariable (0x33)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = 2;
            fields[1].size = 2;
            fields[2].size = compute_string_index_byte();
            tables[static_cast<size_t>(TableType::LocalVariable)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::LocalVariable)].row_fields = fields;
        }
    }

    // LocalConstant (0x34)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_string_index_byte();
            fields[1].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::LocalConstant)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::LocalConstant)].row_fields = fields;
        }
    }

    // ImportScope (0x35)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::ImportScope);
            fields[1].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::ImportScope)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::ImportScope)].row_fields = fields;
        }
    }

    // StateMachineMethod (0x36)
    {
        constexpr size_t FIELD_COUNT = 2;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_byte1(TableType::Method);
            fields[1].size = compute_table_index_byte1(TableType::Method);
            tables[static_cast<size_t>(TableType::StateMachineMethod)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::StateMachineMethod)].row_fields = fields;
        }
    }

    // CustomDebugInformation (0x37)
    {
        constexpr size_t FIELD_COUNT = 3;
        auto fields = pool.calloc_any<CliTableFieldMeta>(FIELD_COUNT);
        if (fields)
        {
            fields[0].size = compute_table_index_bytes(HAS_CUSTOM_DEBUG_INFORMATION, sizeof(HAS_CUSTOM_DEBUG_INFORMATION) / sizeof(TableType),
                                                       TagBits::HAS_CUSTOM_DEBUG_INFORMATION);
            fields[1].size = compute_guid_index_byte();
            fields[2].size = compute_blob_index_byte();
            tables[static_cast<size_t>(TableType::CustomDebugInformation)].row_field_count = FIELD_COUNT;
            tables[static_cast<size_t>(TableType::CustomDebugInformation)].row_fields = fields;
        }
    }
}

std::optional<uint32_t> CliImage::rva_to_image_offset(uint32_t rva) const
{
    for (uint32_t i = 0; i < section_count; ++i)
    {
        const CliSection& sec = sections[i];
        if (rva >= sec.virtual_address_begin && rva < sec.virtual_address_end)
        {
            return rva - sec.virtual_address_begin + sec.raw_data_begin;
        }
    }
    return std::nullopt;
}

RtResult<utils::BinaryReader> CliImage::get_decoded_blob_reader(uint32_t index) const
{
    auto& heap = blob_heap;
    if (index >= heap.size)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    auto data = heap.data + index;
    uint32_t blob_size = 0;
    size_t size_length = 0;
    if (!utils::BinaryReader::try_decode_compressed_uint32(data, heap.size - index, blob_size, size_length))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    if (index + size_length + blob_size > heap.size)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }
    RET_OK(utils::BinaryReader(data + size_length, blob_size));
}

std::optional<RowModule> CliImage::read_module(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Module)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowModule row{};
    row.generation = read_column_u16(row_data, table.row_fields[0]);
    row.name = read_column_u32(row_data, table.row_fields[1]);
    row.mvid = read_column_u32(row_data, table.row_fields[2]);
    row.encid = read_column_u32(row_data, table.row_fields[3]);
    row.enc_base_id = read_column_u32(row_data, table.row_fields[4]);
    return row;
}

std::optional<RowTypeRef> CliImage::read_type_ref(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::TypeRef)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowTypeRef row{};
    row.resolution_scope = read_column_u32(row_data, table.row_fields[0]);
    row.type_name = read_column_u32(row_data, table.row_fields[1]);
    row.type_namespace = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowTypeDef> CliImage::read_type_def(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::TypeDef)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowTypeDef row{};
    row.flags = read_column_u32(row_data, table.row_fields[0]);
    row.type_name = read_column_u32(row_data, table.row_fields[1]);
    row.type_namespace = read_column_u32(row_data, table.row_fields[2]);
    row.extends = read_column_u32(row_data, table.row_fields[3]);
    row.field_list = read_column_u32(row_data, table.row_fields[4]);
    row.method_list = read_column_u32(row_data, table.row_fields[5]);
    return row;
}

std::optional<RowField> CliImage::read_field(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Field)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowField row{};
    row.flags = read_column_u16(row_data, table.row_fields[0]);
    row.name = read_column_u32(row_data, table.row_fields[1]);
    row.signature = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowMethod> CliImage::read_method(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Method)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowMethod row{};
    row.rva = read_column_u32(row_data, table.row_fields[0]);
    row.impl_flags = read_column_u16(row_data, table.row_fields[1]);
    row.flags = read_column_u16(row_data, table.row_fields[2]);
    row.name = read_column_u32(row_data, table.row_fields[3]);
    row.signature = read_column_u32(row_data, table.row_fields[4]);
    row.param_list = read_column_u32(row_data, table.row_fields[5]);
    return row;
}

std::optional<RowParam> CliImage::read_param(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Param)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowParam row{};
    row.flags = read_column_u16(row_data, table.row_fields[0]);
    row.sequence = read_column_u16(row_data, table.row_fields[1]);
    row.name = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowInterfaceImpl> CliImage::read_interface_impl(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::InterfaceImpl)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowInterfaceImpl row{};
    row.class_idx = read_column_u32(row_data, table.row_fields[0]);
    row.interface_idx = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowMemberRef> CliImage::read_member_ref(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::MemberRef)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowMemberRef row{};
    row.class_idx = read_column_u32(row_data, table.row_fields[0]);
    row.name = read_column_u32(row_data, table.row_fields[1]);
    row.signature = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowConstant> CliImage::read_constant(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Constant)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowConstant row{};
    row.type_ = read_column_u8(row_data, table.row_fields[0]);
    row.padding = read_column_u8(row_data, table.row_fields[1]);
    row.parent = read_column_u32(row_data, table.row_fields[2]);
    row.value = read_column_u32(row_data, table.row_fields[3]);
    return row;
}

std::optional<RowCustomAttribute> CliImage::read_custom_attribute(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::CustomAttribute)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowCustomAttribute row{};
    row.parent = read_column_u32(row_data, table.row_fields[0]);
    row.type_ = read_column_u32(row_data, table.row_fields[1]);
    row.value = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowFieldMarshal> CliImage::read_field_marshal(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::FieldMarshal)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowFieldMarshal row{};
    row.parent = read_column_u32(row_data, table.row_fields[0]);
    row.native_type = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowDeclSecurity> CliImage::read_decl_security(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::DeclSecurity)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowDeclSecurity row{};
    row.action = read_column_u16(row_data, table.row_fields[0]);
    row.parent = read_column_u32(row_data, table.row_fields[1]);
    row.permission_set = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowClassLayout> CliImage::read_class_layout(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::ClassLayout)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowClassLayout row{};
    row.packing_size = read_column_u16(row_data, table.row_fields[0]);
    row.class_size = read_column_u32(row_data, table.row_fields[1]);
    row.parent = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowFieldLayout> CliImage::read_field_layout(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::FieldLayout)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowFieldLayout row{};
    row.offset = read_column_u32(row_data, table.row_fields[0]);
    row.field = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowStandaloneSig> CliImage::read_stand_alone_sig(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::StandaloneSig)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowStandaloneSig row{};
    row.signature = read_column_u32(row_data, table.row_fields[0]);
    return row;
}

std::optional<RowEventMap> CliImage::read_event_map(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::EventMap)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowEventMap row{};
    row.parent = read_column_u32(row_data, table.row_fields[0]);
    row.event_list = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowEvent> CliImage::read_event(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Event)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowEvent row{};
    row.event_flags = read_column_u16(row_data, table.row_fields[0]);
    row.name = read_column_u32(row_data, table.row_fields[1]);
    row.event_type = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowPropertyMap> CliImage::read_property_map(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::PropertyMap)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowPropertyMap row{};
    row.parent = read_column_u32(row_data, table.row_fields[0]);
    row.property_list = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowProperty> CliImage::read_property(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Property)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowProperty row{};
    row.flags = read_column_u16(row_data, table.row_fields[0]);
    row.name = read_column_u32(row_data, table.row_fields[1]);
    row.type_ = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowMethodSemantics> CliImage::read_method_semantics(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::MethodSemantics)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowMethodSemantics row{};
    row.semantics = read_column_u16(row_data, table.row_fields[0]);
    row.method = read_column_u32(row_data, table.row_fields[1]);
    row.association = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowMethodImpl> CliImage::read_method_impl(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::MethodImpl)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowMethodImpl row{};
    row.class_idx = read_column_u32(row_data, table.row_fields[0]);
    row.method_body = read_column_u32(row_data, table.row_fields[1]);
    row.method_declaration = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowModuleRef> CliImage::read_module_ref(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::ModuleRef)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowModuleRef row{};
    row.name = read_column_u32(row_data, table.row_fields[0]);
    return row;
}

std::optional<RowTypeSpec> CliImage::read_type_spec(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::TypeSpec)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowTypeSpec row{};
    row.signature = read_column_u32(row_data, table.row_fields[0]);
    return row;
}

std::optional<RowImplMap> CliImage::read_impl_map(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::ImplMap)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowImplMap row{};
    row.mapping_flags = read_column_u16(row_data, table.row_fields[0]);
    row.member_forwarded = read_column_u32(row_data, table.row_fields[1]);
    row.import_name = read_column_u32(row_data, table.row_fields[2]);
    row.import_scope = read_column_u32(row_data, table.row_fields[3]);
    return row;
}

std::optional<RowFieldRva> CliImage::read_field_rva(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::FieldRva)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowFieldRva row{};
    row.rva = read_column_u32(row_data, table.row_fields[0]);
    row.field = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowAssembly> CliImage::read_assembly(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Assembly)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowAssembly row{};
    row.hash_alg_id = read_column_u32(row_data, table.row_fields[0]);
    row.major_version = read_column_u16(row_data, table.row_fields[1]);
    row.minor_version = read_column_u16(row_data, table.row_fields[2]);
    row.build_number = read_column_u16(row_data, table.row_fields[3]);
    row.revision_number = read_column_u16(row_data, table.row_fields[4]);
    row.flags = read_column_u32(row_data, table.row_fields[5]);
    row.public_key = read_column_u32(row_data, table.row_fields[6]);
    row.name = read_column_u32(row_data, table.row_fields[7]);
    row.locale = read_column_u32(row_data, table.row_fields[8]);
    return row;
}

std::optional<RowAssemblyRef> CliImage::read_assembly_ref(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::AssemblyRef)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowAssemblyRef row{};
    row.major_version = read_column_u16(row_data, table.row_fields[0]);
    row.minor_version = read_column_u16(row_data, table.row_fields[1]);
    row.build_number = read_column_u16(row_data, table.row_fields[2]);
    row.revision_number = read_column_u16(row_data, table.row_fields[3]);
    row.flags = read_column_u32(row_data, table.row_fields[4]);
    row.public_key_or_token = read_column_u32(row_data, table.row_fields[5]);
    row.name = read_column_u32(row_data, table.row_fields[6]);
    row.locale = read_column_u32(row_data, table.row_fields[7]);
    row.hash_value = read_column_u32(row_data, table.row_fields[8]);
    return row;
}

std::optional<RowFile> CliImage::read_file(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::File)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowFile row{};
    row.flags = read_column_u32(row_data, table.row_fields[0]);
    row.name = read_column_u32(row_data, table.row_fields[1]);
    row.hash_value = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowExportedType> CliImage::read_exported_type(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::ExportedType)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowExportedType row{};
    row.flags = read_column_u32(row_data, table.row_fields[0]);
    row.type_def_id = read_column_u32(row_data, table.row_fields[1]);
    row.type_name = read_column_u32(row_data, table.row_fields[2]);
    row.type_namespace = read_column_u32(row_data, table.row_fields[3]);
    row.implementation = read_column_u32(row_data, table.row_fields[4]);
    return row;
}

std::optional<RowManifestResource> CliImage::read_manifest_resource(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::ManifestResource)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowManifestResource row{};
    row.offset = read_column_u32(row_data, table.row_fields[0]);
    row.flags = read_column_u32(row_data, table.row_fields[1]);
    row.name = read_column_u32(row_data, table.row_fields[2]);
    row.implementation = read_column_u32(row_data, table.row_fields[3]);
    return row;
}

std::optional<RowNestedClass> CliImage::read_nested_class(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::NestedClass)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowNestedClass row{};
    row.nested_class = read_column_u32(row_data, table.row_fields[0]);
    row.enclosing_class = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowGenericParam> CliImage::read_generic_param(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::GenericParam)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowGenericParam row{};
    row.number = read_column_u16(row_data, table.row_fields[0]);
    row.flags = read_column_u16(row_data, table.row_fields[1]);
    row.owner = read_column_u32(row_data, table.row_fields[2]);
    row.name = read_column_u32(row_data, table.row_fields[3]);
    return row;
}

std::optional<RowMethodSpec> CliImage::read_method_spec(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::MethodSpec)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowMethodSpec row{};
    row.method = read_column_u32(row_data, table.row_fields[0]);
    row.instantiation = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowGenericParamConstraint> CliImage::read_generic_param_constraint(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::GenericParamConstraint)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowGenericParamConstraint row{};
    row.owner = read_column_u32(row_data, table.row_fields[0]);
    row.constraint = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowDocument> CliImage::read_document(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::Document)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowDocument row{};
    row.name = read_column_u32(row_data, table.row_fields[0]);
    row.hash_algorithm = read_column_u32(row_data, table.row_fields[1]);
    row.hash = read_column_u32(row_data, table.row_fields[2]);
    row.language = read_column_u32(row_data, table.row_fields[3]);
    return row;
}

std::optional<RowMethodDebugInformation> CliImage::read_method_debug_information(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::MethodDebugInformation)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowMethodDebugInformation row{};
    row.document = read_column_u32(row_data, table.row_fields[0]);
    row.sequence_points = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowLocalScope> CliImage::read_local_scope(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::LocalScope)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowLocalScope row{};
    row.method = read_column_u32(row_data, table.row_fields[0]);
    row.import_scope = read_column_u32(row_data, table.row_fields[1]);
    row.variables = read_column_u32(row_data, table.row_fields[2]);
    row.constants = read_column_u32(row_data, table.row_fields[3]);
    row.start_offset = read_column_u32(row_data, table.row_fields[4]);
    row.length = read_column_u32(row_data, table.row_fields[5]);
    return row;
}

std::optional<RowLocalVariable> CliImage::read_local_variable(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::LocalVariable)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowLocalVariable row{};
    row.attributes = read_column_u16(row_data, table.row_fields[0]);
    row.index = read_column_u16(row_data, table.row_fields[1]);
    row.name = read_column_u32(row_data, table.row_fields[2]);
    return row;
}

std::optional<RowLocalConstant> CliImage::read_local_constant(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::LocalConstant)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowLocalConstant row{};
    row.name = read_column_u32(row_data, table.row_fields[0]);
    row.signature = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowImportScope> CliImage::read_import_scope(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::ImportScope)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowImportScope row{};
    row.parent = read_column_u32(row_data, table.row_fields[0]);
    row.imports = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowStateMachineMethod> CliImage::read_state_machine_method(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::StateMachineMethod)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowStateMachineMethod row{};
    row.move_next_method = read_column_u32(row_data, table.row_fields[0]);
    row.kickoff_method = read_column_u32(row_data, table.row_fields[1]);
    return row;
}

std::optional<RowCustomDebugInformation> CliImage::read_custom_debug_information(uint32_t row_index) const
{
    const CliTableMeta& table = tables[static_cast<size_t>(TableType::CustomDebugInformation)];
    if (row_index == 0 || row_index > table.row_count)
        return std::nullopt;

    const uint8_t* row_data = table.data + (row_index - 1) * table.total_field_size;
    RowCustomDebugInformation row{};
    row.parent = read_column_u32(row_data, table.row_fields[0]);
    row.kind = read_column_u32(row_data, table.row_fields[1]);
    row.value = read_column_u32(row_data, table.row_fields[2]);
    return row;
}
} // namespace metadata
} // namespace leanclr
