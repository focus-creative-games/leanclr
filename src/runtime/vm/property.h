#pragma once

#include "rt_managed_types.h"
#include "utils/rt_vector.h"

namespace leanclr
{
namespace vm
{
class Property
{
  public:
    static bool is_static(const metadata::RtPropertyInfo* property);
    static bool is_public(const metadata::RtPropertyInfo* property);
    static bool is_private(const metadata::RtPropertyInfo* property);
    static RtResult<vm::RtObject*> get_const_object(const metadata::RtPropertyInfo* property);
    static RtResultVoid get_property_modifiers(const metadata::RtPropertyInfo* property, bool optional, utils::Vector<metadata::RtClass*>& modifiers);
};
} // namespace vm
} // namespace leanclr
