#include "obj_scan_util.h"

#include "utils/mem_op.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/object.h"
#include "vm/type.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace gc
{
static void visit_gc_bitmap(const metadata::RtClass* klass, uint8_t* slot_base, GcVisitObjectFn visit, void* userdata)
{
    const size_t bit_count = klass->gc_bitmap_bit_count;
    const size_t start_bit = vm::Class::kFirstGCBitmapBitIndex;
    if (bit_count <= start_bit)
    {
        return;
    }

    const size_t* bitmap = klass->gc_bitmap;
    const size_t kBitsPerWord = vm::Class::kBitsPerWord;
    const size_t start_word = start_bit / kBitsPerWord;
    const size_t end_word = (bit_count + kBitsPerWord - 1) / kBitsPerWord;

    for (size_t w = start_word; w < end_word; ++w)
    {
        size_t word = bitmap[w];
        while (word != 0)
        {
            const size_t bit_in_word = utils::MemOp::count_trailing_zeros_nonzero(word);
            const size_t bit_index = w * kBitsPerWord + bit_in_word;
            vm::RtObject** slot = reinterpret_cast<vm::RtObject**>(slot_base + bit_index * sizeof(void*));
            vm::RtObject* obj = *slot;
            if (obj != nullptr)
            {
                ObjScanUtil::visit_object(obj, visit, userdata);
            }
            word &= word - 1;
        }
    }
}

static void visit_normal_object(vm::RtObject* obj, GcVisitObjectFn visit, void* userdata)
{
    visit_gc_bitmap(obj->klass, reinterpret_cast<uint8_t*>(obj), visit, userdata);
}

static void visit_value_type(uint8_t* data, GcVisitObjectFn visit, void* userdata, const metadata::RtClass* value_type_class)
{
    visit_gc_bitmap(value_type_class, data - vm::RT_OBJECT_HEADER_SIZE, visit, userdata);
}

static void visit_array_object(vm::RtArray* obj, GcVisitObjectFn visit, void* userdata)
{
    if (!vm::Class::get_has_references(obj->klass))
    {
        return;
    }
    const metadata::RtClass* element_class = obj->klass->element_class;
    if (vm::Class::is_reference_type(element_class))
    {
        vm::RtObject** elements = vm::Array::get_array_data_start_as<vm::RtObject*>(obj);
        for (int32_t i = 0; i < obj->length; ++i)
        {
            ObjScanUtil::visit_object(elements[i], visit, userdata);
        }
    }
    else
    {
        size_t element_size = vm::Array::get_array_element_size(obj);
        void* elements_start_address = vm::Array::get_array_data_start_as_ptr_void(obj);
        for (size_t i = 0, n = static_cast<size_t>(obj->length); i < n; ++i)
        {
            uint8_t* element_address = reinterpret_cast<uint8_t*>(elements_start_address) + i * element_size;
            visit_value_type(element_address, visit, userdata, element_class);
        }
    }
}

void ObjScanUtil::visit_object(vm::RtObject* obj, GcVisitObjectFn visit, void* userdata)
{
    if (!obj)
    {
        return;
    }
    if (!visit(obj, userdata))
    {
        return;
    }
    if (vm::Class::is_array_or_szarray(obj->klass))
    {
        visit_array_object(reinterpret_cast<vm::RtArray*>(obj), visit, userdata);
    }
    else
    {
        visit_normal_object(obj, visit, userdata);
    }
}

void ObjScanUtil::visit_class_static_data(const metadata::RtClass* klass, GcVisitObjectFn visit, void* userdata)
{
    for (uint16_t i = 0; i < klass->field_count; ++i)
    {
        const metadata::RtFieldInfo* field = klass->fields + i;
        if (!vm::Field::is_static_excluded_literal_and_rva(field))
        {
            continue;
        }
        const metadata::RtTypeSig* type_sig = field->type_sig;
        if (type_sig->is_by_ref())
        {
            continue;
        }
        switch (type_sig->ele_type)
        {
        case metadata::RtElementType::Boolean:
        case metadata::RtElementType::Char:
        case metadata::RtElementType::I1:
        case metadata::RtElementType::U1:
        case metadata::RtElementType::I2:
        case metadata::RtElementType::U2:
        case metadata::RtElementType::I4:
        case metadata::RtElementType::U4:
        case metadata::RtElementType::I8:
        case metadata::RtElementType::U8:
        case metadata::RtElementType::R4:
        case metadata::RtElementType::R8:
        case metadata::RtElementType::I:
        case metadata::RtElementType::U:
        case metadata::RtElementType::Ptr:
        case metadata::RtElementType::FnPtr:
        case metadata::RtElementType::TypedByRef:
        case metadata::RtElementType::Var:
        case metadata::RtElementType::MVar:
        {
            break;
        }
        case metadata::RtElementType::Object:
        case metadata::RtElementType::String:
        case metadata::RtElementType::Class:
        case metadata::RtElementType::SZArray:
        case metadata::RtElementType::Array:
        handle_reference_type_field:
        {
            assert(field->offset % sizeof(void*) == 0);
            vm::RtObject* obj = *reinterpret_cast<vm::RtObject**>(reinterpret_cast<uint8_t*>(klass->static_fields_data) + field->offset);
            if (obj != nullptr)
            {
                ObjScanUtil::visit_object(obj, visit, userdata);
            }
            break;
        }
        case metadata::RtElementType::GenericInst:
        {
            bool is_ref_type = vm::Type::is_reference_type(type_sig).unwrap();
            if (is_ref_type)
            {
                goto handle_reference_type_field;
            }
            // fall through
        }
        case metadata::RtElementType::ValueType:
        {
            metadata::RtClass* field_class = vm::Class::get_class_from_typesig(type_sig).unwrap();
            assert(vm::Class::has_initialized_part(field_class, metadata::RtClassInitPart::Field));
            if (!vm::Class::get_has_references(field_class))
            {
                break;
            }
            assert(field->offset % sizeof(void*) == 0);
            uint8_t* cur_field_address = reinterpret_cast<uint8_t*>(klass->static_fields_data) + field->offset;
            visit_value_type(cur_field_address, visit, userdata, field_class);
            break;
        }
        default:
        {
            assert(false && "invalid element type");
        }
        }
    }
}

void ObjScanUtil::visit_all_classes_static_data(GcVisitObjectFn visit, void* userdata)
{
    auto& all_classes_with_static_data = vm::Class::get_all_classes_with_static_data();
    for (const metadata::RtClass* klass : all_classes_with_static_data)
    {
        visit_class_static_data(klass, visit, userdata);
    }
}
} // namespace gc
} // namespace leanclr