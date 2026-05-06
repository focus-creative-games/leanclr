#pragma once

#include <algorithm>
#include <cstring>
#include "core/stl_compat.h"
#include <utility>

#include "utils/mem_op.h"
#include "alloc/mem_pool.h"
#include "cli_metadata.h"
#include "utils/binary_reader.h"

namespace leanclr
{
namespace utils
{
class MemPool;
}
} // namespace leanclr

namespace leanclr
{
namespace metadata
{

struct CliSection
{
    uint32_t virtual_address_begin;
    uint32_t virtual_address_end;
    uint32_t raw_data_begin;
};

struct CliHeap
{
    const uint8_t* name;
    const uint8_t* data;
    uint32_t size;
};

struct RidRange
{
    uint32_t ridBegin;
    uint32_t ridEnd;
};

class CliImage
{
  private:
    // Table field metadata
    struct CliTableFieldMeta
    {
        uint8_t size;
        uint8_t offset;
    };

    struct CliTableMeta
    {
        const uint8_t* data;
        uint32_t row_count;
        bool valid;
        uint8_t total_field_size;
        uint8_t row_field_count;
        const CliTableFieldMeta* row_fields;
    };

  public:
    // Initialize streams and tables from metadata
    RtResultVoid load_streams();
    RtResultVoid load_tables(alloc::MemPool& pool);

    void set_image_data(const uint8_t* data, size_t length)
    {
        image_data = data;
        image_length = length;
    }

    const uint8_t* get_image_data() const
    {
        return image_data;
    }

    const uint8_t* get_image_data_at(uint32_t offset) const
    {
        assert(offset < image_length);
        return image_data + offset;
    }

    const size_t get_image_length() const
    {
        return image_length;
    }

    std::optional<uint32_t> rva_to_image_offset(uint32_t rva) const;

    uint32_t get_table_row_num(TableType table_type) const
    {
        return tables[static_cast<size_t>(table_type)].row_count;
    }

    // Binary searches over sorted tables
    std::optional<RidRange> find_row_range_of_owner_at_sorted_table(TableType table_index, uint8_t field_index, uint32_t owner) const;
    std::optional<uint32_t> find_row_of_owner(TableType table_index, uint8_t field_index, uint32_t owner) const;
    std::optional<uint32_t> find_last_row_less_equal(TableType table_index, uint8_t field_index, uint32_t compared_value) const;

    // Heap accessor methods
    const CliHeap& get_string_heap() const
    {
        return string_heap;
    }

    const CliHeap& get_us_heap() const
    {
        return us_heap;
    }

    const CliHeap& get_blob_heap() const
    {
        return blob_heap;
    }

    const CliHeap& get_guid_heap() const
    {
        return guid_heap;
    }

    const CliHeap& get_tables_heap() const
    {
        return tables_heap;
    }

    const CliHeap& get_pdb_heap() const
    {
        return pdb_heap;
    }

    const EncodedTokenId get_entry_point_token() const
    {
        return entry_point_token;
    }

    void set_entry_point_token(EncodedTokenId token)
    {
        entry_point_token = token;
    }

    void set_sections(const CliSection* sections, uint32_t section_count)
    {
        this->sections = sections;
        this->section_count = static_cast<uint16_t>(section_count);
    }

    uint32_t get_section_count() const
    {
        return section_count;
    }

    void set_metadata_offset_length(uint32_t offset, uint32_t length)
    {
        metadata_offset = offset;
        metadata_length = length;
    }

    uint32_t get_metadata_offset() const
    {
        return metadata_offset;
    }

    uint32_t get_metadata_length() const
    {
        return metadata_length;
    }

    RtResult<utils::BinaryReader> get_decoded_blob_reader(uint32_t index) const;

    // Row read methods - declarations only, implementations in cpp
    std::optional<RowModule> read_module(uint32_t row_index) const;
    std::optional<RowTypeRef> read_type_ref(uint32_t row_index) const;
    std::optional<RowTypeDef> read_type_def(uint32_t row_index) const;
    std::optional<RowField> read_field(uint32_t row_index) const;
    std::optional<RowMethod> read_method(uint32_t row_index) const;
    std::optional<RowParam> read_param(uint32_t row_index) const;
    std::optional<RowInterfaceImpl> read_interface_impl(uint32_t row_index) const;
    std::optional<RowMemberRef> read_member_ref(uint32_t row_index) const;
    std::optional<RowConstant> read_constant(uint32_t row_index) const;
    std::optional<RowCustomAttribute> read_custom_attribute(uint32_t row_index) const;
    std::optional<RowFieldMarshal> read_field_marshal(uint32_t row_index) const;
    std::optional<RowDeclSecurity> read_decl_security(uint32_t row_index) const;
    std::optional<RowClassLayout> read_class_layout(uint32_t row_index) const;
    std::optional<RowFieldLayout> read_field_layout(uint32_t row_index) const;
    std::optional<RowStandaloneSig> read_stand_alone_sig(uint32_t row_index) const;
    std::optional<RowEventMap> read_event_map(uint32_t row_index) const;
    std::optional<RowEvent> read_event(uint32_t row_index) const;
    std::optional<RowPropertyMap> read_property_map(uint32_t row_index) const;
    std::optional<RowProperty> read_property(uint32_t row_index) const;
    std::optional<RowMethodSemantics> read_method_semantics(uint32_t row_index) const;
    std::optional<RowMethodImpl> read_method_impl(uint32_t row_index) const;
    std::optional<RowModuleRef> read_module_ref(uint32_t row_index) const;
    std::optional<RowTypeSpec> read_type_spec(uint32_t row_index) const;
    std::optional<RowImplMap> read_impl_map(uint32_t row_index) const;
    std::optional<RowFieldRva> read_field_rva(uint32_t row_index) const;
    std::optional<RowAssembly> read_assembly(uint32_t row_index) const;
    std::optional<RowAssemblyRef> read_assembly_ref(uint32_t row_index) const;
    std::optional<RowFile> read_file(uint32_t row_index) const;
    std::optional<RowExportedType> read_exported_type(uint32_t row_index) const;
    std::optional<RowManifestResource> read_manifest_resource(uint32_t row_index) const;
    std::optional<RowNestedClass> read_nested_class(uint32_t row_index) const;
    std::optional<RowGenericParam> read_generic_param(uint32_t row_index) const;
    std::optional<RowMethodSpec> read_method_spec(uint32_t row_index) const;
    std::optional<RowGenericParamConstraint> read_generic_param_constraint(uint32_t row_index) const;
    std::optional<RowDocument> read_document(uint32_t row_index) const;
    std::optional<RowMethodDebugInformation> read_method_debug_information(uint32_t row_index) const;
    std::optional<RowLocalScope> read_local_scope(uint32_t row_index) const;
    std::optional<RowLocalVariable> read_local_variable(uint32_t row_index) const;
    std::optional<RowLocalConstant> read_local_constant(uint32_t row_index) const;
    std::optional<RowImportScope> read_import_scope(uint32_t row_index) const;
    std::optional<RowStateMachineMethod> read_state_machine_method(uint32_t row_index) const;
    std::optional<RowCustomDebugInformation> read_custom_debug_information(uint32_t row_index) const;

  private:
    static constexpr size_t MAX_STREAM_NAME_LENGTH = 16;
    static constexpr size_t MAX_TABLE_INDEX = 0x37;
    static constexpr size_t MAX_TABLE_COUNT = MAX_TABLE_INDEX + 1;
    static constexpr size_t TABLE_HEAP_HEADER_SIZE = 24;

    CliTableMeta tables[MAX_TABLE_COUNT];

    // Helper methods for field computation (inline implementations)
    inline uint8_t compute_blob_index_byte() const
    {
        return blob_heap_size_4_byte ? 4 : 2;
    }

    inline uint8_t compute_string_index_byte() const
    {
        return string_heap_size_4_byte ? 4 : 2;
    }

    inline uint8_t compute_guid_index_byte() const
    {
        return guid_heap_size_4_byte ? 4 : 2;
    }

    inline uint8_t compute_index_byte(uint32_t max_row_count, uint32_t tag_bit_num) const
    {
        return ((max_row_count << tag_bit_num) < 65536) ? 2 : 4; // uint16_t::MAX = 65535
    }

    inline uint8_t compute_table_index_byte1(TableType t1) const
    {
        return (tables[static_cast<size_t>(t1)].row_count < 65536) ? 2 : 4;
    }

    inline uint8_t compute_table_index_byte2(TableType t1, TableType t2, uint32_t tag_bit_num) const
    {
        uint32_t max_row_num = std::max(get_table_row_num(t1), get_table_row_num(t2));
        return compute_index_byte(max_row_num, tag_bit_num);
    }

    inline uint8_t compute_table_index_byte3(TableType t1, TableType t2, TableType t3, uint32_t tag_bit_num) const
    {
        uint32_t max_row_num = std::max({get_table_row_num(t1), get_table_row_num(t2), get_table_row_num(t3)});
        return compute_index_byte(max_row_num, tag_bit_num);
    }

    inline uint8_t compute_table_index_byte4(TableType t1, TableType t2, TableType t3, TableType t4, uint32_t tag_bit_num) const
    {
        uint32_t max_row_num = std::max({get_table_row_num(t1), get_table_row_num(t2), get_table_row_num(t3), get_table_row_num(t4)});
        return compute_index_byte(max_row_num, tag_bit_num);
    }

    inline uint8_t compute_table_index_bytes(const TableType* types, size_t type_count, uint32_t tag_bit_num) const
    {
        uint32_t max_row_num = 0;
        for (size_t i = 0; i < type_count; ++i)
        {
            max_row_num = std::max(max_row_num, get_table_row_num(types[i]));
        }
        return compute_index_byte(max_row_num, tag_bit_num);
    }

    // Table initialization
    void init_table_field_metas(alloc::MemPool& pool);
    void init_table_metas_final();

    // Helper for reading column values (inline implementations)
    static inline uint8_t read_column_u8(const uint8_t* row_data, const CliTableFieldMeta& field_meta)
    {
        return row_data[field_meta.offset];
    }

    static inline uint16_t read_column_u16(const uint8_t* row_data, const CliTableFieldMeta& field_meta)
    {
        return utils::MemOp::read_u16_may_unaligned(row_data + field_meta.offset);
    }

    static inline uint32_t read_column_u32(const uint8_t* row_data, const CliTableFieldMeta& field_meta)
    {
        if (field_meta.size == 4)
        {
            return utils::MemOp::read_u32_may_unaligned(row_data + field_meta.offset);
        }
        else
        {
            assert(field_meta.size == 2);
            return utils::MemOp::read_u16_may_unaligned(row_data + field_meta.offset);
        }
    }

    const uint8_t* image_data;
    size_t image_length;
    const CliSection* sections;
    uint16_t section_count;
    uint32_t metadata_offset;
    uint32_t metadata_length;
    EncodedTokenId entry_point_token;

    CliHeap string_heap;
    CliHeap us_heap;
    CliHeap blob_heap;
    CliHeap guid_heap;
    CliHeap tables_heap;
    CliHeap pdb_heap;

    bool string_heap_size_4_byte;
    bool guid_heap_size_4_byte;
    bool blob_heap_size_4_byte;
};

} // namespace metadata
} // namespace leanclr
