#pragma once

#include "core/rt_base.h"
#include "rt_managed_types.h"

namespace leanclr
{
namespace vm
{

class Object
{
  public:
    // Create new instance of a class
    static RtResult<RtObject*> new_object(const metadata::RtClass* klass);

    // Box a value type into an object
    static RtResult<RtObject*> box_object(const metadata::RtClass* klass, const void* value);

    // Get pointer to boxed value data
    static const void* get_box_value_type_data_ptr(const RtObject* obj);

    // Get pointer to boxed enum data
    static const void* get_boxed_enum_data_ptr(const RtObject* obj);

    // Unbox value from boxed object (with optional stack extension)
    static RtResultVoid unbox_any(const RtObject* obj, const metadata::RtClass* klass, void* dst, bool extend_to_stack);

    // Unbox with exact type checking
    static RtResult<const void*> unbox_ex(const RtObject* obj, const metadata::RtClass* unbox_class);

    // Type checking and casting
    static RtObject* is_inst(RtObject* obj, const metadata::RtClass* klass);
    static RtObject* cast_class(RtObject* obj, const metadata::RtClass* klass);

    // Clone an object
    static RtResult<RtObject*> clone(RtObject* obj);

    // Extend small integer to i32 on stack
    static void extends_to_eval_stack(const void* src, interp::RtStackObject* dst, const metadata::RtClass* ele_klass);
};

} // namespace vm
} // namespace leanclr
