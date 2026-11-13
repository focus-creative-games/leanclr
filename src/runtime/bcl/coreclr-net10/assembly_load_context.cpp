#include "managed_layouts.h"
#include "assembly_load_context.h"

#include "alloc/general_allocation.h"
#include "alloc/mem_pool.h"
#include "gc/gc_roots.h"
#include "metadata/module_def.h"
#include "metadata/pe_image_reader.h"
#include "utils/scope_guard.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/gc.h"
#include "vm/field.h"
#include "vm/gchandle.h"
#include "vm/object.h"
#include "vm/rt_exception.h"
#include "vm/rt_string.h"

namespace leanclr
{
namespace bcl
{
namespace
{
struct DefaultContext
{
    void* managed_handle = nullptr;
};

DefaultContext default_context;

RtResult<metadata::RtAssembly*> load_exception(const char* type, int32_t hresult, const char* message)
{
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::RtClass*, klass, vm::Assembly::get_corlib()->mod->get_class_by_name2("System.IO", type, false, true));
    RET_ERR_ON_FAIL(vm::Class::initialize_all(klass));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(vm::RtObject*, obj, LEANCLR_NEWOBJ_INTERNAL(klass, "AssemblyLoadContext::load_exception"));
    gc::GcRoots::register_slot(&obj);
    auto root = utils::make_scope_exit([&]() { gc::GcRoots::unregister_slot(&obj); });
    auto* exception = reinterpret_cast<vm::RtException*>(obj);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, hr_field,
                                            vm::Field::find_field_by_name(vm::Class::get_corlib_types().cls_exception, "_HResult"));
    if (!hr_field)
        RET_ERR(RtErr::ExecutionEngine);
    RET_ERR_ON_FAIL(vm::Field::set_instance_value(hr_field, obj, &hresult));
    auto* text = vm::String::create_string_from_utf8cstr(message);
    if (!text)
        RET_ERR(RtErr::OutOfMemory);
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const metadata::RtFieldInfo*, message_field,
                                            vm::Field::find_field_by_name(vm::Class::get_corlib_types().cls_exception, "_message"));
    if (!message_field)
        RET_ERR(RtErr::ExecutionEngine);
    RET_ERR_ON_FAIL(vm::Field::set_instance_value(message_field, obj, &text));
    vm::Exception::set_current_exception(exception);
    RET_ERR(RtErr::ManagedException);
}

bool same_string(const metadata::CliImage& a, uint32_t ai, const metadata::CliImage& b, uint32_t bi)
{
    const auto& ah = a.get_string_heap();
    const auto& bh = b.get_string_heap();
    if (ai >= ah.size || bi >= bh.size)
        return false;
    const auto* ap = ah.data + ai;
    const auto* bp = bh.data + bi;
    const auto* ae = static_cast<const uint8_t*>(std::memchr(ap, 0, ah.size - ai));
    const auto* be = static_cast<const uint8_t*>(std::memchr(bp, 0, bh.size - bi));
    return ae && be && ae - ap == be - bp && std::memcmp(ap, bp, static_cast<size_t>(ae - ap)) == 0;
}

bool same_blob(const metadata::CliImage& a, uint32_t ai, const metadata::CliImage& b, uint32_t bi)
{
    const auto& ah = a.get_blob_heap();
    const auto& bh = b.get_blob_heap();
    if (ai >= ah.size || bi >= bh.size)
        return false;
    utils::BinaryReader ar(ah.data + ai, ah.size - ai), br(bh.data + bi, bh.size - bi);
    uint32_t as, bs;
    return ar.try_read_compressed_uint32(as) && br.try_read_compressed_uint32(bs) && as == bs && as <= ar.remaining() && bs <= br.remaining() &&
           std::memcmp(ar.get_current_ptr(), br.get_current_ptr(), as) == 0;
}

bool same_definition(const metadata::CliImage& a, const metadata::CliImage& b)
{
    auto am = a.read_module(1), bm = b.read_module(1);
    auto aa = a.read_assembly(1), ba = b.read_assembly(1);
    if (!am || !bm || !aa || !ba || !am->mvid || !bm->mvid)
        return false;
    const uint64_t ai = uint64_t(am->mvid - 1) * 16, bi = uint64_t(bm->mvid - 1) * 16;
    const auto& ag = a.get_guid_heap();
    const auto& bg = b.get_guid_heap();
    if (ai > ag.size || 16 > ag.size - ai || bi > bg.size || 16 > bg.size - bi)
        return false;
    return std::memcmp(ag.data + ai, bg.data + bi, 16) == 0 && aa->major_version == ba->major_version && aa->minor_version == ba->minor_version &&
           aa->build_number == ba->build_number && aa->revision_number == ba->revision_number && aa->flags == ba->flags &&
           same_string(a, aa->name, b, ba->name) && same_string(a, aa->locale, b, ba->locale) && same_blob(a, aa->public_key, b, ba->public_key);
}
} // namespace

RtResult<void*> AssemblyLoadContext::initialize(void* managed_handle, bool is_default, bool collectible) noexcept
{
    // GCHandle's target is its first field. Use the indexed lookup before ever
    // dereferencing a caller-supplied handle (pinned handles carry a low tag bit).
    const auto address = reinterpret_cast<uintptr_t>(managed_handle) & ~uintptr_t{1};
    void* handle = vm::GCHandle::get_handle_by_target_slot(reinterpret_cast<vm::RtObject**>(address));
    if (!handle)
        RET_ERR(RtErr::Argument);
    auto* target = vm::GCHandle::get_target(handle);
    if (!target)
        RET_ERR(RtErr::Argument);
    bool context_object = false;
    for (const auto* klass = target->klass; klass; klass = klass->parent)
    {
        if (std::strcmp(klass->namespaze, "System.Runtime.Loader") == 0 && std::strcmp(klass->name, "AssemblyLoadContext") == 0)
        {
            context_object = true;
            break;
        }
    }
    if (!context_object)
        RET_ERR(RtErr::Argument); // ownership stays with an unrelated caller
    if (!is_default || collectible || default_context.managed_handle)
    {
        // CoreLib allocates the handle before this call and has no constructor
        // rollback. A rejected collectible object must not later attempt unload.
        if (handle != default_context.managed_handle)
        {
            vm::GC::suppress_finalize(target);
            vm::GCHandle::free_handle(handle);
        }
        RET_ERR(RtErr::NotSupported);
    }
    default_context.managed_handle = handle; // retained strong root for process lifetime
    RET_OK(static_cast<void*>(&default_context));
}

RtResult<metadata::RtAssembly*> AssemblyLoadContext::load(void* context, const uint8_t* image, int32_t image_size, const uint8_t* symbols,
                                                          int32_t symbols_size) noexcept
{
    if (context != &default_context || !default_context.managed_handle)
        RET_ERR(RtErr::NotSupported);
    if (image_size <= 0 || !image || symbols_size < 0 || (symbols_size && !symbols))
        RET_ERR(RtErr::BadImageFormat);
    auto* copy = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(image_size)));
    if (!copy)
        RET_ERR(RtErr::OutOfMemory);
    auto image_cleanup = utils::make_scope_exit([copy]() { alloc::GeneralAllocation::free(copy); });
    std::memcpy(copy, image, static_cast<size_t>(image_size));
    uint8_t* symbol_copy = nullptr;
    auto symbol_cleanup = utils::make_scope_exit([&]() { alloc::GeneralAllocation::free(symbol_copy); });
    if (symbols_size)
    {
        symbol_copy = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(symbols_size)));
        if (!symbol_copy)
            RET_ERR(RtErr::OutOfMemory);
        std::memcpy(symbol_copy, symbols, static_cast<size_t>(symbols_size));
    }
    // The frontend owns both copies until publication. The shared loader borrows
    // them during its transaction. Dependencies resolve on demand, as in CoreCLR.
    vm::AssemblyData data{copy, static_cast<size_t>(image_size), true};
    vm::AssemblyData pdb{symbol_copy, static_cast<size_t>(symbols_size), true};
    auto result = vm::Assembly::load_from_data(data, symbols_size ? &pdb : nullptr, true);
    if (result.is_ok())
    {
        image_cleanup.dismiss();
        if (result.unwrap()->mod->get_pdb_image())
            symbol_cleanup.dismiss();
        RET_OK(result.unwrap());
    }
    if (result.unwrap_err() != RtErr::ModuleAlreadyLoaded)
        return result.unwrap_err();

    // Duplicate loads reuse the existing definition, including images with
    // different non-semantic PE padding. A different MVID/identity conflicts.
    alloc::MemPool pool;
    metadata::PeImageReader reader(copy, static_cast<size_t>(image_size));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(metadata::CliImage*, parsed, reader.ReadCliImage(pool));
    RET_ERR_ON_FAIL(parsed->load_streams());
    RET_ERR_ON_FAIL(parsed->load_tables(pool));
    DECLARING_AND_UNWRAP_OR_RET_ERR_ON_FAIL(const char*, name, parsed->read_assembly_name_no_ext());
    auto* existing = vm::Assembly::find_by_name(name);
    if (existing && same_definition(*parsed, existing->mod->get_cli_image()))
        RET_OK(existing);
    return load_exception("FileLoadException", static_cast<int32_t>(0x80131621u), "An assembly with the same name is already loaded.");
}
} // namespace bcl
} // namespace leanclr
