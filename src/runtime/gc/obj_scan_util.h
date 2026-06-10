#pragma once

#include <cassert>
#include <cstdint>

#include "utils/mem_op.h"
#include "utils/rt_vector.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/object.h"
#include "vm/rt_array.h"
#include "vm/rt_managed_types.h"
#include "vm/type.h"

namespace leanclr
{
namespace gc
{
class ObjScanUtil
{
  public:
    static constexpr size_t kDeferredScanBatchThreshold = 256;
    static constexpr int32_t kMaxRecursiveVisitDepth = 16;

    template <typename VisitContext>
    static void visit_object(vm::RtObject* obj, VisitContext& ctx)
    {
        if (obj == nullptr)
        {
            return;
        }
        if (!ctx.visit(obj))
        {
            return;
        }
        if (vm::Class::get_has_references(obj->klass))
        {
            scan_object_fields(obj, ctx);
        }
    }

    template <typename VisitContext>
    static void visit_class_static_data(const metadata::RtClass* klass, VisitContext& ctx)
    {
        if (klass->static_fields_data == nullptr || klass->static_gc_bitmap_word_count == 0)
        {
            return;
        }
        visit_gc_bitmap_words(klass->static_gc_bitmap, klass->static_gc_bitmap_word_count, klass->static_fields_data, ctx);
    }

    template <typename VisitContext>
    static void visit_all_classes_static_data(VisitContext& ctx)
    {
        auto& all_classes_with_static_data = vm::Class::get_all_classes_with_static_data();
        for (const metadata::RtClass* klass : all_classes_with_static_data)
        {
            visit_class_static_data(klass, ctx);
        }
    }

    template <typename VisitContext>
    static void finish_visit(VisitContext& ctx)
    {
        flush_deferred_field_scans(ctx);
        assert(ctx.deferred_field_scan_queue.empty());
    }

    template <typename VisitContext>
    static void flush_deferred_field_scans(VisitContext& ctx)
    {
        ctx.deferred_scan_depth++;
        while (!ctx.deferred_field_scan_queue.empty())
        {
            vm::RtObject* obj = ctx.deferred_field_scan_queue.back();
            ctx.deferred_field_scan_queue.pop_back();
            scan_object_fields(obj, ctx);
        }
        ctx.deferred_scan_depth--;
    }

  private:
    template <typename VisitContext>
    static void enqueue_deferred_field_scan(vm::RtObject* obj, VisitContext& ctx)
    {
        if (obj == nullptr)
        {
            return;
        }
        if (ctx.deferred_scan_depth == 0 && ctx.deferred_field_scan_queue.size() < kDeferredScanBatchThreshold &&
            ctx.recursive_visit_depth < kMaxRecursiveVisitDepth)
        {
            ctx.recursive_visit_depth++;
            visit_object(obj, ctx);
            ctx.recursive_visit_depth--;
            return;
        }
        if (!ctx.visit(obj))
        {
            return;
        }
        if (!vm::Class::get_has_references(obj->klass))
        {
            return;
        }
        ctx.deferred_field_scan_queue.push_back(obj);
        if (ctx.deferred_field_scan_queue.size() >= kDeferredScanBatchThreshold && ctx.deferred_scan_depth == 0)
        {
            flush_deferred_field_scans(ctx);
        }
    }

    template <typename VisitContext>
    static void visit_object_self_only(vm::RtObject* obj, VisitContext& ctx)
    {
        if (obj == nullptr)
        {
            return;
        }
        ctx.visit(obj);
    }

    template <typename VisitContext>
    static void scan_object_fields(vm::RtObject* obj, VisitContext& ctx)
    {
        if (vm::Class::is_array_or_szarray(obj->klass))
        {
            scan_array_object(reinterpret_cast<vm::RtArray*>(obj), ctx);
        }
        else
        {
            scan_normal_object(obj, ctx);
        }
    }

    template <typename VisitContext>
    static void scan_normal_object(vm::RtObject* obj, VisitContext& ctx)
    {
        if (!vm::Class::get_has_references(obj->klass))
        {
            return;
        }
        visit_gc_bitmap(obj->klass, reinterpret_cast<uint8_t*>(obj), ctx);
    }

    template <typename VisitContext>
    static void scan_array_object(vm::RtArray* obj, VisitContext& ctx)
    {
        if (!vm::Class::get_has_references(obj->klass))
        {
            return;
        }
        const metadata::RtClass* element_class = obj->klass->element_class;
        if (vm::Class::is_reference_type(element_class))
        {
            vm::RtObject** elements = vm::Array::get_array_data_start_as<vm::RtObject*>(obj);
            if (!vm::Class::get_has_references(element_class) && vm::Class::is_sealed(element_class) && !vm::Class::is_array_or_szarray(element_class))
            {
                for (int32_t i = 0; i < obj->length; ++i)
                {
                    visit_object_self_only(elements[i], ctx);
                }
            }
            else
            {
                for (int32_t i = 0; i < obj->length; ++i)
                {
                    enqueue_deferred_field_scan(elements[i], ctx);
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
                visit_value_type(element_address, ctx, element_class);
            }
        }
    }

    template <typename VisitContext>
    static void visit_gc_bitmap(const metadata::RtClass* klass, uint8_t* slot_base, VisitContext& ctx)
    {
        visit_gc_bitmap_words(klass->gc_bitmap, klass->gc_bitmap_word_count, slot_base, ctx);
    }

    template <typename VisitContext>
    static void visit_gc_bitmap_words(const size_t* bitmap, size_t word_count, uint8_t* slot_base, VisitContext& ctx)
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
                enqueue_deferred_field_scan(*slot, ctx);
                word &= word - 1;
            }
        }
    }

    template <typename VisitContext>
    static void visit_value_type(uint8_t* data, VisitContext& ctx, const metadata::RtClass* value_type_class)
    {
        visit_gc_bitmap(value_type_class, data - vm::RT_OBJECT_HEADER_SIZE, ctx);
    }
};
} // namespace gc
} // namespace leanclr
