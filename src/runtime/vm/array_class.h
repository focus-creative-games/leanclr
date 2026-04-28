#pragma once

#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace vm
{

class ArrayClass
{
  public:
    // Static initializer
    static RtResultVoid initialize();

    // Array class creation functions
    static RtResult<metadata::RtClass*> get_array_class_from_element_type(const metadata::RtTypeSig* ele_type, uint8_t rank);
    static RtResult<metadata::RtClass*> get_array_class_from_element_klass(const metadata::RtClass* ele_klass, uint8_t rank);
    static RtResult<metadata::RtClass*> get_szarray_class_from_element_typesig(const metadata::RtTypeSig* ele_type);
    static RtResult<metadata::RtClass*> get_szarray_class_from_element_class(const metadata::RtClass* ele_class);
    static const metadata::RtClass* get_array_variance_reduce_type(const metadata::RtClass* klass);

    // Setup functions
    static RtResultVoid setup_interfaces(metadata::RtClass* klass);
    static RtResultVoid setup_methods(metadata::RtClass* klass);
    static RtResultVoid setup_vtables(metadata::RtClass* klass);

    // Interface method initialization
    static RtResultVoid initialize_array_interface_methods();

    static void walk_array_classes(metadata::ClassWalkCallback callback, void* userData);
};
} // namespace vm
} // namespace leanclr
