#include <cstdlib>
#include <vector>

#include "alloc/general_allocation.h"
#include "metadata/pe_image_reader.h"
#include "alloc/mem_pool.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/settings.h"
#include "vm/runtime.h"
#include "metadata/module_def.h"
#include "vm/class.h"
#include "vm/method.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace leanclr;

// Define load_assembly_file using EM_JS so JavaScript can implement it
#ifdef EMSCRIPTEN
EM_JS(int32_t, load_assembly_file, (const char* assembly_name, const char* extension, byte** out_buf, size_t* out_size), {
    // This is a wrapper that JavaScript will override
    // The actual implementation is provided by Module.load_assembly_file
    if (typeof Module.load_assembly_file == 'function')
    {
        return Module.load_assembly_file(assembly_name, extension, out_buf, out_size);
    }
    console.error('load_assembly_file not implemented');
    return 1;
});
#else
// For native compilation, declare it as external
extern "C" int32_t load_assembly_file(const char* assembly_name, const char* extension, byte** out_buf, size_t* out_size);
#endif

extern "C" EMSCRIPTEN_KEEPALIVE void* allocate_bytes(size_t size)
{
    return alloc::GeneralAllocation::malloc_zeroed(size);
}

extern "C" EMSCRIPTEN_KEEPALIVE metadata::RtAssembly* load_assembly(const char* assembly_name)
{
    auto ret = vm::Assembly::load_by_name(assembly_name);
    if (ret.is_err())
    {
        printf("Failed to load assembly: %s err:%d\n", assembly_name, (int32_t)ret.unwrap_err());
        return nullptr;
    }
    return ret.unwrap();
}

extern "C" EMSCRIPTEN_KEEPALIVE int32_t invoke_method(metadata::RtAssembly* assembly, const char* type_name, const char* method_name)
{
    metadata::RtModuleDef* mod = assembly->mod;
    auto ret = mod->get_class_by_nested_full_name(type_name, false, true);
    if (ret.is_err())
    {
        printf("Failed to find type: %s err:%d\n", type_name, (int32_t)ret.unwrap_err());
        return 1;
    }
    metadata::RtClass* klass = ret.unwrap();

    auto ret_init = vm::Class::initialize_all(klass);
    if (ret_init.is_err())
    {
        printf("Failed to initialize type: %s err:%d\n", type_name, (int32_t)ret_init.unwrap_err());
        return 1;
    }

    const metadata::RtMethodInfo* method_def = vm::Method::find_matched_method_in_class_by_name(klass, method_name);
    if (!method_def)
    {
        printf("Failed to find method: %s in type: %s\n", method_name, type_name);
        return 2;
    }
    if (!vm::Method::is_static(method_def))
    {
        printf("Entry method must be static\n");
        return 3;
    }

    if (vm::Method::get_param_count_include_this(method_def) != 0)
    {
        printf("Entry method must have no parameters\n");
        return 4;
    }

    if (vm::Method::contains_not_instantiated_generic_param(method_def))
    {
        printf("Entry method must not be generic\n");
        return 5;
    }
    auto ret2 = vm::Runtime::invoke_with_run_cctor(method_def, nullptr, nullptr);
    if (ret2.is_err())
    {
        printf("Failed to invoke method: %s err:%d\n", method_name, (int32_t)ret2.unwrap_err());
        return 6;
    }
    return 0;
}

static bool local_assembly_loader(const char* assembly_name, const char* extension, vm::FileData& file_data)
{
    byte* buf = nullptr;
    size_t size = 0;
    int32_t res = load_assembly_file(assembly_name, extension, &buf, &size);
    if (res != 0)
    {
        return false;
    }
    file_data.data = buf;
    file_data.length = size;
    file_data.shared = false;
    return true;
}

// Runtime initialization flag
static bool runtime_initialized = false;

extern "C" EMSCRIPTEN_KEEPALIVE int32_t initialize_runtime()
{
    if (runtime_initialized)
    {
        return 0; // Already initialized
    }

    vm::Settings::set_file_loader(local_assembly_loader);
    auto result = vm::Runtime::initialize();
    if (result.is_err())
    {
        return -1;
    }
    runtime_initialized = true;
    return 0;
}

// int main()
// {
// #ifndef EMSCRIPTEN
//     return initialize_runtime();
// #else
//     // For WASM, initialization will be called from JavaScript
//     return 0;
// #endif
// }
