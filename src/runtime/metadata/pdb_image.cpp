#include "pdb_image.h"
#include "utils/binary_reader.h"
#include "utils/string_builder.h"
#include "log/internal_logger.h"

namespace leanclr
{
namespace metadata
{

constexpr uint32_t kHiddenLine = 0xFEEFEE;

RtResultVoid PdbImage::load()
{
    RET_ERR_ON_FAIL(_cli_image.load_streams());
    RET_ERR_ON_FAIL(_cli_image.load_tables(_pool));
    RET_VOID_OK();
}

void PdbImage::add_ir2il_map_for_method(const RtMethodInfo* method_info, const MethodIR2ILMapInfo& map_info)
{
    // TODO merge entries in same sequence point
    IR2ILMapEntry* entries_copy = _pool.calloc_any<IR2ILMapEntry>(map_info.entries.size());
    std::memcpy(entries_copy, map_info.entries.data(), sizeof(IR2ILMapEntry) * map_info.entries.size());

    MethodIR2ILMapInfo new_map_info;
    new_map_info.entries = utils::Span<IR2ILMapEntry>{entries_copy, map_info.entries.size()};
    _method_debug_info_map.insert({method_info, new_map_info});
}

void PdbImage::get_debug_info_for_method(const RtMethodInfo* method_info, int32_t ir_offset, int32_t* il_offset, const char** file_name, int32_t* line_number,
                                         int32_t* column_number)
{
    auto it = _method_debug_info_map.find(method_info);
    auto ret_method_data = GetMethodDataFromCache(method_info->token);
    if (it == _method_debug_info_map.end() || ret_method_data.is_err())
    {
        *il_offset = -1;
        *file_name = nullptr;
        *line_number = -1;
        *column_number = -1;
        return;
    }
    const MethodIR2ILMapInfo& map_info = it->second;
    int32_t ilOffset = FindILOffsetByIROffset(map_info.entries, ir_offset - 1);

    *il_offset = static_cast<int32_t>(ilOffset);

    const SymbolMethodDefData* methodData = ret_method_data.unwrap();
    const SymbolSequencePoint* seqPoint = FindSequencePoint(methodData->sequencePoints, ilOffset);
    if (seqPoint == nullptr || seqPoint->line == kHiddenLine)
    {
        *file_name = nullptr;
        *line_number = -1;
        *column_number = -1;
        return;
    }
    *line_number = static_cast<int32_t>(seqPoint->line);
    *column_number = static_cast<int32_t>(seqPoint->column);
    auto ret_doc_name = GetDocumentName(methodData->document);
    if (ret_doc_name.is_ok())
    {
        *file_name = ret_doc_name.unwrap();
    }
    else
    {
        *file_name = nullptr;
    }
}

int32_t PdbImage::FindILOffsetByIROffset(const utils::Span<IR2ILMapEntry>& ilMapper, int32_t irOffset)
{
    auto it =
        std::upper_bound(ilMapper.begin(), ilMapper.end(), irOffset, [](int32_t offset, const IR2ILMapEntry& mapper) { return offset < mapper.ir_offset; });
    if (it != ilMapper.begin())
    {
        auto next = it;
        if (next != ilMapper.end())
        {
            assert(next->ir_offset > irOffset);
        }
        --it;
        assert(it->ir_offset <= irOffset);
        return it->il_offset;
    }
    return 0;
}

const PdbImage::SymbolSequencePoint* PdbImage::FindSequencePoint(const utils::Span<SymbolSequencePoint>& sequencePoints, int32_t ilOffset)
{
    auto it = std::upper_bound(sequencePoints.begin(), sequencePoints.end(), ilOffset,
                               [](int32_t ilOffset, const SymbolSequencePoint& ssp) { return ilOffset < ssp.ilOffset; });
    if (it != sequencePoints.begin())
    {
        auto next = it;
        if (next != sequencePoints.end())
        {
            assert(next->ilOffset > ilOffset);
        }
        --it;
        assert(it->ilOffset <= ilOffset);
        return &(*it);
    }
    return nullptr;
}

RtResult<const PdbImage::SymbolDocumentData*> PdbImage::GetDocument(metadata::EncodedTokenId documentToken)
{
    auto it = _documents.find(documentToken);
    if (it != _documents.end())
    {
        return it->second;
    }

    uint32_t rowIndex = metadata::RtToken::decode_rid(documentToken);
    auto opt_document = _cli_image.read_document(rowIndex);
    if (!opt_document)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    auto& document = opt_document.value();

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, reader, _cli_image.get_decoded_blob_reader(document.name));

    uint8_t sep;
    if (!reader.try_read_byte(sep))
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    utils::StringBuilder sourceFileNames;
    bool first = true;
    while (reader.not_empty())
    {
        if (sep && !first)
        {
            sourceFileNames.append_char(sep);
        }
        uint32_t sourceFileNameIndex;
        if (!reader.try_read_compressed_uint32(sourceFileNameIndex))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        if (sourceFileNameIndex > 0)
        {
            DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, sourceFileNameReader, _cli_image.get_decoded_blob_reader(sourceFileNameIndex));
            sourceFileNames.append_cstr(sourceFileNameReader.data(), sourceFileNameReader.length());
        }
        first = false;
    }

    char* sourceFileNamesCstr = _pool.calloc_any<char>(sourceFileNames.length() + 1);
    std::memcpy(sourceFileNamesCstr, sourceFileNames.get_data(), sourceFileNames.length());
    sourceFileNamesCstr[sourceFileNames.length()] = '\0';

    SymbolDocumentData* documentData = _pool.malloc_any_zeroed<SymbolDocumentData>();
    documentData->sourceFiles = sourceFileNamesCstr;

    _documents.insert({documentToken, documentData});
    return documentData;
}

RtResult<const PdbImage::SymbolMethodDefData*> PdbImage::GetMethodDataFromCache(metadata::EncodedTokenId methodToken)
{
    auto it = _methods.find(methodToken);
    if (it != _methods.end())
    {
        return it->second;
    }

    uint32_t rowIndex = metadata::RtToken::decode_rid(methodToken);
    auto opt_row = _cli_image.read_method_debug_information(rowIndex);
    if (!opt_row)
    {
        RET_ASSERT_ERR(RtErr::BadImageFormat);
    }

    // see https://github.com/dotnet/runtime/blob/main/docs/design/specs/PortablePdb-Metadata.md
    auto& smb = opt_row.value();
    utils::Vector<SymbolSequencePoint> sequencePoints;
    if (smb.sequence_points > 0)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL3(utils::BinaryReader, reader, _cli_image.get_decoded_blob_reader(smb.sequence_points));
        uint32_t localSignature;
        if (!reader.try_read_compressed_uint32(localSignature))
        {
            RET_ASSERT_ERR(RtErr::BadImageFormat);
        }
        uint32_t document;
        if (smb.document)
        {
            document = smb.document;
        }
        else
        {
            if (!reader.try_read_compressed_uint32(document))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
        }
        int32_t prevStartLine = -1;
        int32_t prevStartColumn = -1;
        while (reader.not_empty())
        {
            uint32_t deltaIlOffset;
            if (!reader.try_read_compressed_uint32(deltaIlOffset))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            // document record
            if (deltaIlOffset == 0 && !sequencePoints.empty())
            {
                if (!reader.try_read_compressed_uint32(document))
                {
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                }
                continue;
            }
            uint32_t deltaLines;
            if (!reader.try_read_compressed_uint32(deltaLines))
            {
                RET_ASSERT_ERR(RtErr::BadImageFormat);
            }
            int32_t deltaColumns;
            if (deltaLines == 0)
            {
                uint32_t temp;
                if (!reader.try_read_compressed_uint32(temp))
                {
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                }
                deltaColumns = static_cast<int32_t>(temp);
            }
            else
            {
                if (!reader.try_read_compressed_int32(deltaColumns))
                {
                    RET_ASSERT_ERR(RtErr::BadImageFormat);
                }
            }

            const int32_t ilOffset =
                sequencePoints.empty() ? static_cast<int32_t>(deltaIlOffset) : sequencePoints.back().ilOffset + static_cast<int32_t>(deltaIlOffset);

            SymbolSequencePoint ssp = {};
            ssp.document = document;
            ssp.ilOffset = ilOffset;
            // hidden-sequence-point record
            if (deltaLines == 0 && deltaColumns == 0)
            {
                ssp.line = ssp.endLine = kHiddenLine;
                ssp.column = ssp.endColumn = 0;
            }
            else
            {
                // sequence-point record
                if (prevStartColumn < 0)
                {
                    uint32_t tempPrevStartLine;
                    if (!reader.try_read_compressed_uint32(tempPrevStartLine))
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    prevStartLine = static_cast<int32_t>(tempPrevStartLine);

                    uint32_t tempPrevStartColumn;
                    if (!reader.try_read_compressed_uint32(tempPrevStartColumn))
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    prevStartColumn = static_cast<int32_t>(tempPrevStartColumn);
                }
                else
                {
                    int32_t temp_delta_start_line;
                    if (!reader.try_read_compressed_int32(temp_delta_start_line))
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    prevStartLine += temp_delta_start_line;
                    int32_t temp_delta_start_column;
                    if (!reader.try_read_compressed_int32(temp_delta_start_column))
                    {
                        RET_ASSERT_ERR(RtErr::BadImageFormat);
                    }
                    prevStartColumn += temp_delta_start_column;
                }
                ssp.line = static_cast<uint32_t>(prevStartLine);
                ssp.endLine = static_cast<uint32_t>(prevStartLine) + deltaLines;
                ssp.column = static_cast<uint32_t>(prevStartColumn);
                ssp.endColumn = static_cast<uint32_t>(static_cast<int64_t>(prevStartColumn) + static_cast<int64_t>(deltaColumns));
            }
            sequencePoints.push_back(ssp);
        }
    }
    SymbolMethodDefData* methodData = _pool.malloc_any_zeroed<SymbolMethodDefData>();
    methodData->document = smb.document;
    if (!sequencePoints.empty())
    {
        SymbolSequencePoint* seqPointsCopy = _pool.calloc_any<SymbolSequencePoint>(sequencePoints.size());
        std::memcpy(seqPointsCopy, sequencePoints.data(), sizeof(SymbolSequencePoint) * sequencePoints.size());
        methodData->sequencePoints = utils::Span<SymbolSequencePoint>{seqPointsCopy, sequencePoints.size()};
    }
    else
    {
        methodData->sequencePoints = utils::Span<SymbolSequencePoint>{nullptr, 0};
    }

    _methods.insert({methodToken, methodData});
    return methodData;
}
} // namespace metadata
} // namespace leanclr