#include "gc/gc_bitmap.h"

#include <cstring>

#include "gc/garbage_collector.h"
#include "vm/class.h"
#include "vm/field.h"

namespace leanclr
{
namespace gc
{

static constexpr size_t kBitsPerWord = sizeof(uintptr_t) * 8;

static void set_bitmap_bit(uintptr_t* bitmap, size_t bit_index)
{
    bitmap[bit_index / kBitsPerWord] |= static_cast<uintptr_t>(1) << (bit_index % kBitsPerWord);
}

static bool type_sig_contains_references(const metadata::RtTypeSig* sig)
{
    if (sig == nullptr)
    {
        return false;
    }
    switch (sig->ele_type)
    {
    case metadata::RtElementType::Object:
    case metadata::RtElementType::String:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::Array:
    case metadata::RtElementType::SZArray:
        return true;
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::GenericInst:
    {
        RtResult<metadata::RtClass*> cls_res = vm::Class::get_class_from_typesig(sig);
        return cls_res.is_ok() && vm::Class::get_has_references(cls_res.unwrap());
    }
    default:
        return false;
    }
}

static void mark_bitmap_for_type(uintptr_t* bitmap, const metadata::RtTypeSig* sig, uint32_t base_offset)
{
    if (!type_sig_contains_references(sig))
    {
        return;
    }

    switch (sig->ele_type)
    {
    case metadata::RtElementType::Object:
    case metadata::RtElementType::String:
    case metadata::RtElementType::Class:
    case metadata::RtElementType::Array:
    case metadata::RtElementType::SZArray:
        set_bitmap_bit(bitmap, static_cast<size_t>(base_offset / sizeof(void*)));
        return;
    case metadata::RtElementType::ValueType:
    case metadata::RtElementType::GenericInst:
    {
        RtResult<metadata::RtClass*> cls_res = vm::Class::get_class_from_typesig(sig);
        if (cls_res.is_err())
        {
            return;
        }
        metadata::RtClass* field_class = cls_res.unwrap();
        if (!vm::Class::get_has_references(field_class))
        {
            return;
        }
        utils::Vector<const metadata::RtFieldInfo*> fields;
        vm::Class::collect_instance_fields(field_class, fields, true);
        for (size_t i = 0; i < fields.size(); ++i)
        {
            const metadata::RtFieldInfo* field = fields[i];
            if (!vm::Field::is_instance(field))
            {
                continue;
            }
            const uint32_t field_offset = base_offset + field->offset;
            mark_bitmap_for_type(bitmap, field->type_sig, field_offset);
        }
        return;
    }
    default:
        return;
    }
}

void setup_class_gc_bitmap(metadata::RtClass* klass)
{
    klass->gc_bitmap = nullptr;
    klass->gc_bitmap_word_count = 0;
    // FIXME
    return;

    if (!vm::Class::get_has_references(klass) || vm::Class::is_array_or_szarray(klass))
    {
        return;
    }

    const size_t object_size = sizeof(vm::RtObject) + klass->instance_size_without_header;
    const size_t slot_count = (object_size + sizeof(void*) - 1) / sizeof(void*);
    if (slot_count == 0)
    {
        return;
    }

    const size_t word_count = (slot_count + kBitsPerWord - 1) / kBitsPerWord;
    uintptr_t* bitmap = static_cast<uintptr_t*>(GarbageCollector::allocate_fixed(word_count * sizeof(uintptr_t)));
    if (bitmap == nullptr)
    {
        return;
    }
    std::memset(bitmap, 0, word_count * sizeof(uintptr_t));

    utils::Vector<const metadata::RtFieldInfo*> instance_fields;
    vm::Class::collect_instance_fields(klass, instance_fields, true);
    for (size_t i = 0; i < instance_fields.size(); ++i)
    {
        const metadata::RtFieldInfo* field = instance_fields[i];
        if (!vm::Field::is_instance(field))
        {
            continue;
        }
        const uint32_t offset = vm::Field::get_instance_field_offset_includes_object_header_for_all_type(field);
        mark_bitmap_for_type(bitmap, field->type_sig, offset);
    }

    klass->gc_bitmap = bitmap;
    klass->gc_bitmap_word_count = static_cast<uint16_t>(word_count);
}

} // namespace gc
} // namespace leanclr
