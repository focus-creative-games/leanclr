#include "gc/roots/gc_roots.h"

#include "utils/rt_vector.h"
#include "vm/class.h"
#include "vm/field.h"

namespace leanclr
{
namespace gc
{

struct StaticFieldsRoot
{
    metadata::RtClass* klass;
    uint8_t* data;
    size_t size;
};

static utils::Vector<vm::RtObject**> s_registered_slots;
static utils::Vector<StaticFieldsRoot> s_static_fields;
static utils::Vector<GcVisitObjectRootsScan> s_visit_object_roots;

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

void GcRoots::register_static_fields(metadata::RtClass* klass, uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0)
    {
        return;
    }
    StaticFieldsRoot root;
    root.klass = klass;
    root.data = data;
    root.size = size;
    s_static_fields.push_back(root);
}

static void scan_static_fields(const StaticFieldsRoot& root, GcRootCallback callback, void* userdata)
{
    utils::Vector<const metadata::RtFieldInfo*> static_fields;
    for (uint16_t i = 0; i < root.klass->field_count; ++i)
    {
        const metadata::RtFieldInfo* field = root.klass->fields + i;
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
        vm::RtObject** slot = reinterpret_cast<vm::RtObject**>(root.data + field->offset);
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

void GcRoots::foreach_root(GcRootCallback callback, void* userdata)
{
    for (size_t i = 0; i < s_registered_slots.size(); ++i)
    {
        callback(s_registered_slots[i], userdata);
    }
    for (size_t i = 0; i < s_static_fields.size(); ++i)
    {
        scan_static_fields(s_static_fields[i], callback, userdata);
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
