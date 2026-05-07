#include "assembly.h"

#include "metadata/module_def.h"
#include "metadata/aot_module.h"
#include "alloc/general_allocation.h"
#include "alloc/mem_pool.h"
#include "utils/rt_unique_ptr.h"
#include "metadata/pe_image_reader.h"
#include "const_strs.h"
#include "settings.h"
#include "rt_array.h"
#include "class.h"
#include "reflection.h"
#include "log/internal_logger.h"

namespace leanclr
{
namespace vm
{

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
    metadata::RtModuleDef* mod = metadata::RtModuleDef::find_module(name);
    if (mod)
    {
        RET_OK(mod->get_assembly());
    }

    auto file_loader = vm::Settings::get_file_loader();
    if (!file_loader)
    {
        RET_ERR(RtErr::FileNotFound);
    }
    auto result = file_loader(name, "dll");
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL2(FileData, dllData, result);

    utils::ByteSpan symbolData;

    auto pdb_ret = file_loader(name, "pdb");
    if (pdb_ret.is_ok())
    {
        auto& pdb_data = pdb_ret.unwrap();
        symbolData = utils::ByteSpan(const_cast<uint8_t*>(pdb_data.data), pdb_data.length);
    }
    else
    {
        log::InternalLogger::debug("PDB file not found for assembly");
    }
    return load_from_data(utils::ByteSpan(const_cast<uint8_t*>(dllData.data), dllData.length), symbolData.data() ? &symbolData : nullptr);
}

RtResult<metadata::RtAssembly*> Assembly::load_by_name(RtAppDomain* app_domain, const char* name_no_ext, RtObject* evidence, bool ref_only,
                                                       RtStackCrawlMark& stack_crawl_mark)
{
    // FIXME
    return load_by_name(name_no_ext);
}

RtResult<metadata::RtAssembly*> Assembly::load_from_data(const utils::Span<byte> dllData, const utils::Span<byte>* ptr_symbol_data)
{
    alloc::MemPool* pool = alloc::GeneralAllocation::new_any<alloc::MemPool>();
    utils::UniquePtr<alloc::MemPool> poolGuard(pool);
    utils::UniquePtr<byte> dllDataGuard(dllData.data());

    metadata::PeImageReader reader(dllData.data(), dllData.size());

    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::CliImage*, image, reader.ReadCliImage(*pool));
    RET_ERR_ON_FAIL(image->load_streams());
    RET_ERR_ON_FAIL(image->load_tables(*pool));

    metadata::PdbImage* pdbImage;
    if (ptr_symbol_data)
    {
        pdbImage = alloc::GeneralAllocation::new_any<metadata::PdbImage>(*pool, ptr_symbol_data->data(), ptr_symbol_data->size());
        auto ret_load = pdbImage->load();
        if (ret_load.is_err())
        {
            log::InternalLogger::error("Failed to load PDB image. Please use portable pdb format. ignoring symbol data");
            // FIXME: free symbol_data.data
            // alloc::GeneralAllocation::free(pdbImage);
            pdbImage = nullptr;
        }
    }
    else
    {
        pdbImage = nullptr;
    };

    metadata::RtAssembly* ass = alloc::GeneralAllocation::malloc_any_zeroed<metadata::RtAssembly>();
    metadata::RtModuleDef* mod = alloc::GeneralAllocation::new_any<metadata::RtModuleDef>(ass, *image, pdbImage, *pool);
    ass->mod = mod;
    RET_ERR_ON_FAIL(mod->load());

    if (metadata::RtModuleDef::find_module(mod->get_name_no_ext()))
    {
        mod->~RtModuleDef();
        RET_ERR(RtErr::ModuleAlreadyLoaded);
    }
    metadata::RtModuleDef::register_module_def(mod);
    const metadata::RtAotModuleData* aotModuleData = metadata::AotModule::find_aot_module_by_name(mod->get_name_no_ext());
    if (aotModuleData != nullptr)
    {
        mod->set_aot_module_data(aotModuleData);
        if (aotModuleData->initializer != nullptr)
        {
            aotModuleData->initializer(mod);
        }
        utils::Vector<metadata::RtAssembly*> ref_assemblies;
        RET_ERR_ON_FAIL(mod->get_reference_assemblies(ref_assemblies));
        // corlib should call deferred_initializer after Class::initialize
        if (!mod->is_corlib() && aotModuleData->deferred_initializer)
        {
            aotModuleData->deferred_initializer(mod);
        }
    }

    // don't free mem pool if succ
    poolGuard.release();
    dllDataGuard.release();
    RET_OK(ass);
}

RtResult<metadata::RtAssembly*> Assembly::load_from_data(RtAppDomain* app_domain, RtArray* dll_data, RtArray* symbol_data, RtObject* evidence, bool ref_only)
{
    if (!dll_data)
    {
        RET_ERR(RtErr::ArgumentNull);
    }
    utils::Span<byte> dll_span(Array::get_array_data_start_as<uint8_t>(dll_data), static_cast<size_t>(Array::get_array_length(dll_data)));
    if (symbol_data)
    {
        utils::Span<byte> symbol_span(Array::get_array_data_start_as<uint8_t>(symbol_data), static_cast<size_t>(Array::get_array_length(symbol_data)));
        return load_from_data(dll_span, &symbol_span);
    }
    else
    {
        return load_from_data(dll_span, nullptr);
    }
}

RtResult<RtArray*> Assembly::get_types(metadata::RtAssembly* ass, bool exported_only)
{
    utils::Vector<metadata::RtClass*> types;
    RET_ERR_ON_FAIL(ass->mod->get_types(exported_only, types));

    metadata::RtClass* cls_type = Class::get_corlib_types().cls_systemtype;
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtArray*, types_arr, Array::new_szarray_from_ele_klass(cls_type, static_cast<int32_t>(types.size())));

    for (size_t i = 0; i < types.size(); ++i)
    {
        DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(RtReflectionType*, ref_type, Reflection::get_klass_reflection_object(types[i]));
        Array::set_array_data_at<RtReflectionType*>(types_arr, static_cast<int32_t>(i), ref_type);
    }
    RET_OK(types_arr);
}

} // namespace vm
} // namespace leanclr
