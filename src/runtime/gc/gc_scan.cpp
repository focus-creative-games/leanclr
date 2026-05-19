#include "gc_scan.h"

#include "vm/rt_array.h"
#include "vm/class.h"
#include "vm/field.h"

namespace leanclr
{
namespace gc
{

static void mark_ref_slot(vm::RtObject** slot, GcMarkFn mark_fn, void* userdata)
{
    assert(slot);
    vm::RtObject* child = *slot;
    if (child != nullptr)
    {
        mark_fn(child, userdata);
    }
}

static void scan_with_bitmap(vm::RtObject* obj, const metadata::RtClass* klass, GcMarkFn mark_fn, void* userdata)
{
    const uintptr_t* bitmap = klass->gc_bitmap;
    const size_t word_count = klass->gc_bitmap_word_count;
    const size_t object_size = sizeof(vm::RtObject) + klass->instance_size_without_header;
    const size_t slot_count = (object_size + sizeof(void*) - 1) / sizeof(void*);
    uint8_t* obj_bytes = reinterpret_cast<uint8_t*>(obj);

    for (size_t bit = 0; bit < slot_count; ++bit)
    {
        const size_t word = bit / (sizeof(uintptr_t) * 8);
        if (word >= word_count)
        {
            break;
        }
        const size_t shift = bit % (sizeof(uintptr_t) * 8);
        if ((bitmap[word] & (static_cast<uintptr_t>(1) << shift)) == 0)
        {
            continue;
        }
        vm::RtObject** slot = reinterpret_cast<vm::RtObject**>(obj_bytes + bit * sizeof(void*));
        mark_ref_slot(slot, mark_fn, userdata);
    }
}

static void scan_reference_fields(vm::RtObject* obj, const metadata::RtClass* klass, GcMarkFn mark_fn, void* userdata)
{
    utils::Vector<const metadata::RtFieldInfo*> fields;
    vm::Class::collect_instance_fields(klass, fields, true);
    for (size_t i = 0; i < fields.size(); ++i)
    {
        const metadata::RtFieldInfo* field = fields[i];
        if (!vm::Field::is_instance(field))
        {
            continue;
        }
        bool is_ref = false;
        const metadata::RtTypeSig* sig = field->type_sig;
        if (sig->ele_type == metadata::RtElementType::Class || sig->ele_type == metadata::RtElementType::Object ||
            sig->ele_type == metadata::RtElementType::String || sig->ele_type == metadata::RtElementType::Array ||
            sig->ele_type == metadata::RtElementType::SZArray)
        {
            is_ref = true;
        }
        else if (sig->ele_type == metadata::RtElementType::ValueType || sig->ele_type == metadata::RtElementType::GenericInst)
        {
            RtResult<metadata::RtClass*> field_class_res = vm::Class::get_class_from_typesig(sig);
            if (field_class_res.is_ok())
            {
                is_ref = vm::Class::get_has_references(field_class_res.unwrap());
            }
        }
        if (!is_ref)
        {
            continue;
        }
        uint32_t offset = vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field);
        vm::RtObject** slot = reinterpret_cast<vm::RtObject**>(reinterpret_cast<uint8_t*>(obj) + offset);
        mark_ref_slot(slot, mark_fn, userdata);
    }
}

void gc_scan_object_refs(vm::RtObject* obj, GcMarkFn mark_fn, void* userdata)
{
    if (obj == nullptr || obj->klass == nullptr)
    {
        return;
    }
    const metadata::RtClass* klass = obj->klass;
    if (!vm::Class::get_has_references(klass))
    {
        return;
    }

    if (vm::Class::is_array_or_szarray(klass))
    {
        vm::RtArray* arr = reinterpret_cast<vm::RtArray*>(obj);
        const metadata::RtClass* ele_class = klass->element_class;
        if (ele_class == nullptr || !vm::Class::get_has_references(ele_class))
        {
            return;
        }
        vm::RtObject** data = vm::Array::get_array_data_start_as<vm::RtObject*>(arr);
        for (int32_t i = 0; i < arr->length; ++i)
        {
            mark_ref_slot(data + i, mark_fn, userdata);
        }
        return;
    }

    if (klass->gc_bitmap != nullptr && klass->gc_bitmap_word_count > 0)
    {
        scan_with_bitmap(obj, klass, mark_fn, userdata);
    }
    else
    {
        scan_reference_fields(obj, klass, mark_fn, userdata);
    }
}

} // namespace gc
} // namespace leanclr
