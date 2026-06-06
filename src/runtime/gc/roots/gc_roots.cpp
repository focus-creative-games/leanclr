#include "gc/roots/gc_roots.h"

#include "utils/rt_vector.h"
#include "vm/class.h"
#include "vm/field.h"

namespace leanclr
{
namespace gc
{

static utils::Vector<vm::RtObject**> s_registered_slots;
static utils::Vector<GcVisitObjectRootsScan> s_visit_object_roots;
static utils::Vector<GcVisitUnknownBlocksScan> s_visit_unknown_blocks;

void GcRoots::register_slot(vm::RtObject** slot)
{
    if (slot == nullptr)
    {
        return;
    }
    s_registered_slots.push_back(slot);
}

void GcRoots::unregister_slot(vm::RtObject** slot)
{
    for (size_t i = 0; i < s_registered_slots.size(); ++i)
    {
        if (s_registered_slots[i] == slot)
        {
            s_registered_slots[i] = s_registered_slots[s_registered_slots.size() - 1];
            s_registered_slots.pop_back();
            return;
        }
    }
}

static void scan_static_fields(const metadata::RtClass* klass, GcRootCallback callback, void* userdata)
{
    utils::Vector<const metadata::RtFieldInfo*> static_fields;
    for (uint16_t i = 0; i < klass->field_count; ++i)
    {
        const metadata::RtFieldInfo* field = klass->fields + i;
        if (vm::Field::is_static_excluded_literal_and_rva(field))
        {
            static_fields.push_back(field);
        }
    }
    for (size_t i = 0; i < static_fields.size(); ++i)
    {
        const metadata::RtFieldInfo* field = static_fields[i];
        const metadata::RtTypeSig* sig = field->type_sig;
        bool is_ref = false;
        if (sig->ele_type == metadata::RtElementType::Class || sig->ele_type == metadata::RtElementType::Object ||
            sig->ele_type == metadata::RtElementType::String || sig->ele_type == metadata::RtElementType::Array ||
            sig->ele_type == metadata::RtElementType::SZArray)
        {
            is_ref = true;
        }
        else
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
        vm::RtObject** slot = reinterpret_cast<vm::RtObject**>(klass->static_fields_data + field->offset);
        callback(slot, userdata);
    }
}

void GcRoots::register_visit_object_roots(GcVisitObjectRootsScan scan)
{
    if (scan == nullptr)
    {
        return;
    }
    s_visit_object_roots.push_back(scan);
}

void GcRoots::register_visit_unknown_blocks(GcVisitUnknownBlocksScan scan)
{
    if (scan == nullptr)
    {
        return;
    }
    s_visit_unknown_blocks.push_back(scan);
}

void GcRoots::foreach_root(GcRootCallback callback, void* userdata)
{
    for (size_t i = 0; i < s_registered_slots.size(); ++i)
    {
        callback(s_registered_slots[i], userdata);
    }
    auto& all_classes_with_static_data = vm::Class::get_all_classes_with_static_data();
    for (const metadata::RtClass* klass : all_classes_with_static_data)
    {
        scan_static_fields(klass, callback, userdata);
    }
    for (size_t i = 0; i < s_visit_unknown_blocks.size(); ++i)
    {
        // TODO: implement this
        //s_visit_unknown_blocks[i](callback, userdata);
    }
}

void GcRoots::foreach_object_root(GcVisitObjectRoot visit, void* userdata)
{
    for (size_t i = 0; i < s_visit_object_roots.size(); ++i)
    {
        s_visit_object_roots[i](visit, userdata);
    }
}

} // namespace gc
} // namespace leanclr
