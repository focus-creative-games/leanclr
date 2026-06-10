#include "gc/obj_scanner.h"

#include <cassert>

#include "utils/mem_op.h"
#include "vm/class.h"
#include "vm/object.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace gc
{

ObjScanner::ObjScanner(ObjVisitFn visit_fn, void* userdata) : visit_fn_(visit_fn), userdata_(userdata), deferred_scan_depth_(0), recursive_visit_depth_(0)
{
}

ObjScanner::~ObjScanner()
{
    assert(deferred_field_scan_queue_.empty());
}

bool ObjScanner::visit(vm::RtObject* obj)
{
    return visit_fn_(obj, userdata_);
}

void ObjScanner::visit_object(vm::RtObject* obj)
{
    if (obj == nullptr)
    {
        return;
    }
    if (!visit(obj))
    {
        return;
    }
    if (vm::Class::get_has_references(obj->klass))
    {
        scan_object_fields(obj);
    }
}

void ObjScanner::visit_class_static_data(const metadata::RtClass* klass)
{
    if (klass->static_gc_bitmap_word_count == 0)
    {
        return;
    }
    visit_gc_bitmap_words(klass->static_gc_bitmap, klass->static_gc_bitmap_word_count, klass->static_fields_data);
}

void ObjScanner::visit_all_classes_static_data()
{
    auto& all_classes_with_static_data = vm::Class::get_all_classes_with_static_data();
    for (const metadata::RtClass* klass : all_classes_with_static_data)
    {
        visit_class_static_data(klass);
    }
}

void ObjScanner::finish_visit()
{
    flush_deferred_field_scans();
    assert(deferred_field_scan_queue_.empty());
}

void ObjScanner::flush_deferred_field_scans()
{
    deferred_scan_depth_++;
    while (!deferred_field_scan_queue_.empty())
    {
        vm::RtObject* obj = deferred_field_scan_queue_.back();
        deferred_field_scan_queue_.pop_back();
        scan_object_fields(obj);
    }
    deferred_scan_depth_--;
}

void ObjScanner::enqueue_deferred_field_scan(vm::RtObject* obj)
{
    assert(obj != nullptr);
    if (!visit(obj))
    {
        return;
    }
    if (!vm::Class::get_has_references(obj->klass))
    {
        return;
    }
    if (deferred_scan_depth_ == 0 && deferred_field_scan_queue_.size() < kDeferredScanBatchThreshold && recursive_visit_depth_ < kMaxRecursiveVisitDepth)
    {
        recursive_visit_depth_++;
        scan_object_fields(obj);
        recursive_visit_depth_--;
        return;
    }
    deferred_field_scan_queue_.push_back(obj);
    if (deferred_field_scan_queue_.size() >= kDeferredScanBatchThreshold && deferred_scan_depth_ == 0)
    {
        flush_deferred_field_scans();
    }
}

void ObjScanner::visit_object_self_only(vm::RtObject* obj)
{
    visit(obj);
}

void ObjScanner::scan_object_fields(vm::RtObject* obj)
{
    if (vm::Class::is_array_or_szarray(obj->klass))
    {
        scan_array_object(reinterpret_cast<vm::RtArray*>(obj));
    }
    else
    {
        scan_normal_object(obj);
    }
}

void ObjScanner::scan_normal_object(vm::RtObject* obj)
{
    assert(vm::Class::get_has_references(obj->klass));
    visit_gc_bitmap(obj->klass, reinterpret_cast<uint8_t*>(obj));
}

void ObjScanner::scan_array_object(vm::RtArray* obj)
{
    assert(vm::Class::get_has_references(obj->klass));
    const metadata::RtClass* element_class = obj->klass->element_class;
    if (vm::Class::is_reference_type(element_class))
    {
        vm::RtObject** elements = vm::Array::get_array_data_start_as<vm::RtObject*>(obj);
        if (vm::Class::is_sealed(element_class) && !vm::Class::get_has_references(element_class))
        {
            for (int32_t i = 0; i < obj->length; ++i)
            {
                vm::RtObject* elem = elements[i];
                if (elem == nullptr)
                {
                    continue;
                }
                visit_object_self_only(elem);
            }
        }
        else
        {
            for (int32_t i = 0; i < obj->length; ++i)
            {
                vm::RtObject* elem = elements[i];
                if (elem == nullptr)
                {
                    continue;
                }
                enqueue_deferred_field_scan(elem);
            }
        }
    }
    else
    {
        size_t element_size = vm::Array::get_array_element_size(obj);
        void* elements_start_address = vm::Array::get_array_data_start_as_ptr_void(obj);
        for (size_t i = 0, n = static_cast<size_t>(obj->length); i < n; ++i)
        {
            uint8_t* element_address = reinterpret_cast<uint8_t*>(elements_start_address) + i * element_size;
            visit_value_type(element_address, element_class);
        }
    }
}

void ObjScanner::visit_gc_bitmap(const metadata::RtClass* klass, uint8_t* slot_base)
{
    visit_gc_bitmap_words(klass->gc_bitmap, klass->gc_bitmap_word_count, slot_base);
}

void ObjScanner::visit_gc_bitmap_words(const size_t* bitmap, size_t word_count, uint8_t* slot_base)
{
    if (word_count == 0)
    {
        return;
    }

    const size_t kBitsPerWord = vm::Class::kBitsPerWord;

    for (size_t w = 0; w < word_count; ++w)
    {
        size_t word = bitmap[w];
        while (word != 0)
        {
            const size_t bit_in_word = utils::MemOp::count_trailing_zeros_nonzero(word);
            const size_t bit_index = w * kBitsPerWord + bit_in_word;
            vm::RtObject** slot = reinterpret_cast<vm::RtObject**>(slot_base + bit_index * sizeof(void*));
            vm::RtObject* child = *slot;
            if (child)
            {
                enqueue_deferred_field_scan(child);
            }
            word &= word - 1;
        }
    }
}

void ObjScanner::visit_value_type(uint8_t* data, const metadata::RtClass* value_type_class)
{
    visit_gc_bitmap(value_type_class, data - vm::RT_OBJECT_HEADER_SIZE);
}

} // namespace gc
} // namespace leanclr
