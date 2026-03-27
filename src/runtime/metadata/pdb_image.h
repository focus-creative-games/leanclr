#pragma once

#include "cli_image.h"
#include "utils/rt_vector.h"
#include "rt_metadata.h"
#include "utils/hashmap.h"
#include "utils/rt_span.h"

namespace leanclr
{
namespace metadata
{
struct IR2ILMapEntry
{
    int32_t il_offset;
    int32_t ir_offset;
};

struct MethodIR2ILMapInfo
{
    utils::Span<IR2ILMapEntry> entries;
};

class PdbImage
{
  public:
    PdbImage(alloc::MemPool& pool, const uint8_t* data, size_t length) : _pool(pool), _cli_image(), _default_section{0, static_cast<uint32_t>(length), 0}
    {
        _cli_image.set_image_data(data, length);
        _cli_image.set_metadata_offset_length(0, static_cast<uint32_t>(length));
        _cli_image.set_entry_point_token(0);
        _cli_image.set_sections(&_default_section, 1);
    }

    RtResultVoid load();

    void add_ir2il_map_for_method(const RtMethodInfo* method_info, const MethodIR2ILMapInfo& map_info);
    void get_debug_info_for_method(const RtMethodInfo* method_info, int32_t ir_offset, int32_t* il_offset, const char** file_name, int32_t* line_number,
                                   int32_t* column_number);

  private:
    alloc::MemPool& _pool;
    CliSection _default_section;
    CliImage _cli_image;

    utils::HashMap<const RtMethodInfo*, MethodIR2ILMapInfo> _method_debug_info_map;

    struct SymbolDocumentData
    {
        const char* sourceFiles;
    };

    struct SymbolSequencePoint
    {
        uint32_t document;
        int32_t ilOffset;
        uint32_t line;
        uint32_t column;
        uint32_t endLine;
        uint32_t endColumn;
    };

    struct SymbolMethodDefData
    {
        uint32_t document;
        utils::Span<SymbolSequencePoint> sequencePoints;
    };

    typedef utils::HashMap<uint32_t, SymbolMethodDefData*> SymbolMethodDataMap;
    SymbolMethodDataMap _methods;

    typedef utils::HashMap<uint32_t, SymbolDocumentData*> SymbolDocumentDataMap;
    SymbolDocumentDataMap _documents;

    static int32_t FindILOffsetByIROffset(const utils::Span<IR2ILMapEntry>& entries, int32_t irOffset);
    static const SymbolSequencePoint* FindSequencePoint(const utils::Span<SymbolSequencePoint>& sequencePoints, int32_t ilOffset);

    RtResult<const SymbolMethodDefData*> GetMethodDataFromCache(metadata::EncodedTokenId methodToken);
    RtResult<const SymbolDocumentData*> GetDocument(metadata::EncodedTokenId documentToken);

    RtResult<const char*> GetDocumentName(metadata::EncodedTokenId documentToken)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const SymbolDocumentData*, document, GetDocument(documentToken));
        return document ? document->sourceFiles : nullptr;
    }
};
} // namespace metadata
} // namespace leanclr