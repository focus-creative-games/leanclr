#include "assembly.h"

#include <cstring>

#include "metadata/module_def.h"
#include "metadata/aot_module.h"
#include "alloc/general_allocation.h"
#include "alloc/mem_pool.h"
#include "utils/scope_guard.h"
#include "metadata/pe_image_reader.h"
#include "const_strs.h"
#include "settings.h"
#include "rt_array.h"
#include "class.h"
#include "reflection.h"
#include "type.h"
#include "log/internal_logger.h"

namespace leanclr
{
namespace vm
{

static utils::Vector<metadata::RtModuleDef*> g_placeholder_modules;

static metadata::RtModuleDef* create_placeholder_assembly(const char* name)
{
    metadata::RtAssembly* ass = alloc::GeneralAllocation::malloc_any_zeroed<metadata::RtAssembly>();
    metadata::CliImage* cliImage = alloc::GeneralAllocation::malloc_any_zeroed<metadata::CliImage>();
    alloc::MemPool* pool = alloc::GeneralAllocation::new_any<alloc::MemPool>();
    metadata::RtModuleDef* mod = alloc::GeneralAllocation::malloc_any_zeroed<metadata::RtModuleDef>();
    ass->mod = mod;
    new (mod) metadata::RtModuleDef(ass, *cliImage, nullptr, *pool);
    mod->init_as_placeholder_module(name);
    metadata::RtModuleDef::register_module_def(mod);
    return mod;
}

static RtResultVoid create_placeholder_assemblies()
{
#ifdef LEANCLR_PLACEHOLDER_ASSEMBLY_NAMES
    const char* placeholder_assembly_names = LEANCLR_TOSTRING(LEANCLR_PLACEHOLDER_ASSEMBLY_NAMES);
#else
    const char* placeholder_assembly_names = "";
#endif
    const char* cur = placeholder_assembly_names;
    while (*cur != '\0')
    {
        const char* comma = std::strchr(cur, '+');
        const char* token_end = comma != nullptr ? comma : cur + std::strlen(cur);
        size_t length = static_cast<size_t>(token_end - cur);
        char* name = (char*)alloc::GeneralAllocation::malloc(length + 1);
        std::memcpy(name, cur, length);
        name[length] = '\0';
        metadata::RtModuleDef* mod = create_placeholder_assembly(name);
        g_placeholder_modules.push_back(mod);
        if (comma != nullptr)
        {
            cur = comma + 1;
        }
        else
        {
            break;
        }
    }
    RET_VOID_OK();
}

static metadata::RtModuleDef* find_placeholder_assembly(const char* name)
{
    for (metadata::RtModuleDef* mod : g_placeholder_modules)
    {
        if (strcmp(mod->get_name_no_ext(), name) == 0)
        {
            return mod;
        }
    }
    return nullptr;
}

static metadata::RtModuleDef* find_and_remove_placeholder_assembly(const char* name)
{
    for (auto it = g_placeholder_modules.begin(); it != g_placeholder_modules.end(); ++it)
    {
        if (strcmp((*it)->get_name_no_ext(), name) == 0)
        {
            metadata::RtModuleDef* mod = *it;
            g_placeholder_modules.erase(it);
            return mod;
        }
    }
    return nullptr;
}

RtResultVoid Assembly::initialize()
{
    return create_placeholder_assemblies();
}

RtResult<metadata::RtAssembly*> Assembly::load_corlib()
{
    return load_by_name(STR_CORLIB_NAME);
}

metadata::RtAssembly* Assembly::get_corlib()
{
    auto corlibMod = metadata::RtModuleDef::get_corlib_module();
    auto corlibAss = corlibMod ? corlibMod->get_assembly() : nullptr;
    return corlibAss;
}

metadata::RtAssembly* Assembly::find_by_name(const char* name_no_ext)
{
    metadata::RtModuleDef* mod = metadata::RtModuleDef::find_module(name_no_ext);
    return mod ? mod->get_assembly() : nullptr;
}

RtResult<metadata::RtAssembly*> Assembly::load_by_name(const char* name)
{
    FullyQualifiedAssemblyName qn(name, std::strlen(name));
    RET_ERR_ON_FAIL(qn.parse());
    std::string assembly_anme(qn.name(), qn.name_length());
    metadata::RtModuleDef* mod = metadata::RtModuleDef::find_module(assembly_anme.c_str());
    if (mod)
    {
        RET_OK(mod->get_assembly());
    }
    auto file_loader = vm::Settings::get_file_loader();
    if (!file_loader)
    {
        RET_ERR(RtErr::FileNotFound);
    }
    FileData dllFileData = {};
    auto result = file_loader(name, "dll", dllFileData);
    if (!file_loader(name, "dll", dllFileData))
    {
        metadata::RtModuleDef* placeholder_mod = find_placeholder_assembly(name);
        if (placeholder_mod)
        {
            RET_OK(placeholder_mod->get_assembly());
        }
        std::printf("Failed to load assembly from file loader for %s\n", name);
        RET_ERR(RtErr::FileNotFound);
    }
    AssemblyData dllData{dllFileData.data, dllFileData.length, dllFileData.shared};

    AssemblyData* pdb_data_ptr = nullptr;
    AssemblyData pdb_data;
    FileData pdb_file_data = {};
    if (file_loader(name, "pdb", pdb_file_data))
    {
        pdb_data = AssemblyData{pdb_file_data.data, pdb_file_data.length, pdb_file_data.shared};
        pdb_data_ptr = &pdb_data;
    }
    else
    {
        log::InternalLogger::debug("PDB file not found for assembly");
    }

    return load_from_data(dllData, pdb_data_ptr);
}

RtResult<metadata::RtAssembly*> Assembly::load_by_name(RtAppDomain* app_domain, const char* name_no_ext, RtObject* evidence, bool ref_only,
                                                       RtStackCrawlMark& stack_crawl_mark)
{
    // FIXME
    return load_by_name(name_no_ext);
}

RtResult<metadata::RtAssembly*> Assembly::load_from_data(const AssemblyData& dll_data, const AssemblyData* symbol_data_ptr)
{
    utils::ScopeGuard cleanup_guard;
    cleanup_guard.register_cleanup(
        [dll_data]()
        {
            if (!dll_data.shared)
            {
                alloc::GeneralAllocation::free(const_cast<uint8_t*>(dll_data.data));
            }
        });
    cleanup_guard.register_cleanup(
        [symbol_data_ptr]()
        {
            if (symbol_data_ptr && !symbol_data_ptr->shared)
            {
                alloc::GeneralAllocation::free(const_cast<uint8_t*>(symbol_data_ptr->data));
            }
        });
    alloc::MemPool* pool = alloc::GeneralAllocation::new_any<alloc::MemPool>();
    cleanup_guard.register_cleanup([pool]() { alloc::GeneralAllocation::delete_any(pool); });

    metadata::PeImageReader reader(dll_data.data, dll_data.length);

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::CliImage*, image, reader.ReadCliImage(*pool));
    RET_ERR_ON_FAIL(image->load_streams());
    RET_ERR_ON_FAIL(image->load_tables(*pool));

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name_no_ext, image->read_assembly_name_no_ext());

    metadata::RtModuleDef* placeholder_mod = find_and_remove_placeholder_assembly(name_no_ext);

    if (metadata::RtModuleDef::find_module(name_no_ext) && !placeholder_mod)
    {
        RET_ERR(RtErr::ModuleAlreadyLoaded);
    }

    metadata::PdbImage* pdbImage;
    if (symbol_data_ptr)
    {
        pdbImage = alloc::GeneralAllocation::new_any<metadata::PdbImage>(*pool, symbol_data_ptr->data, symbol_data_ptr->length);
        auto ret_load = pdbImage->load();
        if (ret_load.is_err())
        {
            log::InternalLogger::error("Failed to load PDB image. Please use portable pdb format. ignoring symbol data");
            alloc::GeneralAllocation::delete_any(pdbImage);
            pdbImage = nullptr;
        }
    }
    else
    {
        pdbImage = nullptr;
    }

    metadata::RtAssembly* ass = nullptr;
    metadata::RtModuleDef* mod = nullptr;
    uint32_t module_id = 0;
    if (placeholder_mod)
    {
        ass = placeholder_mod->get_assembly();
        mod = placeholder_mod;
        module_id = placeholder_mod->get_id();
        placeholder_mod->destroy_placeholder_datas();
        std::memset((void*)mod, 0, sizeof(metadata::RtModuleDef));
        new (mod) metadata::RtModuleDef(ass, *image, pdbImage, *pool);
    }
    else
    {
        ass = alloc::GeneralAllocation::malloc_any_zeroed<metadata::RtAssembly>();
        mod = alloc::GeneralAllocation::new_any<metadata::RtModuleDef>(ass, *image, pdbImage, *pool);
        ass->mod = mod;
    }
    cleanup_guard.register_cleanup(
        [ass, mod]()
        {
            alloc::GeneralAllocation::free(ass);
            alloc::GeneralAllocation::delete_any(mod);
        });
    RET_ERR_ON_FAIL(mod->load(module_id));

    if (placeholder_mod == nullptr)
    {
        metadata::RtModuleDef::register_module_def(mod);
    }
    cleanup_guard.register_cleanup(
        [mod]()
        {
            metadata::RtModuleDef::unregister_module_def(mod);
            std::printf("Failed to load reference assemblies of module %s\n", mod->get_name_no_ext());
        });
    utils::Vector<metadata::RtAssembly*> ref_assemblies;
    RET_ERR_ON_FAIL(mod->get_reference_assemblies(ref_assemblies));
    const metadata::RtAotModuleData* aotModuleData = metadata::AotModule::find_aot_module_by_name(mod->get_name_no_ext());
    if (aotModuleData != nullptr)
    {
        mod->set_aot_module_data(aotModuleData);
        if (aotModuleData->initializer != nullptr)
        {
            aotModuleData->initializer(mod);
        }
        // corlib should call deferred_initializer after Class::initialize
        if (!mod->is_corlib() && aotModuleData->deferred_initializer)
        {
            aotModuleData->deferred_initializer(mod);
        }
    }

    // don't free mem pool if succ
    cleanup_guard.set_not_cleanup();
    RET_OK(ass);
}

RtResult<metadata::RtAssembly*> Assembly::load_from_data(RtAppDomain* app_domain, RtArray* dll_data, RtArray* symbol_data, RtObject* evidence, bool ref_only)
{
    if (!dll_data)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    size_t dll_data_len = static_cast<size_t>(Array::get_array_length(dll_data));
    byte* dup_dll_data = (byte*)utils::MemOp::dup_mem(Array::get_array_data_start_as<uint8_t>(dll_data), dll_data_len);
    AssemblyData dll_data_wrap{dup_dll_data, dll_data_len, false};
    if (symbol_data)
    {
        size_t symbol_data_len = static_cast<size_t>(Array::get_array_length(symbol_data));
        byte* dup_symbol_data = (byte*)utils::MemOp::dup_mem(Array::get_array_data_start_as<uint8_t>(symbol_data), symbol_data_len);
        AssemblyData symbolData{dup_symbol_data, symbol_data_len, false};
        return load_from_data(dll_data_wrap, &symbolData);
    }
    else
    {
        return load_from_data(dll_data_wrap, nullptr);
    }
}

RtResult<RtArray*> Assembly::get_types(metadata::RtAssembly* ass, bool exported_only)
{
    utils::Vector<metadata::RtClass*> types;
    RET_ERR_ON_FAIL(ass->mod->get_types(exported_only, types));

    metadata::RtClass* cls_type = Class::get_corlib_types().cls_systemtype;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, types_arr,
                                            LEANCLR_NEW_SZARRAY_FROM_ELE_KLASS_INTERNAL(cls_type, static_cast<int32_t>(types.size()), "Assembly::get_types"));

    for (size_t i = 0; i < types.size(); ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, Reflection::get_klass_reflection_object(types[i]));
        Array::set_array_data_at<RtReflectionType*>(types_arr, static_cast<int32_t>(i), ref_type);
    }
    RET_OK(types_arr);
}

} // namespace vm
} // namespace leanclr
