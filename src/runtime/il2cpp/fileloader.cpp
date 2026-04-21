#include "fileloader.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>

namespace leanclr
{
namespace il2cpp
{

namespace
{
struct AssemblyEntry
{
    const char* name;
    uint32_t size;
    const uint8_t* file_data;
};

static bool s_bundle_load_attempted = false;
static bool s_bundle_loaded_ok = false;
// Cache the entire global-metadata.dat in memory.
static std::vector<uint8_t> s_cached_bundle_data;
// Entries point directly into s_cached_bundle_data.
static std::vector<AssemblyEntry> s_cached_assembly_entries;

static inline uint32_t read_u32_le(const uint8_t* p)
{
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

static inline size_t align4(size_t v)
{
    return (v + 3) & ~static_cast<size_t>(3);
}

static std::string build_global_metadata_path()
{
    const char* data_dir = vm::Settings::get_data_dir();
    if (data_dir == nullptr || data_dir[0] == '\0')
    {
        return {};
    }

    std::string base(data_dir);
    if (!base.empty() && base.back() != '/' && base.back() != '\\')
    {
        base.push_back('/');
    }

    // Caller says Settings::get_data_dir() is the directory of global-metadata.dat.
    return base + "Metadata/global-metadata.dat";
}

static RtResultVoid load_global_metadata_bundle_once()
{
    s_cached_bundle_data.clear();
    s_cached_assembly_entries.clear();
    s_bundle_loaded_ok = false;

    std::string dat_path = build_global_metadata_path();
    if (dat_path.empty())
    {
        assert(false && "data_dir is not set; global-metadata.dat path is empty");
        return RtErr::ArgumentNull;
    }

    std::ifstream file(dat_path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        assert(false && "global-metadata.dat not found");
        printf("global-metadata.dat not found, data_path='%s'\n", dat_path.c_str());
        return RtErr::FileNotFound;
    }
    printf("global-metadata.dat found, data_path='%s'\n", dat_path.c_str());

    const std::streamoff file_size = file.tellg();
    if (file_size < 8)
    {
        assert(false && "global-metadata.dat is too small");
        return RtErr::BadImageFormat;
    }
    file.seekg(0, std::ios::beg);

    s_cached_bundle_data.resize(static_cast<size_t>(file_size));
    if (!file.read(reinterpret_cast<char*>(s_cached_bundle_data.data()), static_cast<std::streamsize>(file_size)))
    {
        assert(false && "failed to read global-metadata.dat");
        return RtErr::FileNotFound;
    }

    if (!(s_cached_bundle_data[0] == 'C' && s_cached_bundle_data[1] == 'O' && s_cached_bundle_data[2] == 'P' && s_cached_bundle_data[3] == 'H'))
    {
        assert(false && "invalid global-metadata.dat signature, expected COPH");
        printf("invalid global-metadata.dat signature, expected COPH, data_path='%s'\n", dat_path.c_str());
        return RtErr::BadImageFormat;
    }

    const uint32_t assembly_count = read_u32_le(s_cached_bundle_data.data() + 4);
    size_t cursor = 8;
    s_cached_assembly_entries.reserve(assembly_count);
    std::vector<uint32_t> rel_offsets;
    rel_offsets.reserve(assembly_count);

    for (uint32_t i = 0; i < assembly_count; i++)
    {
        if (cursor >= s_cached_bundle_data.size())
        {
            assert(false && "truncated global-metadata.dat while reading assembly entries");
            printf("truncated global-metadata.dat while reading assembly entries, data_path='%s'\n", dat_path.c_str());
            return RtErr::BadImageFormat;
        }

        size_t name_end = cursor;
        while (name_end < s_cached_bundle_data.size() && s_cached_bundle_data[name_end] != 0)
        {
            name_end++;
        }
        if (name_end >= s_cached_bundle_data.size())
        {
            assert(false && "assembly name in global-metadata.dat is not null-terminated");
            printf("assembly name in global-metadata.dat is not null-terminated, data_path='%s'\n", dat_path.c_str());
            return RtErr::BadImageFormat;
        }

        const size_t name_len_with_null = (name_end - cursor) + 1;
        const size_t name_block_len = align4(name_len_with_null);
        if (cursor + name_block_len + 8 > s_cached_bundle_data.size())
        {
            assert(false && "truncated assembly info record in global-metadata.dat");
            printf("truncated assembly info record in global-metadata.dat, data_path='%s'\n", dat_path.c_str());
            return RtErr::BadImageFormat;
        }

        AssemblyEntry e;
        e.name = reinterpret_cast<const char*>(s_cached_bundle_data.data() + cursor);

        cursor += name_block_len;
        e.size = read_u32_le(s_cached_bundle_data.data() + cursor);
        const uint32_t rel_offset = read_u32_le(s_cached_bundle_data.data() + cursor + 4);
        cursor += 8;

        s_cached_assembly_entries.push_back(e);
        rel_offsets.push_back(rel_offset);
    }

    const size_t assembly_bytes_base = cursor;
    for (size_t i = 0; i < s_cached_assembly_entries.size(); i++)
    {
        AssemblyEntry& e = s_cached_assembly_entries[i];
        const uint32_t rel_offset = rel_offsets[i];
        const size_t abs_offset = assembly_bytes_base + static_cast<size_t>(rel_offset);
        const size_t abs_end = abs_offset + static_cast<size_t>(e.size);
        if (abs_end > s_cached_bundle_data.size())
        {
            assert(false && "assembly byte range out of bounds in global-metadata.dat");
            printf("assembly byte range out of bounds in global-metadata.dat, data_path='%s'\n", dat_path.c_str());
            return RtErr::BadImageFormat;
        }
        e.file_data = s_cached_bundle_data.data() + abs_offset;
    }

    s_bundle_loaded_ok = true;
    RET_VOID_OK();
}
} // namespace

RtResult<vm::FileData> assembly_file_loader(const char* assembly_name, const char* extension)
{
    if (assembly_name == nullptr || extension == nullptr)
    {
        return RtErr::ArgumentNull;
    }

    if (!s_bundle_load_attempted)
    {
        s_bundle_load_attempted = true;
        auto load_ret = load_global_metadata_bundle_once();
        assert(load_ret.is_ok() && "failed to load/parse global-metadata.dat");
        if (load_ret.is_err())
        {
            return load_ret.unwrap_err();
        }
    }
    if (!s_bundle_loaded_ok)
    {
        return RtErr::FileNotFound;
    }

    std::string ext(extension);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (ext != "dll")
    {
        // global-metadata.dat bundle currently stores managed assemblies only.
        return RtErr::FileNotFound;
    }

    const AssemblyEntry* matched = nullptr;
    for (const auto& e : s_cached_assembly_entries)
    {
        if (std::strcmp(e.name, assembly_name) == 0)
        {
            matched = &e;
            break;
        }
    }
    if (!matched)
    {
        return RtErr::FileNotFound;
    }
    return vm::FileData{matched->file_data, matched->size};
}

} // namespace il2cpp
} // namespace leanclr