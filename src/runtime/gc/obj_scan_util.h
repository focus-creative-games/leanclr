#pragma once

#include "vm/rt_managed_types.h"

namespace leanclr
{
namespace gc
{
///
/// Returns true if the object is first visited.
//
typedef bool (*GcVisitObjectFn)(vm::RtObject* obj, void* userdata);

class ObjScanUtil
{
public:
    static void visit_object(vm::RtObject* obj, GcVisitObjectFn visit, void* userdata);
    static void visit_class_static_data(const metadata::RtClass* klass, GcVisitObjectFn visit, void* userdata);
    static void visit_all_classes_static_data(GcVisitObjectFn visit, void* userdata);
};
} // namespace gc
} // namespace leanclr