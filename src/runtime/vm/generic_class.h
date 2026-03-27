#pragma once

#include "metadata/rt_metadata.h"

namespace leanclr
{
namespace vm
{
class GenericClass
{
  public:
    // Generic class creation
    static RtResult<metadata::RtClass*> get_class(uint32_t baseTypeDefGid, const metadata::RtGenericInst* classInst);
    static RtResult<metadata::RtClass*> get_base_class(const metadata::RtGenericClass* genericClass);

    static RtResultVoid setup_interfaces(metadata::RtClass* klass);
    static RtResultVoid setup_fields(metadata::RtClass* klass);
    static RtResultVoid setup_methods(metadata::RtClass* klass);
    static RtResultVoid setup_properties(metadata::RtClass* klass);
    static RtResultVoid setup_events(metadata::RtClass* klass);
    static RtResultVoid setup_vtables(metadata::RtClass* klass);
};
} // namespace vm
} // namespace leanclr
