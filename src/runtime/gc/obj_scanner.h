#pragma once

#include <cstdint>

#include "utils/rt_vector.h"
#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace metadata
{
struct RtClass;
}

namespace vm
{
struct RtArray;
}

namespace gc
{

using ObjVisitFn = bool (*)(vm::RtObject* obj, void* userdata);

class ObjScanner
{
  public:
    static constexpr size_t kDeferredScanBatchThreshold = 256;
    static constexpr int32_t kMaxRecursiveVisitDepth = 16;

    ObjScanner(ObjVisitFn visit_fn, void* userdata);
    ~ObjScanner();

    void visit_object(vm::RtObject* obj);
    void visit_class_static_data(const metadata::RtClass* klass);
    void visit_all_classes_static_data();
    void finish_visit();

  private:
    ObjVisitFn visit_fn_;
    void* userdata_;
    utils::Vector<vm::RtObject*> deferred_field_scan_queue_;
    int32_t deferred_scan_depth_;
    int32_t recursive_visit_depth_;

    bool visit(vm::RtObject* obj);
    void flush_deferred_field_scans();
    void enqueue_deferred_field_scan(vm::RtObject* obj);
    void visit_object_self_only(vm::RtObject* obj);
    void scan_object_fields(vm::RtObject* obj);
    void scan_normal_object(vm::RtObject* obj);
    void scan_array_object(vm::RtArray* obj);
    void visit_gc_bitmap(const metadata::RtClass* klass, uint8_t* slot_base);
    void visit_gc_bitmap_words(const size_t* bitmap, size_t word_count, uint8_t* slot_base);
    void visit_value_type(uint8_t* data, const metadata::RtClass* value_type_class);
};

} // namespace gc
} // namespace leanclr
