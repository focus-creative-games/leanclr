#pragma once

#include "core/rt_base.h"
#include "rt_managed_types.h"
#include "gc/garbage_collector.h"


namespace leanclr
{
namespace vm
{

class Object
{
  public:
    // Create new instance of a class
    // please don't use this method directly, use LEANCLR_NEWOBJ instead
    static RtResult<RtObject*> __new_object(const metadata::RtClass* klass LEANCLR_GC_DECLARE_CALL_SITE_PARAM);
    
    static RtResult<RtObject*> internal_create_instance(const metadata::RtTypeSig* type_sig LEANCLR_GC_DECLARE_CALL_SITE_PARAM);

    // Box a value type into an object
    static RtResult<RtObject*> __box_object(const metadata::RtClass* klass, const void* value LEANCLR_GC_DECLARE_CALL_SITE_PARAM);

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
    static RtResult<RtObject*> __clone(RtObject* obj LEANCLR_GC_DECLARE_CALL_SITE_PARAM);

    // Extend small integer to i32 on stack
    static void extends_to_eval_stack(const void* src, interp::RtStackObject* dst, const metadata::RtClass* ele_klass);
};

#if LEANCLR_GC_DEBUG
#define LEANCLR_NEWOBJ(klass, call_site)                                                              \
      ::leanclr::vm::Object::__new_object(klass, call_site)
#define LEANCLR_BOX_OBJECT(klass, value, call_site)                                                              \
      ::leanclr::vm::Object::__box_object(klass, value, call_site)
#define LEANCLR_CLONE(obj, call_site)                                                              \
      ::leanclr::vm::Object::__clone(obj, call_site)
#define LEANCLR_CREATE_INSTANCE(type_sig, call_site)                                                              \
      ::leanclr::vm::Object::internal_create_instance(type_sig, call_site)
#else
#define LEANCLR_NEWOBJ(klass, call_site)                                                              \
      ::leanclr::vm::Object::__new_object(klass)
#define LEANCLR_BOX_OBJECT(klass, value, call_site)                                                              \
      ::leanclr::vm::Object::__box_object(klass, value)
#define LEANCLR_CLONE(obj, call_site)                                                              \
      ::leanclr::vm::Object::__clone(obj)
#define LEANCLR_CREATE_INSTANCE(type_sig, call_site)                                                              \
      ::leanclr::vm::Object::internal_create_instance(type_sig)
#endif

// Runtime-native allocation with optional GC allocation site (see gc/gc_newobj_macros.h).
#define LEANCLR_NEWOBJ_INTERNAL(klass, native_runtime_method)                                                            \
  LEANCLR_NEWOBJ(klass, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_runtime_method))
#define LEANCLR_BOX_OBJECT_INTERNAL(klass, value, native_runtime_method)                                                            \
  LEANCLR_BOX_OBJECT(klass, value, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_runtime_method))
#define LEANCLR_CLONE_INTERNAL(obj, native_runtime_method)                                                            \
  LEANCLR_CLONE(obj, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_runtime_method))
#define LEANCLR_CREATE_INSTANCE_INTERNAL(type_sig, native_runtime_method)                                                            \
  LEANCLR_CREATE_INSTANCE(type_sig, ::leanclr::gc::GcAllocSite::make_internal(__FILE__, __LINE__, native_runtime_method))
} // namespace vm
} // namespace leanclr
