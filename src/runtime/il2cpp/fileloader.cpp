#include "fileloader.h"

namespace leanclr
{
namespace il2cpp
{

static std::vector<std::string> g_lib_dirs;

RtResult<vm::FileData> assembly_file_loader(const char* assembly_name, const char* extension)
{
    for (const auto& dir : g_lib_dirs)
    {
        std::string file_path = dir + "/" + assembly_name + "." + extension;
        std::ifstream dll_file(file_path, std::ios::binary | std::ios::ate);
        if (!dll_file.is_open())
        {
            continue; // Try next directory
        }

        std::streamsize file_size = dll_file.tellg();
        dll_file.seekg(0, std::ios::beg);

        auto* dll_data = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(file_size));
        if (!dll_data)
        {
            return RtErr::OutOfMemory;
        }

        if (!dll_file.read(reinterpret_cast<char*>(dll_data), file_size))
        {
            alloc::GeneralAllocation::free(dll_data);
            continue;
        }
        dll_file.close();

        return vm::FileData{dll_data, static_cast<size_t>(file_size)};
    }

    return RtErr::FileNotFound;
}

} // namespace il2cpp
} // namespace leanclr